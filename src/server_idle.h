/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_SERVER_IDLE_H
#define MYGPIOD_SERVER_IDLE_H

#include "config.h"

bool handle_idle(struct t_list_node *node);
bool handle_noidle(struct t_list_node *node);

#endif
