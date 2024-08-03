/**
 * @file config.h
 * @brief Project-wide configuration constants and defines
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "driver/gpio.h"
#include "soc/gpio_periph.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"


/**
 * @brief Define the ALPHA constant for the Duffing map.
 * @details Sets the value of the DUFFING_ALPHA constant/macro to 2.75.
 */
#define DUFFING_ALPHA 2.75
/**
 * @brief Define the BETA constant for the Duffing map.
 * @details Sets the value of the DUFFING_BETA constant/macro to 0.2.
 */
#define DUFFING_BETA 0.2

/** @brief GPIO pin number used for output */
#define TX_GPIO_PIN_NUM       GPIO_NUM_6

/**
 * @brief GPIO pin mask for the LED bundle.
 * @note Pin is the first pin in the bundle.
 */
#define TX_GPIO_PIN_SEL (1ULL << TX_GPIO_PIN_NUM)

/**
 * @brief LED toggling period in microseconds.
 */
#define TX_PERIOD_MICROS 20

/**
 * @brief Timer resolution in Hz.
 */
#define TIMER_RESOLUTION_HZ 1000000

/** @brief GPIO pin number used for input */
#define RX_GPIO_PIN_NUM       GPIO_NUM_7

/** @brief GPIO pin selection bitmask for the input pin
 *  This creates a 64-bit bitmask with only the RX_GPIO_PIN_NUM bit set
 */
#define RX_GPIO_PIN_SEL  (1ULL << RX_GPIO_PIN_NUM)

/** @brief Default interrupt flag
 *  0 means default interrupt flag, no special behavior
 */
#define INTR_LEVEL ESP_INTR_FLAG_LEVEL2

#define TIMER_INTERRUPTION_PRIORITY 3

/** @brief Period for reception in microseconds
 *  This is set to be the same as the LED period
 */
#define RX_PERIOD_MICROS TX_PERIOD_MICROS

#define RX_DELAY_PERIOD_MICROS RX_PERIOD_MICROS/4

/**
 * @brief Maximum size of the ring buffer.
 */
#define BUFFER_MAX_SIZE 32

#endif // CONFIG_H