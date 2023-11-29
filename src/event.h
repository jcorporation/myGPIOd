/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_EVENT_H
#define MYGPIOD_EVENT_H

#include <poll.h>

/**
 * Poll fd types
 */
enum pfd_types {
    PFD_TYPE_GPIO = 0,
    PFD_TYPE_TIMER,
    PFD_TYPE_SIGNAL
};

unsigned poll_fd_add(struct pollfd *pfds, int *pfds_type, unsigned pfd_count, int fd, short events, int pfd_type);

#endif
