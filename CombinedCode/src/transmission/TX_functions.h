/**
 * @file TX_functions.h
 * @brief Combined header file for transmission, data processing, and LED control functions.
 * @date August 2, 2024
 */

#ifndef TX_FUNCTIONS_H
#define TX_FUNCTIONS_H

#include <stdint.h>
#include <string.h>
#include "driver/gptimer.h"
#include "common_utils/config.h"
#include "common_utils/gpio_direct_RW.h"
#include "common_utils/ring_buffer.h"
#include "common_utils/duffing_msws32.h"
#include "driver/gpio.h"
#include "soc/gpio_periph.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/dedic_gpio.h"

extern RingBuffer* ring_buffer_TX; /**< Pointer to the ring buffer for transmitted data */
extern gptimer_handle_t timer_TX;  /**< Handle to the GPTimer used for transmission */
extern volatile bool in_transmission; /**< Flag to indicate if a transmission is in progress */
extern const char* TAG1; /**< Tag for logging messages related to LED operations */
extern volatile uint32_t value_TX; /**< Value currently being transmitted */
extern volatile uint8_t bit_counter_TX; /**< Bit counter for the current value */
extern encryption_vars_t TX_encryption_vars; /**< Structure to store the encryption variables for transmission */
/**
 * @brief Combines 4 chars into a uint32_t.
 * 
 * @param b0 The least significant byte.
 * @param b1 The second byte.
 * @param b2 The third byte.
 * @param b3 The most significant byte.
 * @return uint32_t The combined 32-bit unsigned integer.
 */
uint32_t combineCharsToUint32(unsigned char b0, unsigned char b1, unsigned char b2, unsigned char b3);

/**
 * @brief Starts the transmission process.
 */
void start_transmission();

/**
 * @brief Adds a string to the ring buffer in 4-byte chunks, padding with zeros if necessary.
 * 
 * @param input_str The input string to be added to the ring buffer.
 */
void add_str_to_buffer(const char* input_str);

/**
 * @brief Function to get data from the user.
 */
void get_data_from_user(void);

/**
 * @brief Checks if the TX buffer size is greater than 0 and starts a transmission if so.
 */
void check_TX_buffer_size();

/**
 * @brief Timer ISR to toggle LED state.
 * 
 * @param timer Handle of the timer that triggered the interrupt.
 * @param edata Pointer to the alarm event data.
 * @param arg User-provided argument (unused in this case).
 * @return true if the ISR was executed successfully.
 */
bool timer_TX_ISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *arg);

/**
 * @brief Setup function to initialize the LED timer.
 */
void setup_timer_TX();

/**
 * @brief Setup function to initialize GPIO for LED control.
 */
void setup_gpio_TX(void);

#endif /* TX_FUNCTIONS_H */