/**
 * @file TX_functions.c
 * @author Ricardo Jorge Dias Sampaio
 * @brief Implementation of transmission for the Secure VLC Project.
 * @version 1.0
 * @date 2024-09-03
 * * \par License:
 *   \ref mit_license "MIT License".
 * 
 * @details This file contains the implementation of functions related to data transmission,
 * including buffer management, GPIO setup, and timer configuration for the Secure VLC Project.
 */

#include "TX_functions.h"

/** @brief Structure to store the encryption variables for transmission. */
encryption_vars_t TX_encryption_vars;

/** @brief Pointer to the ring buffer for transmitted data. */
static RingBuffer* ring_buffer_TX = NULL;

/** @brief Tag for logging messages related to TX operations. */
static const char* TX_TAG = "TX";

/** @brief Handle to the GPTimer used for transmission. */
static gptimer_handle_t timer_TX = NULL;

/** @brief Value currently being transmitted. */
static volatile uint32_t value_TX = 0;

/** @brief Bit counter for the current value. */
static volatile uint8_t bit_counter_TX = 0;

/** @brief Flag to indicate if a transmission is in progress. */
static volatile bool in_transmission = false;

/**
 * @brief Combines four bytes into a 32-bit unsigned integer.
 * 
 * @param b0 Least significant byte
 * @param b1 Second byte
 * @param b2 Third byte
 * @param b3 Most significant byte
 * @return uint32_t Combined 32-bit unsigned integer
 */
static uint32_t combineCharsToUint32(unsigned char b0, unsigned char b1, unsigned char b2, unsigned char b3) {
    return  ((uint32_t)b3 << 24) | 
            ((uint32_t)b2 << 16) | 
            ((uint32_t)b1 << 8)  | 
            (uint32_t)b0;
}

/**
 * @brief Starts the transmission process.
 * 
 * This function pops a value from the ring buffer, sets the GPIO low,
 * and starts the transmission timer.
 */
static void start_transmission() {
    ringBufferPop(ring_buffer_TX, &value_TX);
    directWriteLow_single();
    gptimer_start(timer_TX);
}

/**
 * @brief Adds a string to the transmission buffer.
 * 
 * This function takes an input string, processes it in 4-byte chunks,
 * applies encryption, and pushes the resulting values to the ring buffer.
 * 
 * @param input_str The input string to be added to the buffer
 */
void add_str_to_buffer(const char* input_str) {
    size_t len = strlen(input_str);
    size_t i;
    char b0, b1, b2, b3;
    for (i = 0; i < len; i += 4) {
        b0 = (i < len) ? input_str[i] : 0;
        b1 = (i + 1 < len) ? input_str[i + 1] : 0;
        b2 = (i + 2 < len) ? input_str[i + 2] : 0;
        b3 = (i + 3 < len) ? input_str[i + 3] : 0;

        volatile uint32_t value =   combineCharsToUint32(b0, b1, b2, b3)^
                                    key_generator(&TX_encryption_vars);
        ringBufferPush(ring_buffer_TX, value);
    }
}

/**
 * @brief Function to add a "Hello World!" to the buffer.
 * 
 * This function currently adds a hardcoded string "Hello World!" to the buffer.
 * 
 */
static void hello_world_TX(void) {
    char *input_str = "Hello World!";
    add_str_to_buffer(input_str);
}

/**
 * @brief Checks the transmission buffer and starts transmission if possible.
 * 
 * This function checks if the buffer is not empty and no transmission is currently
 * in progress. If both conditions are met, it starts a new transmission.
 */
static void check_TX() {
    if ((!ringBufferIsEmpty(ring_buffer_TX)) & (!in_transmission)) {
        in_transmission = true;
        vTaskDelay(pdMS_TO_TICKS(10));
        start_transmission();
    }
}

/**
 * @brief Interrupt Service Routine for the transmission timer.
 * 
 * This function is called on each timer interrupt. It handles the bit-by-bit
 * transmission of the current value, setting the GPIO high or low accordingly.
 * 
 * @param timer Timer handle
 * @param edata Pointer to alarm event data
 * @param arg User argument (unused)
 * @return true if the ISR should yield, false otherwise
 */
static bool IRAM_ATTR timer_TX_ISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *arg) {
    if (__builtin_expect(bit_counter_TX < 32, 1)) {
        ((value_TX >> bit_counter_TX) & 0x1) ? directWriteHigh_single() : directWriteLow_single();
    } else if(bit_counter_TX == 32)  {
        directWriteHigh_single();
    } else {
        gptimer_stop(timer_TX);
        in_transmission = false;
        bit_counter_TX=0;
        return true;
    }
    bit_counter_TX++;
    return true;
}

/**
 * @brief Sets up the transmission timer.
 * 
 * This function configures and initializes the timer used for transmission timing.
 */
static void setup_timer_TX() {
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION_HZ,
        .intr_priority = TIMER_INTERRUPTION_PRIORITY,
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer_TX));

    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,
        .alarm_count = TX_PERIOD_MICROS,
        .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer_TX, &alarm_config));

    gptimer_event_callbacks_t call_back_timer = {
        .on_alarm = timer_TX_ISR,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer_TX, &call_back_timer, NULL));

    ESP_ERROR_CHECK(gptimer_enable(timer_TX));
}

/**
 * @brief Sets up the GPIO for transmission.
 * 
 * This function configures the GPIO pin used for transmission and sets up
 * a dedicated GPIO bundle for efficient control.
 */
static void setup_gpio_TX(void) {
    const int TX_bundle_gpios[] = {TX_GPIO_PIN_NUM};

    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = TX_GPIO_PIN_SEL,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    dedic_gpio_bundle_handle_t TX_bundle = NULL;
    dedic_gpio_bundle_config_t TX_bundle_config = {
        .gpio_array = TX_bundle_gpios,
        .array_size = sizeof(TX_bundle_gpios) / sizeof(TX_bundle_gpios[0]),
        .flags = {
            .out_en = 1,
        },
    };
    ESP_ERROR_CHECK(dedic_gpio_new_bundle(&TX_bundle_config, &TX_bundle));
    ESP_LOGI(TX_TAG, "GPIO Setup Complete");
}

/**
 * @brief TX control task.
 *
 * This task sets up the GPIO for transmission, initializes the transmission timer,
 * waits for encryption values to be set, and then enters a loop to manage the transmission buffer.
 *
 * @param pvParameters Pointer to task parameters (unused).
 */
void TX_control_task(void *pvParameters) {
    setup_gpio_TX();
    setup_timer_TX();
    ring_buffer_TX = createRingBuffer();
    if (ring_buffer_TX == NULL) {
        ESP_LOGE(TX_TAG, "Failed to create TX ring buffer");
        vTaskDelete(NULL);
    }
    directWriteHigh_single();
    ESP_LOGW(TX_TAG, "Need to set encryption values for reception and transmission before proceeding");
    while((!tx_encryption_set) || (!rx_encryption_set)){
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    
    //hello_world_TX();
    //vTaskDelay(pdMS_TO_TICKS(100));
    ESP_LOGI(TX_TAG,"ENTERING LOOP");
    while (1) {
        check_TX();
        vTaskDelay(pdMS_TO_TICKS(10)); // Wait some time
    }
}