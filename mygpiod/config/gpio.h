/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_CONFIG_GPIO_H
#define MYGPIOD_CONFIG_GPIO_H

#include "mygpiod/config/config.h"

#include <stdbool.h>

bool parse_gpio_config_file(int direction, void *data, const char *dirname, const char *filename);
bool parse_gpio_config_file_in_kv(sds key, sds value, struct t_gpio_in_data *data);
bool parse_gpio_config_file_out_kv(sds key, sds value, struct t_gpio_out_data *data);
struct t_gpio_in_data *gpio_in_data_new(void);
struct t_gpio_out_data *gpio_out_data_new(void);
void gpio_in_data_clear(struct t_gpio_in_data *data);
void gpio_node_in_clear(struct t_list_node *node);
void gpio_out_data_clear(struct t_gpio_out_data *data);
void gpio_node_out_clear(struct t_list_node *node);

#endif
