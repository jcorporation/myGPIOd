/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_TIMER_H
#define MYGPIOD_TIMER_H

int timer_new(int timeout);
void timer_next_expire(int timer_fd);

#endif
