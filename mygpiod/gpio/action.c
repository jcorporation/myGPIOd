/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/gpio/action.h"

#include "mygpiod/actions/system.h"
#include "mygpiod/gpio/input.h"
#include "mygpiod/lib/config.h"
#include "mygpiod/lib/events.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/timer.h"
#include "mygpiod/lib/util.h"

#include <errno.h>
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>

// private definitions
static void action_delay(struct t_gpio_in_data *data);
static void action_execute(const char *action);

// public functions

/**
 * Handles the configured actions for an event
 * and notifies the clients.
 * @param config pointer to config
 * @param gpio the gpio number
 * @param timestamp timestamp of the event in nanoseconds
 * @param event_type the event type
 * @param data gpio config data
 */
void action_handle(struct t_config *config, unsigned gpio, uint64_t timestamp,
        enum gpiod_edge_event_type event_type, struct t_gpio_in_data *data)
{
    MYGPIOD_LOG_INFO("Event: \"%s\" gpio: \"%u\" timestamp: \"%llu\"",
        lookup_event_type(event_type), gpio, (long long unsigned)timestamp);

    if (data->ignore_event == true) {
        data->ignore_event = false;
        return;
    }
    if (event_type == GPIOD_EDGE_EVENT_FALLING_EDGE) {
        event_enqueue(config, gpio, MYGPIOD_EVENT_FALLING, timestamp);
        if (sdslen(data->action_falling) > 0) {
            if (data->request_event == GPIOD_LINE_EDGE_FALLING ||
                data->request_event == GPIOD_LINE_EDGE_BOTH)
            {
                action_execute(data->action_falling);
            }
        }
        if (data->long_press_event == GPIOD_LINE_EDGE_FALLING &&
            sdslen(data->long_press_action) > 0 &&
            data->long_press_timeout > 0)
        {
            action_delay(data);
        }
    }
    else {
        event_enqueue(config, gpio, MYGPIOD_EVENT_RISING, timestamp);
        if (sdslen(data->action_rising) > 0) {
            if (data->request_event == GPIOD_LINE_EDGE_RISING ||
                data->request_event == GPIOD_LINE_EDGE_BOTH)
            {
                action_execute(data->action_rising);
            }
        }
        if (data->long_press_event == GPIOD_LINE_EDGE_RISING &&
            sdslen(data->long_press_action) > 0 &&
            data->long_press_timeout > 0)
        {
            action_delay(data);
        }
    }
}

/**
 * Checks if the gpio value has not changed since the initial event 
 * and executes the defined action
 * @param gpio the gpio number
 * @param data gpio config data
 * @param config config
 */
void action_execute_delayed(unsigned gpio, struct t_gpio_in_data *data, struct t_config *config) {
    // check if gpio value has not changed
    enum gpiod_line_value value = gpio_get_value(config, gpio);

    if ((value == GPIOD_LINE_VALUE_ACTIVE && data->long_press_event == GPIOD_LINE_EDGE_RISING) ||
        (value == GPIOD_LINE_VALUE_INACTIVE && data->long_press_event == GPIOD_LINE_EDGE_FALLING))
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        uint64_t timestamp = (uint64_t)(ts.tv_sec * 1000000000 + ts.tv_nsec);
        event_enqueue(config, gpio, MYGPIOD_EVENT_LONG_PRESS, timestamp);
        action_execute(data->long_press_action);
        if (data->request_event == GPIOD_LINE_EDGE_BOTH) {
            // ignore the release event
            data->ignore_event = true;
        }
    }
    // remove timerfd
    action_delay_abort(data);
}

/**
 * Closes a timerfd for a delayed action
 * @param node pointer to node
 */
void action_delay_abort(struct t_gpio_in_data *data) {
    close_fd(&data->timer_fd);
}

//private functions

/**
 * Creates a timerfd for the long press action
 * @param node gpio config data
 * @param event_type the event type
 */
static void action_delay(struct t_gpio_in_data *data) {
    if (data->timer_fd > -1) {
        action_delay_abort(data);
    }
    data->timer_fd = timer_new(data->long_press_timeout);
}

/**
 * Executes the action
 * @param action action to execute
 */
static void action_execute(const char *action) {
    MYGPIOD_LOG_INFO("Executing \"%s\"", action);
    if (action[0] == '/') {
        action_system(action);
    }
    else {
        MYGPIOD_LOG_ERROR("Invalid action");
    }
}
