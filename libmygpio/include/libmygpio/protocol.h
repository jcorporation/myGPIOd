/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

// Do not include this file directly, use libmygpio.h

#ifndef LIBMYGPIO_PROTOCOL_H
#define LIBMYGPIO_PROTOCOL_H

struct t_mygpio_connection;

#include <stdbool.h>

/**
 * Finishes reading the response from myGPIOd and empties the input buffer.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @return true on success, else false
 */
bool mygpio_response_end(struct t_mygpio_connection *connection);

#endif
