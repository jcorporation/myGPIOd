/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Timer event command handling
 */

#ifndef MYGPIOD_SERVER_TIMEREV_H
#define MYGPIOD_SERVER_TIMEREV_H

#include "mygpiod/config/config.h"

#include <stdbool.h>

bool handle_timerevlist(struct t_config *config, struct t_list_node *client_node);

#endif
