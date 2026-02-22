/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Input device event handling
 */

#ifndef MYGPIOD_INPUT_H
#define MYGPIOD_INPUT_H

#include "mygpiod/config/config.h"
#include "mygpiod/event_loop/event_loop.h"

bool inputs_open(struct t_config *config, struct t_poll_fds *poll_fds);

#endif
