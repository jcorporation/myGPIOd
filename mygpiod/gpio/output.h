/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_GPIO_OUTPUT_H
#define MYGPIOD_GPIO_OUTPUT_H

#include "mygpiod/lib/config.h"

#include <gpiod.h>

bool gpio_set_outputs(struct t_config *config);
bool gpio_set_output(struct gpiod_chip *chip, unsigned gpio, struct t_gpio_out_data *data);

bool gpio_set_value(struct t_config *config, unsigned gpio, enum gpiod_line_value value);
bool gpio_toggle_value(struct t_config *config, unsigned gpio);
bool gpio_set_value_by_line_request(struct t_config *config, struct gpiod_line_request *line_request, unsigned gpio, enum gpiod_line_value value);
bool gpio_toggle_value_by_line_request(struct t_config *config, struct gpiod_line_request *line_request, unsigned gpio);
bool gpio_blink(struct t_config *config, unsigned gpio, int timeout_ms, int interval_ms);

#endif
