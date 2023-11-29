/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "event.h"

unsigned poll_fd_add(struct pollfd *pfds, int *pfds_type, unsigned pfd_count, int fd, short events, int pfd_type) {
    pfds->fd = fd;
    pfds->events = events;
    *pfds_type = pfd_type;
    return ++pfd_count;
}
