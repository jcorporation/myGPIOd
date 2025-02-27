/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOC_OPTIONS_H
#define MYGPIOC_OPTIONS_H

#include <stdbool.h>

struct t_options {
    char *socket;
    int timeout_ms;
};

void print_usage(void);
int handle_options(int argc, char **argv, struct t_options *options);
void init_options(struct t_options *options);
void clear_options(struct t_options *options);

#endif
