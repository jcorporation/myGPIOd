/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/config/input.h"

bool parse_input_ev(struct t_config *config, sds value) {
    (void) config;
    (void) value;
    return true;
}
