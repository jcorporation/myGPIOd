/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_GPIO_TIMER_H
#define MYGPIOD_GPIO_TIMER_H

#include "mygpiod/lib/config.h"

bool gpio_in_timer_handle_event(struct t_config *config, int *fd);
bool gpio_out_timer_handle_event(struct t_config *config, int *fd);

#endif
