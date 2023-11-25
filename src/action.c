/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "action.h"

#include "config.h"
#include "event.h"
#include "log.h"
#include "util.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>

// private definitions
static void action_delay(struct t_config *config, struct t_config_node *cn);
static void action_execute(struct t_config_node *cn);

// public functions

/**
 * Handles the configured actions for an event.
 * It forks and executes the script via system() call
 * @param offset the gpio number
 * @param ts timestamp of the event
 * @param event_type the event type
 * @param config pointer to myGPIOd config
 */
void action_handle(unsigned offset, const struct timespec *ts, int event_type, struct t_config *config) {
    MYGPIOD_LOG_INFO("Event: \"%s\" gpio: \"%u\" timestamp: \"[%8lld.%09ld]\"",
        (event_type == GPIOD_CTXLESS_EVENT_CB_RISING_EDGE ? " RISING EDGE" : "FALLING EDGE"), 
        offset, (long long)ts->tv_sec, ts->tv_nsec);

    //map GPIOD_CTXLESS_EVENT_CB_* to GPIOD_CTXLESS_EVENT_*
    if (event_type == GPIOD_CTXLESS_EVENT_CB_FALLING_EDGE) {
        event_type = GPIOD_CTXLESS_EVENT_FALLING_EDGE;
    }
    else if (event_type == GPIOD_CTXLESS_EVENT_CB_RISING_EDGE) {
        event_type = GPIOD_CTXLESS_EVENT_RISING_EDGE;
    }
    
    //get cmd
    struct t_config_node *current = config->head;
    while (current != NULL) {
       if (current->gpio == offset && 
            (current->edge == GPIOD_CTXLESS_EVENT_BOTH_EDGES || current->edge == event_type))
        {
            if (current->ignore_event == true) {
                // ignore this event, it was a long press
                current->ignore_event = false;
                return;
            }
            if (current->long_press == 0) {
                action_execute(current);
            }
            else {
                action_delay(config, current);
            }
        }
       current = current->next;
    }
}

/**
 * Checks if the gpio value has not changed since the initial event 
 * and executes the defined action
 * @param ctx 
 */
void action_execute_delayed(struct t_mon_ctx *ctx) {
    // check if gpio value has not changed
    int rv = gpiod_ctxless_get_value(ctx->config->chip, ctx->config->delayed_event.cn->gpio, ctx->config->active_low, MYGPIOD_NAME);
    if (rv < 0) {
        MYGPIOD_LOG_ERROR("Error reading value from gpio %u", ctx->config->delayed_event.cn->gpio);
        return;
    }
    if (rv == 1) {
        action_execute(ctx->config->delayed_event.cn);
        ctx->config->delayed_event.cn->ignore_event = true;
    }
    // remove timerfd
    action_delay_abort(ctx->config);
}

/**
 * Closes a timerfd for a delayed action
 * @param config pointer to config
 */
void action_delay_abort(struct t_config *config) {
    if (config->delayed_event.timer_fd > -1) {
        close(config->delayed_event.timer_fd);
        config->delayed_event.timer_fd = -1;
        config->delayed_event.cn = NULL;
    }
}

//private functions

/**
 * Creates a timerfd for the long press action
 * @param config pointer to config
 * @param cn pointer to gpio event config
 */
static void action_delay(struct t_config *config, struct t_config_node *cn) {
    if (config->delayed_event.timer_fd > -1) {
        action_delay_abort(config);
    }
    errno = 0;
    config->delayed_event.timer_fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    if (config->delayed_event.timer_fd == -1) {
        MYGPIOD_LOG_ERROR("Can not create timer: \"%s\"", strerror(errno));
        return;
    }
    struct itimerspec its;
    its.it_value.tv_sec = cn->long_press;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    errno = 0;
    if (timerfd_settime(config->delayed_event.timer_fd, 0, &its, NULL) == -1) {
        MYGPIOD_LOG_ERROR("Can not set expiration for timer: \"%s\"", strerror(errno));
        action_delay_abort(config);
        return;
    }
    config->delayed_event.cn = cn;
}

/**
 * Runs a system command in a new process
 * @param cn pointer to gpio config
 */
static void action_execute(struct t_config_node *cn) {
    MYGPIOD_LOG_INFO("Executing \"%s\"", cn->cmd);
    errno = 0;
    int rc = fork();
    if (rc == -1) {
        MYGPIOD_LOG_ERROR("Could not fork: %s", strerror(errno));
    }
    else if (rc == 0) {
        // this is the child process
        char gpio[16];
        snprintf(gpio, 16, "MYGPIOD_GPIO=%u", cn->gpio);
        char edge[15];
        snprintf(edge, 15, "MYGPIOD_EDGE=%d", cn->edge);
        char long_press[22];
        snprintf(long_press, 22, "MYGPIOD_LONG_PRESS=%d", cn->long_press);

        char *exec_argv[2] = { cn->cmd, NULL };
        char *exec_envp[4] = { gpio, edge, long_press, NULL };
        errno = 0;
        execve(cn->cmd, exec_argv, exec_envp);
        // successful execve call does not return
        MYGPIOD_LOG_ERROR("Error executing cmd \"%s\": %s", cn->cmd, strerror(errno));
        exit(EXIT_FAILURE);
    }
    else {
        MYGPIOD_LOG_DEBUG("Forked process with id %d", rc);
    }
}
