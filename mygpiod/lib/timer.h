/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Timer implementation
 */

#ifndef MYGPIOD_TIMER_H
#define MYGPIOD_TIMER_H

#include <stdbool.h>

int timer_new(int timeout_ms, int interval_ms);
bool timer_set(int timer_fd, int timeout_ms, int interval_ms);
void timer_log_next_expire(const char *name, int timer_fd);
bool timer_repeat(int timer_fd);
bool timerfd_read_value(int *fd);

#endif
