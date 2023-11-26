/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_UTIL_H
#define MYGPIOD_UTIL_H

#include "config.h"

#include <stdbool.h>
#include <stddef.h>

int make_signalfd(void);
bool value_in_array(unsigned value, unsigned *array, size_t len);
const char *lookup_gpio_value(enum gpio_values);
bool parse_chip(char *str, struct t_config *config);
bool parse_bool(const char *str);
const char *bool_to_str(bool v);
int parse_bias(const char *option);
const char *lookup_bias(int bias);
int parse_edge(const char *str);
const char *lookup_ctxless_event(int event);
bool parse_uint(char *str, unsigned *result, char **rest, unsigned min, unsigned max);
bool parse_int(char *str, int *result, char **rest, unsigned min, unsigned max);

#endif
