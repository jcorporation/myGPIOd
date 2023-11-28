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

int flags_to_line_request_flags(bool active_low, int flags);
int make_signalfd(void);
const char *lookup_gpio_value(int value);
bool parse_bool(const char *str);
const char *bool_to_str(bool v);
int parse_bias(const char *option);
const char *lookup_bias(int bias);
int parse_event_request(const char *str);
const char *lookup_event_request(int event);
int parse_event(const char *str);
const char *lookup_event(int event);
bool parse_uint(char *str, unsigned *result, char **rest, unsigned min, unsigned max);
bool parse_int(char *str, int *result, char **rest, unsigned min, unsigned max);

#endif
