/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_INPUT_ACTION_H
#define MYGPIOD_INPUT_ACTION_H

#include "mygpiod/input/util.h"
#include "mygpiod/lib/config.h"

void action_handle(struct t_config *config, const char *device, struct t_input_event *input_data);

#endif
