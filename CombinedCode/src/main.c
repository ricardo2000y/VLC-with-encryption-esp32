/**
 * @file main.c
 * @author Ricardo Jorge Dias Sampaio
 * @brief Main application file for the Secure VLC Project.
 * @version 1.0
 * @date 2024-09-03
 * 
 * \par License:
 *   \ref mit_license "MIT License".
 * @details This file contains the main application logic, including LED control and reception tasks.
 */

#include <esp_task_wdt.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "console/console_commands.h"
#include "reception/RX_functions.h"
#include "transmission/TX_functions.h"


/** @brief Tag for logging messages related to console operations */
static const char *MAIN_TAG = "MAIN";

/**
 * @brief Main entry point of the application.
 *
 * This function initializes the system, sets up the console, creates ring buffers,
 * starts the transmission and reception control tasks.
 */
void app_main(void)
{
    esp_task_wdt_deinit();  // Temporarily disabling watchdog
    xTaskCreatePinnedToCore(console_and_logging_task, "Console & Logging Task", CONSOLE_STACK_SIZE, NULL, 1, NULL, CONSOLE_TASK_CORE);
    xTaskCreatePinnedToCore(TX_control_task, "TX CONTROL Task", TX_STACK_SIZE, NULL, 1, NULL, TX_TASK_CORE);
    xTaskCreatePinnedToCore(RX_control_task, "RX CONTROL Task", RX_STACK_SIZE, NULL, 1, NULL, RX_TASK_CORE);

    ESP_LOGW(MAIN_TAG, "TX and RX tasks created. Waiting for encryption values to be set.");

    // Main loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // Sleep for 1 second
        // You can add any main loop logic here if needed
    }
}

