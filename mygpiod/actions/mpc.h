/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_ACTIONS_MPC_H
#define MYGPIOD_ACTIONS_MPC_H

#include "mygpiod/lib/config.h"

#include <stdbool.h>

bool action_mpc(struct t_config *config, const char *cmd);

#endif
