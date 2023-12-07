/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LIBMYGPIO_IDLE_H
#define LIBMYGPIO_IDLE_H

#include "libmygpio/src/connection.h"

#include <stdbool.h>

bool mygpio_send_idle(struct t_mygpio_connection *connection);
bool mygpio_recv_idle(struct t_mygpio_connection *connection, int timeout);
bool mygpio_send_noidle(struct t_mygpio_connection *connection);

#endif
