/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "event.h"

#include "action.h"
#include "config.h"
#include "log.h"

#include <poll.h>
#include <unistd.h>

// private definitions
static void handle_event(int event_type, unsigned line_offset, const struct timespec *timestamp, void *data);

// public functions

/**
 * Poll callback for gpiod_ctxless_event_monitor_multiple_ext
 * @param num_lines count of gpios to poll
 * @param fds poll fds
 * @param timeout poll timeout
 * @param data void pointer to t_mon_ctx
 * @return int 
 */
int poll_callback(unsigned num_lines, struct gpiod_ctxless_event_poll_fd *fds, const struct timespec *timeout, void *data) {
    struct pollfd pfds[GPIOD_LINE_BULK_MAX_LINES + 1];
    struct t_mon_ctx *ctx = data;
    unsigned i;

    for (i = 0; i < num_lines; i++) {
        pfds[i].fd = fds[i].fd;
        pfds[i].events = POLLIN | POLLPRI;
    }

    pfds[i].fd = ctx->sigfd;
    pfds[i].events = POLLIN | POLLPRI;

    long ts = timeout->tv_sec * 1000 + timeout->tv_nsec / 1000000;

    int cnt = poll(pfds, num_lines + 1, (int)ts);
    if (cnt < 0) {
        return GPIOD_CTXLESS_EVENT_POLL_RET_ERR;
    }
    if (cnt == 0) {
        return GPIOD_CTXLESS_EVENT_POLL_RET_TIMEOUT;
    }

    int rv = cnt;
    for (i = 0; i < num_lines; i++) {
        if (pfds[i].revents) {
            fds[i].event = true;
            if (!--cnt) {
                return rv;
            }
        }
    }

    /*
     * If we're here, then there's a signal pending. No need to read it,
     * we know we should quit now.
     */
    close(ctx->sigfd);

    return GPIOD_CTXLESS_EVENT_POLL_RET_STOP;
}

/**
 * Event callback for gpiod_ctxless_event_monitor_multiple_ext
 * @param event_type the event type: rising or falling
 * @param line_offset the gpio number
 * @param timestamp timestamp of the event
 * @param data void pointer to t_mon_ctx
 * @return 0 on success
 */
int event_callback(int event_type, unsigned line_offset, const struct timespec *timestamp, void *data) {
    struct t_mon_ctx *ctx = data;

    switch (event_type) {
        case GPIOD_CTXLESS_EVENT_CB_RISING_EDGE:
        case GPIOD_CTXLESS_EVENT_CB_FALLING_EDGE:
            handle_event(event_type, line_offset, timestamp, ctx);
            break;
        default:
            /*
            * REVISIT: This happening would indicate a problem in the
            * library.
            */
            return GPIOD_CTXLESS_EVENT_CB_RET_OK;
    }

    if (ctx->events_wanted && ctx->events_done >= ctx->events_wanted) {
        return GPIOD_CTXLESS_EVENT_CB_RET_STOP;
    }

    return GPIOD_CTXLESS_EVENT_CB_RET_OK;
}

// private functions

/**
 * Event handler
 * @param event_type the event type: rising or falling
 * @param line_offset the gpio number
 * @param timestamp timestamp of the event
 * @param data void pointer to t_mon_ctx
 */
static void handle_event(int event_type, unsigned line_offset, const struct timespec *timestamp, void *data) {
    struct t_mon_ctx *ctx = data;
    ctx->events_done++;

    time_t now = time(NULL);
    if (now < ctx->config->startup_time + 5) {
        MYGPIOD_LOG_INFO("Ignoring events at startup");
        return;
    }
    
    action_execute(line_offset, timestamp, event_type, ctx->config);
}
