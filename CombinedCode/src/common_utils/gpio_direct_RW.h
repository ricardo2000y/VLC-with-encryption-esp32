/**
 * @file gpio_direct_RW.h
 * @brief Header containing the functions needed for "gpio_direct_RW.c".
 * @details Header containing the functions needed for "gpio_direct_RW.c", including commentaries.
 * @version 0.1
 * @date 2024-07-29
 * 
 * @note This header is part of a high-performance GPIO manipulation module.
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef GPIO_DIRECT_RW_H
#define GPIO_DIRECT_RW_H

#include <stdint.h>
#include "esp_system.h"

/**
 * @brief Set a single GPIO pin to high state.
 *
 * This function uses an assembly instruction to directly set a GPIO pin high,
 * providing the fastest possible pin manipulation.
 *
 * @note This function is always inlined for maximum performance.
 */
inline __attribute__((always_inline))
void IRAM_ATTR directWriteHigh_single(void);

/**
 * @brief Set a single GPIO pin to low state.
 *
 * This function uses an assembly instruction to directly set a GPIO pin low,
 * providing the fastest possible pin manipulation.
 *
 * @note This function is always inlined for maximum performance.
 */
inline __attribute__((always_inline))
void IRAM_ATTR directWriteLow_single(void);

/**
 * @brief Read the state of GPIO pins directly.
 *
 * This function uses an assembly instruction to read the current state
 * of all GPIO pins in a single operation.
 *
 * @return uint32_t A 32-bit value representing the state of all GPIO pins.
 * @note This function is always inlined for maximum performance.
 */
inline __attribute__((always_inline))
uint32_t IRAM_ATTR gpioDirectRead(void);

// Function definitions

inline __attribute__((always_inline))
void directWriteHigh_single(void) {
    asm volatile ("EE.SET_BIT_GPIO_OUT %0" :: "I"(0x1) : );
}

inline __attribute__((always_inline))
void directWriteLow_single(void) {
    asm volatile ("EE.CLR_BIT_GPIO_OUT %0" :: "I"(0x1) : );
}

inline __attribute__((always_inline))
uint32_t gpioDirectRead(void)
{
    uint32_t read_value = 0;
    asm volatile("ee.get_gpio_in %0" : "=r"(read_value) : :);
    return read_value;
}

#endif // GPIO_DIRECT_RW_H