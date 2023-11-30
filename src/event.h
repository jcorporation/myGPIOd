/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_EVENT_H
#define MYGPIOD_EVENT_H

#include <gpiod.h>
#include <poll.h>

/**
 * Poll fd types
 */
enum pfd_types {
    PFD_TYPE_GPIO = 0,
    PFD_TYPE_TIMER,
    PFD_TYPE_SIGNAL
};

#define MAX_FDS (GPIOD_LINE_BULK_MAX_LINES * 2 + 1)
struct t_poll_fds {
    struct pollfd fd[MAX_FDS];
    int type[MAX_FDS];
    unsigned len;
};

bool poll_fd_add(struct t_poll_fds *poll_fds, int fd, short events, int pfd_type);

#endif
