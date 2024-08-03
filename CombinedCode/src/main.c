/**
 * @file main.c
 * @brief Main application file for GPIO and LED control.
 * @details This file contains the main entry point of the application and the LED control task.
 * @version 0.1
 * @date 2024-07-29
 * 
 * @note This example code is in the Public Domain (or CC0 licensed, at your option.)
 */

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <esp_task_wdt.h>
#include "driver/gptimer.h"
#include "transmission/TX_functions.h"
#include "reception/RX_functions.h"
#include "common_utils/gpio_direct_RW.h"
#include "common_utils/duffing_msws32.h"



gptimer_handle_t timer_TX = NULL;
gptimer_handle_t delay_timer_RX = NULL;
gptimer_handle_t timer_RX = NULL;

/**
 * @brief LED control task.
 * @param pvParameters Pointer to task parameters (unused).
 */
void TX_control_task(void *pvParameters) {
    
    uint8_t hello_counter =0; //!temporary 
    // Setup the LED timer and GPIO
    setup_gpio_TX();
    setup_timer_TX();
    directWriteHigh_single();
    
    //esp_task_wdt_deinit();// temporary, disabling watchdog 
    get_data_from_user();
    ESP_LOGI(TAG1,"ENTERING LOOP");
    // Main loop for the LED control task
    while (1) {
        // Add your LED control logic here
        if((ringBufferIsEmpty(ring_buffer_TX))&(hello_counter<100)){
            get_data_from_user();
            hello_counter++;
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Wait for 1 second
        check_TX_buffer_size();
    }
}

/**
 * @brief Main entry point of the application.
 */
void app_main(void)
{
    ring_buffer_RX = createRingBuffer();
    ring_buffer_TX = createRingBuffer();
    if((ring_buffer_RX ==NULL) | (ring_buffer_TX ==NULL)){
        ESP_LOGE(TAG2, "Failed to create ring buffer");
        while(1){}
    }
    setup_timer_RX();
    //setup_delay_timer_RX();
    setup_gpio_RX();

    gpio_isr_handler_add(RX_GPIO_PIN_NUM, RX_gpio_ISR, NULL);
    
    key_generator_setup(&RX_encryption_vars.duffing_map_1,RX_encryption_vars.iterations_map1,
                        &RX_encryption_vars.duffing_map_2,RX_encryption_vars.iterations_map2,
                        &RX_encryption_vars.msws32_input);
    
    key_generator_setup(&TX_encryption_vars.duffing_map_1,TX_encryption_vars.iterations_map1,
                        &TX_encryption_vars.duffing_map_2,TX_encryption_vars.iterations_map2,
                        &TX_encryption_vars.msws32_input);
    
    vTaskDelay(pdMS_TO_TICKS(100)); // Wait for 1 second
    xTaskCreatePinnedToCore(TX_control_task, "TX CONTROL Task", 8192, NULL, 1, NULL, 1);
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
        check_RX_buffer_size();   
    }
}