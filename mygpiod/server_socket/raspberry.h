/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_SERVER_RASPBERRY_H
#define MYGPIOD_SERVER_RASPBERRY_H

#include "mygpiod/lib/config.h"
#include "mygpiod/server_socket/protocol.h"

#include <stdbool.h>

bool handle_raspberry_vcio(struct t_list_node *client_node, const char *command);

#endif
