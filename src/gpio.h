/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_GPIO_H
#define MYGPIOD_GPIO_H

#include "config.h"

bool gpio_handle_event(struct t_config *config, unsigned idx);
bool gpio_set_outputs(struct t_config *config);
bool gpio_request_inputs(struct t_config *config);

#endif
