/**
 * @file config.h
 * @author Ricardo Jorge Dias Sampaio
 * @brief Project-wide configuration constants and defines for the Secure VLC Project
 * @version 1.0
 * @date 2024-09-03 
 * \par License:
 *   \ref mit_license "MIT License".
 * @details This file contains various configuration parameters and constants used throughout the Secure VLC Project,
 * including GPIO pin assignments, timer settings, and buffer sizes.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "driver/gpio.h"
#include "soc/gpio_periph.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"


// GPIO Configuration
/**
 * @brief GPIO pin number used for transmission (TX) output.
 *
 * This macro defines the GPIO pin number used for the TX output in the project.
 */
#define TX_GPIO_PIN_NUM       GPIO_NUM_6

/**
 * @brief GPIO pin mask for the TX bundle.
 *
 * This macro creates a bitmask for the TX GPIO pin. The pin is the first in the bundle.
 */
#define TX_GPIO_PIN_SEL (1ULL << TX_GPIO_PIN_NUM)

/**
 * @brief GPIO pin number used for reception (RX) input.
 *
 * This macro defines the GPIO pin number used for the RX input in the project.
 */
#define RX_GPIO_PIN_NUM       GPIO_NUM_7

/**
 * @brief GPIO pin selection bitmask for the RX input pin.
 *
 * This macro creates a 64-bit bitmask with only the RX_GPIO_PIN_NUM bit set.
 */
#define RX_GPIO_PIN_SEL  (1ULL << RX_GPIO_PIN_NUM)

// Timer Configuration
/**
 * @brief TX toggling period in microseconds.
 *
 * This macro defines the period for toggling the TX signal in microseconds.
 */
#define TX_PERIOD_MICROS 20

/**
 * @brief Period for reception in microseconds.
 *
 * This macro defines the period for reception, set to be the same as the TX period.
 */
#define RX_PERIOD_MICROS TX_PERIOD_MICROS

/**
 * @brief Timer resolution in Hz.
 *
 * This macro sets the timer resolution in Hertz for the project.
 */
#define TIMER_RESOLUTION_HZ 1000000

/**
 * @brief Timer interrupt priority.
 *
 * This macro sets the priority level for timer interrupts in the project.
 */
#define TIMER_INTERRUPTION_PRIORITY 3

// Interrupt Configuration
/**
 * @brief Default interrupt flag.
 *
 * This macro defines the default interrupt flag for the project.
 */
#define INTR_LEVEL ESP_INTR_FLAG_LEVEL2

// Buffer and Console Configuration
/**
 * @brief Maximum size of the ring buffer.
 *
 * This macro defines the maximum number of elements that can be stored in the ring buffer.
 */
#define BUFFER_MAX_SIZE 128

/**
 * @brief Maximum length of a command line in the console.
 *
 * This macro defines the maximum length of a command line, set to four times the buffer size.
 */
#define MAX_CMDLINE_LENGTH BUFFER_MAX_SIZE * 4

/**
 * @brief Maximum length of data that can be processed.
 *
 * This macro defines the maximum length of data that can be processed, set to be the same as the maximum command line length.
 */
#define MAX_DATA_LENGTH MAX_CMDLINE_LENGTH

/**
 * @brief Prompt string for the console.
 *
 * This macro defines the prompt string to be displayed in the console.
 */
#define PROMPT_STR CONFIG_IDF_TARGET " >"

// Task Configuration
/**
 * @brief Defines the core on which the Console and Logging task will run.
 *
 * This macro specifies the core number for the Console and Logging task execution.
 * It's used to pin the Console and Logging task to a specific core for better performance management.
 */
#define CONSOLE_TASK_CORE 0

/**
 * @brief Defines the stack size in bytes for the Console and Logging task.
 *
 * This macro sets the stack size in bytes allocated for the Console and Logging task.
 * Ensure this value is sufficient for the task's memory requirements.
 */
#define CONSOLE_STACK_SIZE 16384

/**
 * @brief Defines the core on which the TX task will run.
 *
 * This macro specifies the core number for the TX task execution.
 * It's used to pin the TX task to a specific core for better performance management.
 */
#define TX_TASK_CORE 0

/**
 * @brief Defines the stack size in bytes for the TX task.
 *
 * This macro sets the stack size in bytes allocated for the TX task.
 * Ensure this value is sufficient for the task's memory requirements.
 */
#define TX_STACK_SIZE 16384

/**
 * @brief Defines the core on which the RX task will run.
 *
 * This macro specifies the core number for the RX task execution.
 * It's used to pin the RX task to a specific core, separate from the TX task.
 */
#define RX_TASK_CORE 1

/**
 * @brief Defines the stack size in bytes for the RX task.
 *
 * This macro sets the stack size in bytes allocated for the RX task.
 * Ensure this value is sufficient for the task's memory requirements.
 */
#define RX_STACK_SIZE 16384

#endif // CONFIG_H