/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Hook command handling
 */

#ifndef MYGPIOD_SERVER_HOOK_H
#define MYGPIOD_SERVER_HOOK_H

#include "mygpiod/config/config.h"
#include "mygpiod/server_socket/protocol.h"

#include <stdbool.h>

bool handle_hook(struct t_cmd_options *options, struct t_config *config, struct t_list_node *client_node);

#endif
