/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "mygpioc/util.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Prints a message if verbose is true
 * @param fmt 
 * @param ... 
 */
void verbose_printf(const char *fmt, ...) {
    if (verbose == false) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
    vprintf(fmt, args);
    va_end(args);
    #pragma GCC diagnostic pop
    printf("\n");
}
