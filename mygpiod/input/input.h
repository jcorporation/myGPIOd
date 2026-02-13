/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_INPUT_H
#define MYGPIOD_INPUT_H

#include "mygpiod/lib/config.h"
#include "mygpiod/event_loop/event_loop.h"

bool inputs_open(struct t_config *config, struct t_poll_fds *poll_fds);
bool input_handle_event(struct t_config *config, int *fd);

#endif
