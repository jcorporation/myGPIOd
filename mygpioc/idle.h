/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOC_IDLE_H
#define MYGPIOC_IDLE_H

#include "libmygpio/include/libmygpio/libmygpio.h"

int handle_idle(int argc, char **argv, int option_index, struct t_mygpio_connection *conn);

#endif
