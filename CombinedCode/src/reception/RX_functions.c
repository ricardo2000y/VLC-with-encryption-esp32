/**
 * @file RX_functions.c
 * @brief Combined implementation of reception, GPIO handling, and data processing functions.
 * @author Your Name
 * @date August 2, 2024
 */

#include "RX_functions.h"


RingBuffer* ring_buffer_RX = NULL;
//gptimer_handle_t delay_timer_RX = NULL;
//gptimer_handle_t timer_RX = NULL;
const char *TAG2 = "RX";
volatile uint32_t value_RX = 0;
volatile uint8_t bit_counter_RX = 0;

// Cache the function pointers
volatile gptimer_start_func_t cached_gptimer_start = gptimer_start;
volatile gptimer_stop_func_t cached_gptimer_stop = gptimer_stop;

encryption_vars_t RX_encryption_vars = {
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


void splitUint32ToChars(uint32_t value, unsigned char* b0, unsigned char* b1, unsigned char* b2, unsigned char* b3) {
    *b0 = (unsigned char)(value & 0xFF);
    *b1 = (unsigned char)((value >> 8) & 0xFF);
    *b2 = (unsigned char)((value >> 16) & 0xFF);
    *b3 = (unsigned char)((value >> 24) & 0xFF);
    
}

bool process_buffer_recursive(char* output_str, size_t max_length, size_t* current_length) {
    uint32_t value;
    if (ringBufferPop(ring_buffer_RX, &value)) {
        value^= key_generator(&RX_encryption_vars.duffing_map_1,&RX_encryption_vars.msws32_input);
        unsigned char b0, b1, b2, b3;
        splitUint32ToChars(value, &b0, &b1, &b2, &b3);
        
        // Append non-zero bytes to the output string
        if (b0 && *current_length < max_length - 1) output_str[(*current_length)++] = b0;
        if (b1 && *current_length < max_length - 1) output_str[(*current_length)++] = b1;
        if (b2 && *current_length < max_length - 1) output_str[(*current_length)++] = b2;
        if (b3 && *current_length < max_length - 1) output_str[(*current_length)++] = b3;
        
        // Recursively process the next value
        return process_buffer_recursive(output_str, max_length, current_length);
    }
    return false; // Ring buffer is empty
}

void check_RX_buffer_size(void) {
    if (!ringBufferIsEmpty(ring_buffer_RX)) {
        process_data();
    }
    
}

void process_data(void) {
    char output_str[BUFFER_MAX_SIZE*4 + 1]; 
    size_t current_length = 0;
    
    process_buffer_recursive(output_str, sizeof(output_str), &current_length);
    
    // Null-terminate the string
    output_str[current_length] = '\0';
    
    // Print the entire string
    ESP_LOGI(TAG2, "Received: %s", output_str);
    
}

void RX_gpio_ISR(void* arg) {
    //gptimer_start(delay_timer_RX);
    //cached_gptimer_start(delay_timer_RX);
    cached_gptimer_start(timer_RX);

    gpio_isr_handler_remove(RX_GPIO_PIN_NUM);
    
}

bool timer_delay_RX_ISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *arg) {   
    //gptimer_start(timer_RX);
    //gptimer_stop(delay_timer_RX);
    cached_gptimer_start(timer_RX);
    cached_gptimer_stop(delay_timer_RX);
    return true;
}

bool timer_RX_ISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *arg) {
    if (__builtin_expect(bit_counter_RX != 32, 1)) {
        // Common case: bit_counter_RX is not 32
        value_RX |= (gpioDirectRead()<<bit_counter_RX);
        bit_counter_RX++;
    } else {
        // Less common case: bit_counter_RX is 32
        //gptimer_stop(timer_RX);
        cached_gptimer_stop(timer_RX);
        bit_counter_RX = 0;
        ringBufferPush(ring_buffer_RX, value_RX);
        value_RX = 0; // Reset the value for the next reception
        gpio_isr_handler_add(RX_GPIO_PIN_NUM, RX_gpio_ISR, NULL);
    }
    
    return true;
}

void setup_gpio_RX(void) {
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
    ESP_LOGI(TAG2, "Reception GPIO Setup Complete");
    
}

void setup_timer_RX(void) {
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
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer_RX, &call_back_timer,NULL));

    ESP_ERROR_CHECK(gptimer_enable(timer_RX));
    ESP_LOGI(TAG2, "Reception Timer Setup Complete");
    
}

void setup_delay_timer_RX(void) {
    gptimer_config_t reception_timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION_HZ, // 1MHz, 1 tick = 1us
        .intr_priority = TIMER_INTERRUPTION_PRIORITY,
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&reception_timer_config, &delay_timer_RX));

    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0, // counter will reload with 0 on alarm event
        .alarm_count = RX_DELAY_PERIOD_MICROS, // period
        .flags.auto_reload_on_alarm = true, // enable auto-reload
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(delay_timer_RX, &alarm_config));

    gptimer_event_callbacks_t call_back_timer = {
        .on_alarm = timer_delay_RX_ISR, // register user callback
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(delay_timer_RX, &call_back_timer,NULL));

    ESP_ERROR_CHECK(gptimer_enable(delay_timer_RX));
    ESP_LOGI(TAG2, "Reception Delay Timer Setup Complete");
}