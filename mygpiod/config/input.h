/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_CONFIG_INPUT_H
#define MYGPIOD_CONFIG_INPUT_H

#include "mygpiod/config/config.h"

#include <stdbool.h>

bool parse_input_ev(struct t_config *config, sds config_value);
void input_data_clear(struct t_input_device *data);
void input_node_data_clear(struct t_list_node *node);

#endif
