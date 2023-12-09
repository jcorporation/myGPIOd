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

bool mygpio_response_end(struct t_mygpio_connection *connection);

#endif
