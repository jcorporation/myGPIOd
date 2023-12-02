/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_GPIO_H
#define MYGPIOD_GPIO_H

#include "src/lib/config.h"
#include "src/event_loop/event_loop.h"

bool gpio_open_chip(struct t_config *config);
bool gpio_handle_event(struct t_config *config, unsigned idx);
bool gpio_set_outputs(struct t_config *config);
bool gpio_request_inputs(struct t_config *config, struct t_poll_fds *poll_fds);

#endif
