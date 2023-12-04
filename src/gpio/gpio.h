/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_GPIO_H
#define MYGPIOD_GPIO_H

#include "src/lib/config.h"
#include "src/event_loop/event_loop.h"

#include <gpiod.h>

bool gpio_open_chip(struct t_config *config);
bool gpio_set_outputs(struct t_config *config);
bool gpio_set_output(struct gpiod_chip *chip, unsigned gpio, struct t_gpio_out_data *data);
bool gpio_request_inputs(struct t_config *config, struct t_poll_fds *poll_fds);
bool gpio_request_input(struct gpiod_chip *chip, unsigned gpio, struct t_gpio_in_data *data);
bool gpio_handle_event(struct t_config *config, int *fd);

enum gpiod_line_value gpio_get_value(struct t_config *config, unsigned gpio);

#endif
