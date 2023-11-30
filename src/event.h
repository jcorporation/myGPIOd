/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_EVENT_H
#define MYGPIOD_EVENT_H

#include "config.h"

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

bool event_poll_fd_add(struct t_poll_fds *poll_fds, int fd, int pfd_type);
void event_add_timer_fds(struct t_config *config, struct t_poll_fds *poll_fds);
bool event_read_delegate(struct t_config *config, struct t_poll_fds *poll_fds);

#endif
