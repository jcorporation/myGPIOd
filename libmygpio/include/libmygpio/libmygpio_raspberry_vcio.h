/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief myGPIOd client library
 *
 * Do not include this header directly. Use libmygpio/libmygpio.h instead.
 */

#ifndef LIBMYGPIO_GPIO_H
#define LIBMYGPIO_GPIO_H

#include "libmygpio_gpio_struct.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct t_mygpio_connection;

/**
 * @defgroup libmygpio_raspberry_vcio_functions
 *
 * @brief This module provides functions to get values from /dev/vcio from Raspberry devices
 *
 * @{
 */

/**
 * Returns values reported by /dev/vcio
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @param command On of: vciotemp, vciovolts, vcioclock, vciothrottled
 * @return Current temperature or NULL on error
 */
struct t_mygpio_pair *mygpio_vcio(struct t_mygpio_connection *connection, const char *command);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
