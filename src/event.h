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

/**
 * Maximum number off fds to poll
 */
#define MAX_FDS (GPIOD_LINE_BULK_MAX_LINES * 2 + 1)

/**
 * Struct to hold poll fd data
 */
struct t_poll_fds {
    struct pollfd fd[MAX_FDS];  //!< file descriptors
    int type[MAX_FDS];          //!< type of the corrsponding fd
    unsigned len;               //!< number of file descriptors
};

bool poll_fd_add(struct t_poll_fds *poll_fds, int fd, short events, int pfd_type);

#endif
