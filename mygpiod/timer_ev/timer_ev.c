/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Input device event handling
 */

#include "config/timer_ev.h"
#include "compile_time.h"

#include "mygpiod/config/config.h"
#include "mygpiod/config/timer_ev.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/timer.h"
#include "mygpiod/timer_ev/timer_ev.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/timerfd.h>

// Private definitions
static int calc_starttime(int start_hour, int start_minute, int interval);

// Public functions

/**
 * Opens the timer event fds and add's it to the poll fds array
 * @param config Pointer to config
 * @param poll_fds Pointer to poll_fds array
 * @return true on success, else false
 */
bool timer_ev_open(struct t_config *config, struct t_poll_fds *poll_fds) {
    if (config->timer_definitions.length == 0) {
        MYGPIOD_LOG_INFO("No timer events configured");
        return true;
    }
    MYGPIOD_LOG_INFO("Opening timer event fds");
    struct t_list_node *current = config->timer_definitions.head;
    while (current != NULL) {
        struct t_timer_definition *definition = (struct t_timer_definition *)current->data;
        errno = 0;
        definition->fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        if (definition->fd < 0) {
            MYGPIOD_LOG_ERROR("Failure opening timer fd for \"%s\"", definition->name);
            MYGPIOD_LOG_ERRNO(errno);
            return false;
        }
        //Calculate and set next start time
        int start_in = calc_starttime(definition->start_hour, definition->start_minute, definition->interval);
        if (timer_set(definition->fd, start_in * 1000, definition->interval * 1000) == false) {
            return false;
        }
        timer_log_next_expire(definition->name, definition->fd);
        event_poll_fd_add(poll_fds, definition->fd, PFD_TYPE_TIMER_EV, POLLIN | POLLPRI);
        current = current->next;
    }
    return true;
}

/**
 * Get the timer definition by fd object
 * @param inputs Pointer to inputs list
 * @param fd fd to find
 * @return struct t_input_device* or NULL if not found
 */
struct t_timer_definition *timer_ev_get_by_fd(struct t_list *timer_definitions, int *fd) {
    struct t_list_node *current = timer_definitions->head;
    while (current != NULL) {
        struct t_timer_definition *data = (struct t_timer_definition *)current->data;
        if (data->fd == *fd) {
            return data;
        }
        current = current->next;
    }
    return NULL;
}

// Private functions

/**
 * Calculates the offset from now for next start time for a timer in seconds
 * @param start_hour start hour
 * @param start_minute start minute
 * @param interval reschedule interval
 * @return unix timestamp of next start
 */
static int calc_starttime(int start_hour, int start_minute, int interval) {
    time_t now = time(NULL);
    struct tm tms;
    (void)localtime_r(&now, &tms);
    tms.tm_hour = start_hour;
    tms.tm_min = start_minute;
    tms.tm_sec = 0;
    time_t start = mktime(&tms);

    if (interval <= 0) {
        interval = 86400;
    }

    while (start < now) {
        start += interval;
    }
    return (int)(start - now);
}
