/**
 * @file RX_functions.h
 * @brief Combined header file for reception, GPIO handling, and data processing functions.
 * @author Your Name
 * @date August 2, 2024
 */

#ifndef RX_FUNCTIONS_H
#define RX_FUNCTIONS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "esp_log.h"
#include "driver/gptimer.h"
#include "driver/gpio.h"
#include "driver/dedic_gpio.h"
#include "common_utils/config.h"
#include "common_utils/gpio_direct_RW.h"
#include "common_utils/ring_buffer.h"
#include "common_utils/duffing_msws32.h"

// Declare function pointer types
typedef esp_err_t (*gptimer_start_func_t)(gptimer_handle_t timer);
typedef esp_err_t (*gptimer_stop_func_t)(gptimer_handle_t timer);


extern RingBuffer* ring_buffer_RX; /**< Pointer to the ring buffer for received data */
extern gptimer_handle_t timer_RX; /**< Handle for the reception timer */
extern gptimer_handle_t delay_timer_RX; /**< Handle for the delay timer */
extern const char *TAG2; /**< Tag for logging messages */
extern volatile uint32_t value_RX; /**< Variable to store the read value from GPIO */
extern volatile uint8_t bit_counter_RX; /**< Bit counter for the current value */
extern encryption_vars_t RX_encryption_vars; /**< structure to store the encryption variables for reception */
/**
 * @brief Splits a uint32_t into 4 chars.
 * 
 * @param value The 32-bit unsigned integer to split.
 * @param b0 Pointer to store the least significant byte.
 * @param b1 Pointer to store the second byte.
 * @param b2 Pointer to store the third byte.
 * @param b3 Pointer to store the most significant byte.
 */
void splitUint32ToChars(uint32_t value, unsigned char* b0, unsigned char* b1, unsigned char* b2, unsigned char* b3);

/**
 * @brief Recursively processes the ring buffer and builds the output string.
 * 
 * @param output_str Pointer to the buffer where the reconstructed string will be stored.
 * @param max_length The maximum length of the output string buffer.
 * @param current_length Pointer to the current length of the output string.
 * @return bool True if bytes were successfully processed, false if the ring buffer is empty.
 */
bool process_buffer_recursive(char* output_str, size_t max_length, size_t* current_length);

/**
 * @brief Checks if the RX buffer size is bigger than 0 and processes the data if so.
 */
void check_RX_buffer_size(void);

/**
 * @brief Processes the data from the RX buffer.
 */
void process_data(void);

/**
 * @brief GPIO ISR handler.
 * @param arg Pointer to arguments (unused).
 */
void IRAM_ATTR RX_gpio_ISR(void* arg);

/**
 * @brief Delay Timer ISR for GPIO reception.
 * @param timer Handle of the timer that triggered the interrupt.
 * @param edata Pointer to the alarm event data.
 * @param arg User-provided argument (unused).
 */

bool IRAM_ATTR timer_delay_RX_ISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *arg);

/**
 * @brief Timer ISR for GPIO reception.
 * @param timer Handle of the timer that triggered the interrupt.
 * @param edata Pointer to the alarm event data.
 * @param arg User-provided argument (unused).
 */
bool IRAM_ATTR timer_RX_ISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *arg);

/**
 * @brief Setup function to initialize GPIO for reception.
 */
void setup_gpio_RX(void);

/**
 * @brief Setup function to initialize the reception timer.
 */
void setup_timer_RX(void);

/**
 * @brief Setup function to initialize the delay timer for reception.
 */
void setup_delay_timer_RX(void);


#endif /* RX_FUNCTIONS_H */