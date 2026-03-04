/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief General GPIO functions
 */

#ifndef MYGPIOD_GPIO_GPIO_H
#define MYGPIOD_GPIO_GPIO_H

#include "mygpiod/config/config.h"
#include "mygpiod/event_loop/event_loop.h"

#include <gpiod.h>

bool gpio_init(struct t_config *config, struct t_poll_fds *poll_fds);
enum gpiod_line_value gpio_get_value(struct t_config *config, unsigned gpio);

#endif
