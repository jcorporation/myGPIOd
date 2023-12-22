/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_EVENT_LOOP_H
#define MYGPIOD_EVENT_LOOP_H

#include "compile_time.h"
#include "mygpiod/lib/config.h"

#include <gpiod.h>
#include <poll.h>

extern bool update_pollfds;

/**
 * Poll fd types
 */
enum pfd_types {
    PFD_TYPE_GPIO = 0,
    PFD_TYPE_GPIO_IN_TIMER,
    PFD_TYPE_GPIO_OUT_TIMER,
    PFD_TYPE_SIGNAL,
    PFD_TYPE_CONNECT,
    PFD_TYPE_CLIENT,
    PFD_TYPE_CLIENT_TIMEOUT
};

/**
 * Maximum number off fds to poll
 */
#define MAX_FDS (GPIOS_MAX * 2 + CLIENT_CONNECTIONS_MAX * 2 + 1)

/**
 * Struct to hold poll fd data
 */
struct t_poll_fds {
    struct pollfd fd[MAX_FDS];  //!< file descriptors
    int type[MAX_FDS];          //!< type of the corresponding fd
    unsigned len;               //!< number of file descriptors
};

bool event_poll_fd_add(struct t_poll_fds *poll_fds, int fd, int pfd_type, short events);
void event_add_gpio_in_timer_fds(struct t_config *config, struct t_poll_fds *poll_fds);
void event_add_gpio_out_timer_fds(struct t_config *config, struct t_poll_fds *poll_fds);
void event_add_client_fds(struct t_config *config, struct t_poll_fds *poll_fds);
void close_fd(int *fd);
bool event_read_delegate(struct t_config *config, struct t_poll_fds *poll_fds);

#endif
