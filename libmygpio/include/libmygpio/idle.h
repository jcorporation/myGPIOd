/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

// Do not include this file directly, use libmygpio.h

#ifndef LIBMYGPIO_IDLE_H
#define LIBMYGPIO_IDLE_H

#include <stdbool.h>

struct t_mygpio_connection;

bool mygpio_recv_idle(struct t_mygpio_connection *connection, int timeout);
bool mygpio_send_idle(struct t_mygpio_connection *connection);
bool mygpio_send_noidle(struct t_mygpio_connection *connection);

#endif
