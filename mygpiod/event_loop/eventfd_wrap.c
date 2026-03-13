/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myMPD (c) 2018-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Event handling
 */

#include "compile_time.h"
#include "mygpiod/event_loop/eventfd_wrap.h"

#include "mygpiod/lib/log.h"

#include <errno.h>
#include <sys/eventfd.h>
#include <unistd.h>

/**
 * Creates an eventfd
 * @return the eventfd
 */
int event_eventfd_create(void) {
    errno = 0;
    int fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK | EFD_SEMAPHORE);
    if (fd == -1) {
        MYGPIOD_LOG_ERROR("Unable to create eventfd");
        MYGPIOD_LOG_ERRNO(errno);
    }
    return fd;
}

/**
 * Increments the eventfd by one
 * @param fd fd to write
 * @return true on success, else false
 */
bool event_eventfd_write(int fd) {
    errno = 0;
    if (eventfd_write(fd, 1) != 0) {
        MYGPIOD_LOG_ERROR("Unable to write to eventfd");
        MYGPIOD_LOG_ERRNO(errno);
        return false;
    }
    return true;
}

/**
 * Reads from an eventfd
 * @param fd read from this fd
 * @return true on success, else false
 */
bool event_eventfd_read(int fd) {
    eventfd_t exp;
    errno = 0;
    if (eventfd_read(fd, &exp) != 0) {
        MYGPIOD_LOG_ERROR("Unable to read from eventfd");
        MYGPIOD_LOG_ERRNO(errno);
        return false;
    }
    return true;
}

/**
 * Closes the fd
 * @param fd fd to close
 */
void event_fd_close(int fd) {
    if (fd > -1) {
        close(fd);
    }
}
