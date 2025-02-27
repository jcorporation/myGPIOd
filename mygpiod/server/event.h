/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_SERVER_EVENT_H
#define MYGPIOD_SERVER_EVENT_H

#include "mygpiod/lib/config.h"
#include "mygpiod/server/protocol.h"

#include <stdbool.h>

bool handle_event(struct t_cmd_options *options, struct t_config *config, struct t_list_node *client_node);

#endif
