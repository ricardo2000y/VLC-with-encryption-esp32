/**
 * @file RX_functions.h
 * @author Ricardo Jorge Dias Sampaio
 * @brief Header file for reception for the Secure VLC Project. 
 * @version 1.0
 * @date 2024-09-03
 * \par License:
 *   \ref mit_license "MIT License".
 * 
 * @details This file contains the declarations of functions and variables used for data reception,
 * including buffer management, GPIO setup, and timer configuration for the Secure VLC Project.
 */

#ifndef RX_FUNCTIONS_H
#define RX_FUNCTIONS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
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


/** @brief Structure to store the encryption variables for reception */
extern encryption_vars_t RX_encryption_vars;

/**
 * @brief RX control task.
 *
 * This task sets up the GPIO for reception, initializes the reception timer,
 * waits for encryption values to be set, and then enters a loop to manage the reception buffer.
 *
 * @param pvParameters Pointer to task parameters (unused).
 */
void RX_control_task(void *pvParameters);

#endif /* RX_FUNCTIONS_H */
