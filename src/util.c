/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "util.h"

#include "log.h"

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
    int sigfd, rv;

    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGTERM);
    sigaddset(&sigmask, SIGINT);

    errno = 0;
    rv = sigprocmask(SIG_BLOCK, &sigmask, NULL);
    if (rv < 0) {
        MYGPIOD_LOG_ERROR("Error masking signals: \"%s\"", strerror(errno));
        return -1;
    }

    errno = 0;
    sigfd = signalfd(-1, &sigmask, 0);
    if (sigfd < 0) {
        MYGPIOD_LOG_ERROR("Error creating signalfd: \"%s\"", strerror(errno));
        return -1;
    }

    return sigfd;
}

/**
 * Checks if the value is in the array
 * @param value value to match
 * @param array array to check
 * @param len length of array
 * @return true if value is in array, else false
 */
bool value_in_array(unsigned value, unsigned *array, size_t len) {
    for(size_t i = 0; i < len; i++) {
        if(array[i] == value)
            return true;
    }
    return false;
}
