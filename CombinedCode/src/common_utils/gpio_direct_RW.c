/**
 * @file gpio_direct_RW.c
 * @brief Implementation of direct GPIO read/write operations.
 * @details This file provides the external definitions for the inline GPIO functions
 * defined in gpio_direct_RW.h. These definitions ensure that non-inlined
 * versions of the functions are available if needed by the linker.
 * @version 0.1
 * @date 2024-07-29
 * 
 * @note This source file is part of a high-performance GPIO manipulation module.
 * 
 * @copyright Copyright (c) 2024
 */

#include "gpio_direct_RW.h"

/**
 * @brief External definition for directWriteHigh_single function.
 *
 * This external definition is provided to satisfy the linker in case
 * the function is not inlined at all call sites.
 */
extern inline void directWriteHigh_single(void);

/**
 * @brief External definition for directWriteLow_single function.
 *
 * This external definition is provided to satisfy the linker in case
 * the function is not inlined at all call sites.
 */
extern inline void directWriteLow_single(void);

/**
 * @brief External definition for gpioDirectRead function.
 *
 * This external definition is provided to satisfy the linker in case
 * the function is not inlined at all call sites.
 */
extern inline uint32_t gpioDirectRead(void);