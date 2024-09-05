/**
 * @file console_commands.h
 * @author Ricardo Jorge Dias Sampaio
 * @brief Header file for console commands in the Secure VLC Project.
 * @version 1.0
 * @date 2024-09-03
 *
 * \par License:
 *   \ref mit_license "MIT License".
 * @details This file contains declarations for public functions and variables related to
 * the console command system in the Secure VLC Project.
 */

#ifndef CONSOLE_COMMANDS_H
#define CONSOLE_COMMANDS_H

#include <stdarg.h>
#include "argtable3/argtable3.h"
#include "esp_console.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "common_utils/config.h"
#include "common_utils/encryption.h"
#include "reception/RX_functions.h"
#include "transmission/TX_functions.h"

/**
 * @brief Global flag to track if TX encryption values have been set.
 */
extern bool tx_encryption_set;

/**
 * @brief Global flag to track if RX encryption values have been set.
 */
extern bool rx_encryption_set;

/**
 * @brief Task for initializing and managing the REPL console and logging.
 *
 * This task initializes NVS, sets up custom logging, initializes the console,
 * and starts the REPL.
 *
 * @param pvParameters Pointer to task parameters (not used in this case)
 */
void console_and_logging_task(void *pvParameters);

#endif // CONSOLE_COMMANDS_H
