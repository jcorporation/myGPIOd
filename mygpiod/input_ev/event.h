/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Input device event handling
 */

#ifndef MYGPIOD_INPUT_EVENT_H
#define MYGPIOD_INPUT_EVENT_H

#include "mygpiod/config/config.h"

bool input_ev_handle_event(struct t_config *config, int *fd);

#endif
