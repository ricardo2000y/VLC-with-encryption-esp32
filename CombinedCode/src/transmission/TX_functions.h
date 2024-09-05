/**
 * @file TX_functions.h
 * @author Ricardo Jorge Dias Sampaio
 * @brief Header file for transmission for the Secure VLC Project.
 * @version 1.0
 * @date 2024-09-03
 * \par License:
 *   \ref mit_license "MIT License".
 * 
 * @details This file contains the declarations of functions and variables used for data transmission,
 * including buffer management, GPIO setup, and timer configuration for the Secure VLC Project.
 */

#ifndef TX_FUNCTIONS_H
#define TX_FUNCTIONS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "esp_log.h"
#include "driver/dedic_gpio.h"
#include "driver/gptimer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "common_utils/config.h"
#include "common_utils/encryption.h"
#include "common_utils/gpio_direct_RW.h"
#include "common_utils/ring_buffer.h"
#include "console/console_commands.h"

/**
 * @brief Structure to store the encryption variables for transmission.
 */
extern encryption_vars_t TX_encryption_vars;

/**
 * @brief Adds a string to the transmission buffer.
 * 
 * This function takes an input string, processes it in 4-byte chunks,
 * applies encryption, and pushes the resulting values to the ring buffer.
 * 
 * @param input_str The input string to be added to the buffer
 */
void add_str_to_buffer(const char* input_str);

/**
 * @brief TX control task.
 *
 * This task sets up the GPIO for transmission, initializes the transmission timer,
 * waits for encryption values to be set, and then enters a loop to manage the transmission buffer.
 *
 * @param pvParameters Pointer to task parameters (unused).
 */
void TX_control_task(void *pvParameters);

#endif /* TX_FUNCTIONS_H */
