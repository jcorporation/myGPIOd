/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_TIMER_H
#define MYGPIOD_TIMER_H

#include "config.h"

bool timer_handle_event(int *fd, struct t_config *config, unsigned idx);

#endif
