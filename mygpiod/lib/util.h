/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
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

uint64_t get_timestamp_ns(enum gpiod_line_clock event_clock);
sds sds_getline(sds s, FILE *fp, size_t max, int *nread);
sds sds_getfile(sds s, const char *file_path, int *nread);
sds *sds_splitfirst(sds s, char sep, int *count);
const char *lookup_gpio_value(enum gpiod_line_value value);
enum gpiod_line_value parse_gpio_value(const char *str);
enum gpiod_line_bias parse_bias(const char *str);
const char *lookup_bias(enum gpiod_line_bias bias);
enum gpiod_line_clock parse_event_clock(const char *str);
const char *lookup_event_clock(enum gpiod_line_clock clock);
enum gpiod_line_edge parse_event_request(const char *str);
const char *lookup_event_request(enum gpiod_line_edge event);
enum gpiod_edge_event_type  parse_event_type(const char *str);
const char *lookup_event_type(enum gpiod_edge_event_type event);
enum gpiod_line_drive parse_drive(const char *str);
const char *lookup_drive(enum gpiod_line_drive value);

#endif
