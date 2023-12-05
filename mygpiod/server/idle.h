/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_SERVER_IDLE_H
#define MYGPIOD_SERVER_IDLE_H

#include "mygpiod/lib/config.h"

bool handle_idle(struct t_list_node *client_node);
bool handle_noidle(struct t_config *config, struct t_list_node *client_node);
bool send_idle_events(struct t_list_node *client_node);

#endif
