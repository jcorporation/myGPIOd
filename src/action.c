/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "action.h"

#include "config.h"
#include "log.h"
#include "util.h"

#include <errno.h>
#include <gpiod.h>
#include <stdio.h>
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>

// private definitions
static void action_delay(struct t_gpio_node_in *node);
static void action_execute(unsigned gpio, struct t_gpio_node_in *node, bool long_press);

// public functions

/**
 * Handles the configured actions for an event.
 * It forks and executes the script via system() call
 * @param gpio the gpio number
 * @param ts timestamp of the event
 * @param event_type the event type
 * @param config pointer to myGPIOd config
 */
void action_handle(unsigned gpio, const struct timespec *ts, int event_type, struct t_gpio_node_in *node) {
    MYGPIOD_LOG_INFO("Event: \"%s\" gpio: \"%u\" timestamp: \"[%8lld.%09ld]\" ",
        (event_type == GPIOD_CTXLESS_EVENT_RISING_EDGE ? "RISING" : "FALLING"), 
        gpio, (long long)ts->tv_sec, ts->tv_nsec);

    if (node->ignore_event == true) {
        node->ignore_event = false;
        return;
    }
    if (node->event == event_type ||
        node->event == GPIOD_CTXLESS_EVENT_BOTH_EDGES)
    {
        action_execute(gpio, node, false);
    }
    else if (node->long_press_timeout > 0 &&
             node->long_press_event == event_type)
    {
        action_delay(node);
    }
}

/**
 * Checks if the gpio value has not changed since the initial event 
 * and executes the defined action
 * @param ctx 
 */
void action_execute_delayed(unsigned gpio, struct t_gpio_node_in *node, struct t_config *config) {
    // check if gpio value has not changed
    int rv = gpiod_ctxless_get_value(config->chip, gpio, config->active_low, MYGPIOD_NAME);
    if (rv < 0) {
        MYGPIOD_LOG_ERROR("Error reading value from gpio %u", gpio);
        return;
    }
    if (rv == 1) {
        action_execute(gpio, node, true);
        node->ignore_event = true;
    }
    // remove timerfd
    action_delay_abort(node);
}

/**
 * Closes a timerfd for a delayed action
 * @param node pointer to node
 */
void action_delay_abort(struct t_gpio_node_in *node) {
    if (node->timer_fd > -1) {
        close(node->timer_fd);
        node->timer_fd = -1;
    }
}

//private functions

/**
 * Creates a timerfd for the long press action
 * @param config pointer to config
 * @param cn pointer to gpio event config
 */
static void action_delay(struct t_gpio_node_in *node) {
    if (node->timer_fd > -1) {
        action_delay_abort(node);
    }
    errno = 0;
    node->timer_fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    if (node->timer_fd == -1) {
        MYGPIOD_LOG_ERROR("Can not create timer: \"%s\"", strerror(errno));
        return;
    }
    struct itimerspec its;
    its.it_value.tv_sec = node->long_press_timeout;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    errno = 0;
    if (timerfd_settime(node->timer_fd, 0, &its, NULL) == -1) {
        MYGPIOD_LOG_ERROR("Can not set expiration for timer: \"%s\"", strerror(errno));
        action_delay_abort(node);
        return;
    }
}

/**
 * Runs a system command in a new process
 * @param cn pointer to gpio config
 */
static void action_execute(unsigned gpio, struct t_gpio_node_in *node, bool long_press) {
    char *cmd = long_press == false
        ? node->cmd
        : node->long_press_cmd;
    int event = long_press == false
        ? node->event
        : node->long_press_event;
    if (cmd == NULL) {
        MYGPIOD_LOG_INFO("Command for gpio %u is empty", gpio);
    }
    MYGPIOD_LOG_INFO("Executing \"%s\"", cmd);
    errno = 0;
    int rc = fork();
    if (rc == -1) {
        MYGPIOD_LOG_ERROR("Could not fork: %s", strerror(errno));
    }
    else if (rc == 0) {
        // this is the child process
        char gpio_str[16];
        snprintf(gpio_str, 16, "MYGPIOD_GPIO=%u", gpio);
        char event_str[16];
        snprintf(event_str, 16, "MYGPIOD_EVENT=%d", event);

        char *exec_argv[2] = { cmd, NULL };
        char *exec_envp[4] = { gpio_str, event_str, NULL };
        errno = 0;
        execve(cmd, exec_argv, exec_envp);
        // successful execve call does not return
        MYGPIOD_LOG_ERROR("Error executing cmd \"%s\": %s", cmd, strerror(errno));
        exit(EXIT_FAILURE);
    }
    else {
        MYGPIOD_LOG_DEBUG("Forked process with id %d", rc);
    }
}
