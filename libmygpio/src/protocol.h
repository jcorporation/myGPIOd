/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LIBMYGPIO_PROTOCOL_H
#define LIBMYGPIO_PROTOCOL_H

#include "libmygpio/include/libmygpio.h"

#include <stdbool.h>

bool check_response_ok(const char *line);
bool check_response_end(const char *line);

struct t_mygpio_pair *parse_pair(const char *line);
void free_pair(struct t_mygpio_pair *pair);

#endif
