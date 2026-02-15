/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Option handling
 */

#ifndef MYGPIOC_OPTIONS_H
#define MYGPIOC_OPTIONS_H

#include <stdbool.h>

/**
 * Struct to save mygpioc options
 */
struct t_options {
    char *socket;    //!< Path to mygpiod socket
    int timeout_ms;  //!< Socket connection timeout
};

void print_usage(void);
int handle_options(int argc, char **argv, struct t_options *options);
void init_options(struct t_options *options);
void clear_options(struct t_options *options);

#endif
