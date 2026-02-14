/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_INPUT_ACTION_H
#define MYGPIOD_INPUT_ACTION_H

#include "mygpiod/input/input_event.h"
#include "mygpiod/config/config.h"

void input_action_handle(struct t_config *config, struct t_input_device *device, struct t_input_event *input_data);

#endif
