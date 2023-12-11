/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_ACTIONS_GPIOSET_H
#define MYGPIOD_ACTIONS_GPIOSET_H

#include "mygpiod/lib/config.h"

#include <stdbool.h>

bool action_gpioset(struct t_config *config, const char *cmd);

#endif
