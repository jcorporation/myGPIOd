/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_EVENT_LOOP_H
#define MYGPIOD_EVENT_LOOP_H

#include "config.h"

#include <gpiod.h>
#include <poll.h>

/**
 * Poll fd types
 */
enum pfd_types {
    PFD_TYPE_GPIO = 0,
    PFD_TYPE_TIMER,
    PFD_TYPE_SIGNAL,
    PFD_TYPE_CONNECT,
    PFD_TYPE_CLIENT
};

/**
 * Maximum number off fds to poll
 */
#define MAX_FDS (GPIOD_LINE_BULK_MAX_LINES * 2 + MAX_CLIENT_CONNECTIONS + 1)

/**
 * Struct to hold poll fd data
 */
struct t_poll_fds {
    struct pollfd fd[MAX_FDS];  //!< file descriptors
    int type[MAX_FDS];          //!< type of the corrsponding fd
    unsigned len;               //!< number of file descriptors
};

bool event_poll_fd_add(struct t_poll_fds *poll_fds, int fd, int pfd_type, short events);
void event_add_timer_fds(struct t_config *config, struct t_poll_fds *poll_fds);
void event_add_client_fds(struct t_config *config, struct t_poll_fds *poll_fds);
bool event_read_delegate(struct t_config *config, struct t_poll_fds *poll_fds);

#endif
