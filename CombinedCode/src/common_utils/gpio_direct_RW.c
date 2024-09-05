/**
 * @file gpio_direct_RW.c
 * @author Ricardo Jorge Dias Sampaio
 * @brief Implementation of direct GPIO read/write operations.
 * @version 1.0
 * @date 2024-09-03
 * 
 * \par License:
 *   \ref mit_license "MIT License".
 * @details This file provides the external definitions for the inline GPIO functions
 * defined in gpio_direct_RW.h. These definitions ensure that non-inlined
 * versions of the functions are available if needed by the linker.
 */

#include "gpio_direct_RW.h"

/**
 * @brief External definition for directWriteHigh_single function.
 *
 * This external definition is provided to satisfy the linker in case
 * the function is not inlined at all call sites. It sets the specified GPIO pin to high.
 */
extern inline void directWriteHigh_single(void);

/**
 * @brief External definition for directWriteLow_single function.
 *
 * This external definition is provided to satisfy the linker in case
 * the function is not inlined at all call sites. It sets the specified GPIO pin to low.
 */
extern inline void directWriteLow_single(void);

/**
 * @brief External definition for gpioDirectRead function.
 *
 * This external definition is provided to satisfy the linker in case
 * the function is not inlined at all call sites. It reads the state of the specified GPIO pin.
 *
 * @return uint32_t The state of the GPIO pin (0 or 1).
 */
extern inline uint32_t gpioDirectRead(void);
