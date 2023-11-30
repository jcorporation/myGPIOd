/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "event.h"

#include "log.h"

/**
 * Adds a fd to the list of fds to poll
 * @param poll_fds struct to add the new fd
 * @param fd fd to add
 * @param events events to poll
 * @param pfd_type type of poll fd
 * @return true on success, else false
 */
bool poll_fd_add(struct t_poll_fds *poll_fds, int fd, short events, int pfd_type) {
    if (poll_fds->len == MAX_FDS) {
        MYGPIOD_LOG_ERROR("Maximum number of poll fds reached");
        return false;
    }
    poll_fds->fd[poll_fds->len].fd = fd;
    poll_fds->fd[poll_fds->len].events = events;
    poll_fds->type[poll_fds->len] = pfd_type;
    poll_fds->len++;
    return true;
}
