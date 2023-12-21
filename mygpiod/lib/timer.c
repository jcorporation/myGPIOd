/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/lib/timer.h"

#include "mygpiod/lib/log.h"
#include "mygpiod/lib/util.h"

#include <errno.h>
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>

/**
 * Creates a new timer fd.
 * @param timeout_ms relative timeout in milliseconds
 * @param interval_ms interval in milliseconds
 * @return created timer fd or -1 on error
 */
int timer_new(int timeout_ms, int interval_ms) {
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timer_fd == -1) {
        MYGPIOD_LOG_ERROR("Can not create timer: \"%s\"", strerror(errno));
        return -1;
    }
    if (timer_set(timer_fd, timeout_ms, interval_ms) == false) {
        close(timer_fd);
        return -1;
    }
    return timer_fd;
}

/**
 * Sets the relative timeout for a timer fd.
 * @param timer_fd timer fd
 * @param timeout_ms relative timeout in milliseconds
 * @param interval_ms interval in milliseconds
 * @return true on success, else false
 */
bool timer_set(int timer_fd, int timeout_ms, int interval_ms) {
    struct itimerspec its;
    its.it_value.tv_sec = timeout_ms / 1000;
    its.it_value.tv_nsec = (long)((timeout_ms % 1000) * 1000000);
    its.it_interval.tv_sec = interval_ms / 1000;
    its.it_interval.tv_nsec = (long)((interval_ms % 1000) * 1000000);
    errno = 0;
    if (timerfd_settime(timer_fd, 0, &its, NULL) == -1) {
        MYGPIOD_LOG_ERROR("Can not set expiration for timer: \"%s\"", strerror(errno));
        close_fd(&timer_fd);
        return false;
    }
    return true;
}

/**
 * Logs the next timer expiration.
 * @param timer_fd timer fd
 */
void timer_log_next_expire(int timer_fd) {
    struct itimerspec its;
    errno = 0;
    if (timerfd_gettime(timer_fd, &its) == -1) {
        MYGPIOD_LOG_ERROR("Can not get expiration for timer: \"%s\"", strerror(errno));
        return;
    }
    int64_t timestamp = its.it_value.tv_sec * 1000 + its.it_value.tv_nsec / 1000000;
    MYGPIOD_LOG_DEBUG("Timer expires in %lld milliseconds", (long long)timestamp);
}
