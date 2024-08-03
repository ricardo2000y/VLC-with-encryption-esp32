/**
 * @file TX_functions.c
 * @brief Combined implementation of transmission, data processing, and LED control functions.
 * @date August 2, 2024
 */

#include "TX_functions.h"

RingBuffer* ring_buffer_TX = NULL;
//gptimer_handle_t timer_TX = NULL;
volatile bool in_transmission = false;
const char* TAG1 = "TX";
volatile uint32_t value_TX = 0;
volatile uint8_t bit_counter_TX = 0;

encryption_vars_t TX_encryption_vars = {
    .duffing_map_1 = {
        .x =0.1,
        .y =1.1,
    },
    .iterations_map1 = 10000,
    .duffing_map_2= {
        .x =0.5,
        .y =0.89,
    },
    .iterations_map2 = 10000,
    .msws32_input ={
        .x =0,
        .w =0,
        .s =0,
    },
};

uint32_t combineCharsToUint32(unsigned char b0, unsigned char b1, unsigned char b2, unsigned char b3) {
    return  ((uint32_t)b3 << 24) | 
            ((uint32_t)b2 << 16) | 
            ((uint32_t)b1 << 8)  | 
            (uint32_t)b0;
}

void start_transmission() {
    ringBufferPop(ring_buffer_TX, &value_TX);
    ESP_LOGI(TAG1, "Transmitting :%" PRIx32, value_TX);
    directWriteLow_single();
    bit_counter_TX = 0;
    gptimer_start(timer_TX);
}

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
                                    key_generator(&TX_encryption_vars.duffing_map_1,&TX_encryption_vars.msws32_input);
        ringBufferPush(ring_buffer_TX, value);
    }
}

void get_data_from_user(void) {
    char *input_str = "Hello World!";
    add_str_to_buffer(input_str);
}

void check_TX_buffer_size() {
    if ((!ringBufferIsEmpty(ring_buffer_TX)) & (!in_transmission)) {
        in_transmission = true;
        start_transmission();
    }
}

bool IRAM_ATTR timer_TX_ISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *arg) {
    if (__builtin_expect(bit_counter_TX < 32, 1)) {
        ((value_TX >> bit_counter_TX) & 0x1) ? directWriteHigh_single() : directWriteLow_single();
    } else if (bit_counter_TX == 32) {
        directWriteHigh_single();
    } else {
        gptimer_stop(timer_TX);
        in_transmission = false;
        return true;
    }
    bit_counter_TX++;
    return true;
}

void setup_timer_TX() {
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

void setup_gpio_TX(void) {
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
    ESP_LOGI(TAG1, "GPIO Setup Complete");
}