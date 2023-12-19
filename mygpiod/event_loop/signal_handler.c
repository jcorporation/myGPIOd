/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/event_loop/signal_handler.h"

#include "mygpiod/lib/log.h"

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/signalfd.h>

/**
 * Creates a signalfd to exit on SIGTERM and SIGINT
 * @return the created signal fd
 */
int make_signalfd(void) {
    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGTERM);
    sigaddset(&sigmask, SIGINT);

    errno = 0;
    int rv = sigprocmask(SIG_BLOCK, &sigmask, NULL);
    if (rv < 0) {
        MYGPIOD_LOG_ERROR("Error masking signals: \"%s\"", strerror(errno));
        return -1;
    }

    errno = 0;
    int sigfd = signalfd(-1, &sigmask, 0);
    if (sigfd < 0) {
        MYGPIOD_LOG_ERROR("Error creating signalfd: \"%s\"", strerror(errno));
        return -1;
    }

    return sigfd;
}
