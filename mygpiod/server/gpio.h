/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_SERVER_GPIO_H
#define MYGPIOD_SERVER_GPIO_H

#include "mygpiod/lib/config.h"
#include "mygpiod/server/protocol.h"

#include <stdbool.h>

bool handle_gpiolist(struct t_config *config, struct t_list_node *client_node);
bool handle_gpioget(struct t_cmd_options *options, struct t_config *config, struct t_list_node *client_node);
bool handle_gpioset(struct t_cmd_options *options, struct t_config *config, struct t_list_node *client_node);
bool handle_gpiotoggle(struct t_cmd_options *options, struct t_config *config, struct t_list_node *client_node);

#endif
