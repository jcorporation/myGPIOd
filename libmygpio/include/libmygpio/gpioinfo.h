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

#ifndef LIBMYGPIO_GPIOINFO_H
#define LIBMYGPIO_GPIOINFO_H

#include <stdbool.h>

struct t_mygpio_connection;
struct t_mygpio_gpio;

/**
 * Lists the current settings of a GPIO.
 * Retrieve the settings with mygpio_recv_gpio_info and end the response with mygpio_response_end.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @param gpio GPIO number
 * @return bool true on success, else false.
 */
bool mygpio_gpioinfo(struct t_mygpio_connection *connection, unsigned gpio);

/**
 * Receives a list element of mygpio_gpioinfo.
 * Free it with mygpio_free_gpio.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @return Allocated struct t_mygpio_gpio or NULL on list end or error.
 */
struct t_mygpio_gpio *mygpio_recv_gpio_info(struct t_mygpio_connection *connection);

#endif
