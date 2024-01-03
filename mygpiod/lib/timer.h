/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_TIMER_H
#define MYGPIOD_TIMER_H

#include <stdbool.h>

int timer_new(int timeout_ms, int interval_ms);
bool timer_set(int timer_fd, int timeout_ms, int interval_ms);
void timer_log_next_expire(int timer_fd);
bool timer_repeat(int timer_fd);

#endif
