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
static void action_execute(const char *cmd);

// public functions

/**
 * Handles the configured actions for an event.
 * It forks and executes the script via system() call
 * @param gpio the gpio number
 * @param ts timestamp of the event
 * @param event_type the event type
 * @param node gpio config data
 */
void action_handle(unsigned gpio, const struct timespec *ts, int event_type, struct t_gpio_node_in *node) {
    MYGPIOD_LOG_INFO("Event: \"%s\" gpio: \"%u\" timestamp: \"[%8lld.%09ld]\" ",
        lookup_event(event_type),
        gpio, (long long)ts->tv_sec, ts->tv_nsec);

    if (node->ignore_event == true) {
        node->ignore_event = false;
        return;
    }
    if (event_type == GPIOD_LINE_EVENT_FALLING_EDGE) {
        if (node->cmd_falling != NULL) {
            if (node->request_event == GPIOD_LINE_REQUEST_EVENT_FALLING_EDGE ||
                node->request_event == GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES)
            {
                action_execute(node->cmd_falling);
            }
        }
        if (node->long_press_event == GPIOD_LINE_EVENT_FALLING_EDGE &&
            node->long_press_cmd != NULL &&
            node->long_press_timeout > 0)
        {
            action_delay(node);
        }
    }
    else {
        if (node->cmd_rising != NULL) {
            if (node->request_event == GPIOD_LINE_REQUEST_EVENT_RISING_EDGE ||
                node->request_event == GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES)
            {
                action_execute(node->cmd_rising);
            }
        }
        if (node->long_press_event == GPIOD_LINE_EVENT_RISING_EDGE &&
            node->long_press_cmd != NULL &&
            node->long_press_timeout > 0)
        {
            action_delay(node);
        }
    }
}

/**
 * Checks if the gpio value has not changed since the initial event 
 * and executes the defined action
 * @param gpio the gpio number
 * @param node gpio config data
 * @param config config
 */
void action_execute_delayed(unsigned gpio, struct t_gpio_node_in *node, struct t_config *config) {
    // check if gpio value has not changed
    struct gpiod_line *line = gpiod_chip_get_line(config->chip, gpio);
    if (line == NULL) {
        MYGPIOD_LOG_ERROR("Error getting gpio %u", gpio);
        return;
    }
    errno = 0;
    int rv = gpiod_line_get_value(line);
    if (rv < 0) {
        MYGPIOD_LOG_ERROR("Error reading value from gpio %u: %s", gpio, strerror(errno));
        return;
    }
    if ((rv == GPIO_VALUE_HIGH && node->long_press_event == GPIOD_LINE_REQUEST_EVENT_RISING_EDGE) ||
        (rv == GPIO_VALUE_LOW && node->long_press_event == GPIOD_LINE_REQUEST_EVENT_FALLING_EDGE))
    {
        action_execute(node->long_press_cmd);
        if (node->request_event == GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES) {
            // ignore the release event
            node->ignore_event = true;
        }
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
 * @param node gpio config data
 * @param event_type the event type
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
 * @param cmd command to execute
 */
static void action_execute(const char *cmd) {
    MYGPIOD_LOG_INFO("Executing \"%s\"", cmd);
    errno = 0;
    int rc = fork();
    if (rc == -1) {
        MYGPIOD_LOG_ERROR("Could not fork: %s", strerror(errno));
    }
    else if (rc == 0) {
        // this is the child process
        errno = 0;
        execl(cmd, cmd, (char *)NULL);
        // successful execl call does not return
        MYGPIOD_LOG_ERROR("Error executing cmd \"%s\": %s", cmd, strerror(errno));
        exit(EXIT_FAILURE);
    }
    else {
        MYGPIOD_LOG_DEBUG("Forked process with id %d", rc);
    }
}
