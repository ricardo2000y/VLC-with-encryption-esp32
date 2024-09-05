/**
 * @file RX_functions.c
 * @author Ricardo Jorge Dias Sampaio
 * @brief Implementation of reception for the Secure VLC Project.
 * @version 1.0
 * @date 2024-09-03
 * \par License:
 *   \ref mit_license "MIT License".
 * 
 * @details This file contains the implementation of functions related to data reception,
 * including buffer management, GPIO setup, and timer configuration for the Secure VLC Project.
 */

#include "RX_functions.h"

/** @brief Structure to store the encryption variables for reception */
encryption_vars_t RX_encryption_vars;

/** @brief Pointer to the ring buffer for received data */
static RingBuffer* ring_buffer_RX = NULL;

/** @brief Tag for logging RX messages */
static const char *RX_TAG = "RX";

/** @brief Handle for the reception timer. */
static gptimer_handle_t timer_RX = NULL;

/** @brief Variable to store the read value from GPIO */
static volatile uint32_t value_RX = 0;

/** @brief Bit counter for the current value */
static volatile uint8_t bit_counter_RX = 0;

/** @brief Flag to indicate if data reception is complete */
static volatile bool reception_complete = false;

/**
 * @brief Cached function pointer for gptimer_start.
 *
 * This function pointer is used to start the general-purpose timer.
 */
static esp_err_t (*cached_gptimer_start)(gptimer_handle_t timer) = gptimer_start;

/**
 * @brief Cached function pointer for gptimer_stop.
 *
 * This function pointer is used to stop the general-purpose timer.
 */
static esp_err_t (*cached_gptimer_stop)(gptimer_handle_t timer) = gptimer_stop;

/**
 * @brief Cached function pointer for gpio_isr_handler_remove.
 *
 * This function pointer is used to remove the GPIO interrupt service routine handler.
 */
static esp_err_t (*cached_gpio_isr_handler_remove)(gpio_num_t) = gpio_isr_handler_remove;

/**
 * @brief Cached function pointer for gpio_isr_handler_add.
 *
 * This function pointer is used to add a GPIO interrupt service routine handler.
 */
static esp_err_t (*cached_gpio_isr_handler_add)(gpio_num_t, gpio_isr_t, void*) = gpio_isr_handler_add;

/**
 * @brief Cached function pointer for ringBufferPush.
 *
 * This function pointer is used to push a value into the ring buffer.
 */
static bool (*cached_ring_buffer_push)(RingBuffer* rb, volatile uint32_t value) = ringBufferPush;

/**
 * @brief Splits a 32-bit unsigned integer into four bytes.
 * 
 * @param value The 32-bit unsigned integer to split.
 * @param b0 Pointer to the least significant byte.
 * @param b1 Pointer to the second byte.
 * @param b2 Pointer to the third byte.
 * @param b3 Pointer to the most significant byte.
 */
static void splitUint32ToChars(uint32_t value, unsigned char* b0, unsigned char* b1, unsigned char* b2, unsigned char* b3) {
    *b0 = (unsigned char)(value & 0xFF);
    *b1 = (unsigned char)((value >> 8) & 0xFF);
    *b2 = (unsigned char)((value >> 16) & 0xFF);
    *b3 = (unsigned char)((value >> 24) & 0xFF);
}

/**
 * @brief Checks if a string contains only printable ASCII characters.
 * 
 * @param str The string to check.
 * @param length The length of the string.
 * @return true if the string is printable, false otherwise.
 */
static bool is_printable_string(const char* str, size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (!isprint((unsigned char)str[i])) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Recursively processes the ring buffer and appends data to the output string.
 * 
 * @param output_str The output string to store the processed data.
 * @param hex_str The string to store the hex representation.
 * @param max_length The maximum length of the output string.
 * @param current_length Pointer to the current length of the output string.
 * @param hex_length Pointer to the current length of the hex string.
 * @return true if the buffer was processed successfully, false otherwise.
 */
static bool process_buffer_recursive(char* output_str, char* hex_str, size_t max_length, size_t* current_length, size_t* hex_length) {
    uint32_t value;
    if (ringBufferPop(ring_buffer_RX, &value)) {
        value ^= key_generator(&RX_encryption_vars);
        unsigned char b0, b1, b2, b3;
        splitUint32ToChars(value, &b0, &b1, &b2, &b3);
        
        // Append bytes to the output string
        if (*current_length < max_length - 1) output_str[(*current_length)++] = b0;
        if (*current_length < max_length - 1) output_str[(*current_length)++] = b1;
        if (*current_length < max_length - 1) output_str[(*current_length)++] = b2;
        if (*current_length < max_length - 1) output_str[(*current_length)++] = b3;
        
        // Append hex representation
        *hex_length += snprintf(hex_str + *hex_length, max_length - *hex_length, 
                                "%02X%02X%02X%02X ", b0, b1, b2, b3);
        
        // Recursively process the next value
        return process_buffer_recursive(output_str, hex_str, max_length, current_length, hex_length);
    }
    return false; // Ring buffer is empty
}

/**
 * @brief Processes the received data and logs the result.
 */
static void process_reception_complete(void) {
    char output_str[BUFFER_MAX_SIZE*4 + 1]; 
    char hex_str[BUFFER_MAX_SIZE*9 + 1];  // 8 chars per uint32 (2 per byte) + space
    size_t current_length = 0;
    size_t hex_length = 0;
    
    process_buffer_recursive(output_str, hex_str, sizeof(output_str), &current_length, &hex_length);
    
    // Null-terminate the strings
    output_str[current_length] = '\0';
    hex_str[hex_length] = '\0';
    
    // Print the hex representation
    ESP_LOGI(RX_TAG, "Received (HEX): %s", hex_str);
    
    // Check if the string is printable and print if it is
    if (is_printable_string(output_str, current_length)) {
        ESP_LOGI(RX_TAG, "Received (ASCII): %s", output_str);
    } else {
        ESP_LOGI(RX_TAG, "Received data contains non-printable characters");
    }
}

/**
 * @brief Checks the reception buffer size and processes received data if not empty.
 */
static void check_RX(void) {
    if (reception_complete) {
        process_reception_complete();
        reception_complete = false;
    }
}

/**
 * @brief ISR for the GPIO used in reception.
 * 
 * This ISR starts the reception timer and resets the reception value and bit counter.
 * 
 * @param arg User-provided argument (unused).
 */
static void IRAM_ATTR RX_gpio_ISR(void* arg) {
    
    value_RX = 0; // Reset the value_RX for the next reception
    bit_counter_RX = 0; // Reset the bit_counter_RX for the next reception
    cached_gptimer_start(timer_RX);
    cached_gpio_isr_handler_remove(RX_GPIO_PIN_NUM);
}

/**
 * @brief ISR for the reception timer.
 * 
 * This ISR reads the GPIO state and updates the reception value and bit counter.
 * 
 * @param timer Timer handle.
 * @param edata Pointer to alarm event data.
 * @param arg User-provided argument (unused).
 * @return true if the ISR should yield, false otherwise.
 */
static bool IRAM_ATTR timer_RX_ISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *arg) {
    if (__builtin_expect(bit_counter_RX != 32, 1)) {
        // Common case: bit_counter_RX is not 32
        value_RX |= (gpioDirectRead() << bit_counter_RX);
        bit_counter_RX++;
    } else {
        // Less common case: bit_counter_RX is 32
        cached_gptimer_stop(timer_RX);
        cached_gpio_isr_handler_add(RX_GPIO_PIN_NUM, RX_gpio_ISR, NULL);
        cached_ring_buffer_push(ring_buffer_RX, value_RX);
        reception_complete=true;
    }
    
    return true;
}

/**
 * @brief Sets up the GPIO for reception.
 * 
 * This function configures the GPIO pin used for reception and sets up
 * a dedicated GPIO bundle for efficient control.
 */
static void setup_gpio_RX(void) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .pin_bit_mask = RX_GPIO_PIN_SEL,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = 1,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    const int reception_bundle_pins[] = {RX_GPIO_PIN_NUM};
    dedic_gpio_bundle_handle_t reception_bundle = NULL;
    dedic_gpio_bundle_config_t reception_bundle_config = {
        .gpio_array = reception_bundle_pins,
        .array_size = sizeof(reception_bundle_pins) / sizeof(reception_bundle_pins[0]),
        .flags = {
            .in_en = 1,
        },
    };
    ESP_ERROR_CHECK(dedic_gpio_new_bundle(&reception_bundle_config, &reception_bundle));
    ESP_ERROR_CHECK(gpio_install_isr_service(INTR_LEVEL));
    ESP_LOGI(RX_TAG, "Reception GPIO Setup Complete");
}

/**
 * @brief Sets up the timer for reception.
 * 
 * This function configures and initializes the timer used for reception timing.
 */
static void setup_timer_RX(void) {
    gptimer_config_t reception_timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION_HZ, // 1MHz, 1 tick = 1us
        .intr_priority = TIMER_INTERRUPTION_PRIORITY,
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&reception_timer_config, &timer_RX));

    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0, // counter will reload with 0 on alarm event
        .alarm_count = RX_PERIOD_MICROS, // period
        .flags.auto_reload_on_alarm = true, // enable auto-reload
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer_RX, &alarm_config));

    gptimer_event_callbacks_t call_back_timer = {
        .on_alarm = timer_RX_ISR, // register user callback
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer_RX, &call_back_timer, NULL));

    ESP_ERROR_CHECK(gptimer_enable(timer_RX));
    ESP_LOGI(RX_TAG, "Reception Timer Setup Complete");
}


/**
 * @brief RX control task.
 *
 * This task sets up the GPIO for reception, initializes the reception timer,
 * waits for encryption values to be set, and then enters a loop to manage the reception buffer.
 *
 * @param pvParameters Pointer to task parameters (unused).
 */
void RX_control_task(void *pvParameters) {
    setup_gpio_RX();
    setup_timer_RX();
    ring_buffer_RX = createRingBuffer();
    if (ring_buffer_RX == NULL) {
        ESP_LOGE(RX_TAG, "Failed to create RX ring buffer");
        vTaskDelete(NULL);
    }
    ESP_LOGW(RX_TAG, "Need to set encryption values for reception and transmission before proceeding");
    while(!rx_encryption_set){
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    gpio_isr_handler_add(RX_GPIO_PIN_NUM, RX_gpio_ISR, NULL);
    ESP_LOGI(RX_TAG,"ENTERING RX LOOP");   
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        check_RX();   
    }
}