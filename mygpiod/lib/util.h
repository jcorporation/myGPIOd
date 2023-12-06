/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_UTIL_H
#define MYGPIOD_UTIL_H

#include "dist/sds/sds.h"

#include <gpiod.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define FREE_SDS(SDS_PTR) do { \
    sdsfree(SDS_PTR); \
    SDS_PTR = NULL; \
} while (0)

void close_fd(int *fd);
int sds_getline(sds *s, FILE *fp, size_t max);
sds sds_catchar(sds s, const char c);
const char *lookup_gpio_value(enum gpiod_line_value value);
enum gpiod_line_value parse_gpio_value(const char *str);
bool parse_bool(const char *str);
const char *bool_to_str(bool v);
enum gpiod_line_bias parse_bias(const char *str);
const char *lookup_bias(enum gpiod_line_bias bias);
enum gpiod_line_clock parse_event_clock(const char *str);
enum gpiod_line_edge parse_event_request(const char *str);
const char *lookup_event_request(enum gpiod_line_edge event);
const char *lookup_event_type(enum gpiod_edge_event_type event);
enum gpiod_line_drive parse_drive(const char *str);
const char *lookup_drive(enum gpiod_line_drive value);
bool parse_uint(const char *str, unsigned *result, char **rest, unsigned min, unsigned max);
bool parse_ulong(const char *str, unsigned long *result, char **rest, unsigned long min, unsigned long max);
bool parse_int(const char *str, int *result, char **rest, int min, int max);

#endif
