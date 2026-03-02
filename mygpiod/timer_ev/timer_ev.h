/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Input device event handling
 */

#ifndef MYGPIOD_TIMER_EV_H
#define MYGPIOD_TIMER_EV_H

#include "mygpiod/config/config.h"
#include "mygpiod/event_loop/event_loop.h"

bool timer_ev_open(struct t_config *config, struct t_poll_fds *poll_fds);
struct t_timer_definition *timer_ev_get_by_fd(struct t_list *timer_definitions, int *fd);

#endif
