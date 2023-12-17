/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief myGPIOd client library
 *
 * Do not include this header directly. Use libmygpio/libmygpio.h instead.
 */

#ifndef LIBMYGPIO_GPIO_H
#define LIBMYGPIO_GPIO_H

#include "gpio_struct.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct t_mygpio_connection;

/**
 * @defgroup gpio_functions GPIO functions
 *
 * @brief This module provides functions to set and get values of a GPIO.
 *
 * @{
 */

/**
 * Returns the current value of a configured input or output GPIO.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @param gpio GPIO number
 * @return Value of the GPIO or MYGPIO_GPIO_VALUE_UNKNOWN on error.
 */
enum mygpio_gpio_value mygpio_gpioget(struct t_mygpio_connection *connection, unsigned gpio);

/**
 * Sets the value of a configured output GPIO.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @param gpio GPIO number
 * @param value Value to set: MYGPIO_GPIO_VALUE_LOW or MYGPIO_GPIO_VALUE_HIGH
 * @return true on success, else false.
 */
bool mygpio_gpioset(struct t_mygpio_connection *connection, unsigned gpio, enum mygpio_gpio_value value);

/**
 * Toggles the value of a configured output GPIO.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @param gpio GPIO number
 * @return true on success, else false.
 */
bool mygpio_gpiotoggle(struct t_mygpio_connection *connection, unsigned gpio);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
