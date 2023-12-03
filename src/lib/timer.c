/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "src/lib/timer.h"

#include "src/lib/log.h"
#include "src/lib/util.h"

#include <errno.h>
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>

/**
 * Creates a new timer fd
 * @param timeout timeout in seconds
 * @return created timer fd or -1 on error
 */
int timer_new(int timeout) {
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timer_fd == -1) {
        MYGPIOD_LOG_ERROR("Can not create timer: \"%s\"", strerror(errno));
        return -1;
    }
    if (timer_set(timer_fd, timeout) == false) {
        close(timer_fd);
        return -1;
    }
    return timer_fd;
}

/**
 * Sets the realtive timeout for a timer fd
 * @param timer_fd timer fd
 * @param timeout relative timeout in seconds
 * @return true on success, else false
 */
bool timer_set(int timer_fd, int timeout) {
    struct itimerspec its;
    its.it_value.tv_sec = timeout;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    errno = 0;
    if (timerfd_settime(timer_fd, 0, &its, NULL) == -1) {
        MYGPIOD_LOG_ERROR("Can not set expiration for timer: \"%s\"", strerror(errno));
        close_fd(&timer_fd);
        return false;
    }
    return true;
}

/**
 * Logs the next timer expiration
 * @param timer_fd timer fd
 */
void timer_log_next_expire(int timer_fd) {
    struct itimerspec its;
    errno = 0;
    if (timerfd_gettime(timer_fd, &its) == -1) {
        MYGPIOD_LOG_ERROR("Can not get expiration for timer: \"%s\"", strerror(errno));
        return;
    }
    MYGPIOD_LOG_DEBUG("Timer expires in %lld seconds", (long long)its.it_value.tv_sec);
}
