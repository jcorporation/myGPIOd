/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/gpio/action.h"

#include "mygpiod/actions/gpio.h"
#include "mygpiod/actions/system.h"
#include "mygpiod/event_loop/event_loop.h"
#include "mygpiod/gpio/gpio.h"
#include "mygpiod/lib/action.h"
#include "mygpiod/lib/config.h"
#include "mygpiod/lib/events.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/timer.h"
#include "mygpiod/lib/util.h"

#ifdef MYGPIOD_ENABLE_ACTION_MPC
    #include "mygpiod/actions/mpc.h"
#endif

#ifdef MYGPIOD_ENABLE_ACTION_HTTP
    #include "mygpiod/actions/http.h"
    #include "mygpiod/actions/mympd.h"
#endif

#include <errno.h>
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>

// private definitions
static void action_delay(struct t_gpio_in_data *data);
static void action_execute(struct t_config *config, struct t_list *actions);

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
    if (data->ignore_event == true) {
        data->ignore_event = false;
        // long_press event has fired for this gpio
        MYGPIOD_LOG_INFO("Event: \"long_press_release\" gpio: \"%u\" timestamp: \"%llu ns\"",
            gpio, (long long unsigned)timestamp);
        event_enqueue(config, gpio, MYGPIOD_EVENT_LONG_PRESS_RELEASE, timestamp);
        action_execute(config, &data->long_press_release_action);
        return;
    }

    MYGPIOD_LOG_INFO("Event: \"%s\" gpio: \"%u\" timestamp: \"%llu ns\"",
        lookup_event_type(event_type), gpio, (long long unsigned)timestamp);

    if (event_type == GPIOD_EDGE_EVENT_FALLING_EDGE) {
        event_enqueue(config, gpio, MYGPIOD_EVENT_FALLING, timestamp);
        if (data->action_falling.length > 0) {
            if (data->event_request == GPIOD_LINE_EDGE_FALLING ||
                data->event_request == GPIOD_LINE_EDGE_BOTH)
            {
                action_execute(config, &data->action_falling);
            }
        }
        // long press
        if (data->long_press_event == GPIOD_LINE_EDGE_FALLING &&
            data->long_press_timeout_ms > 0)
        {
            data->long_press_value = gpio_get_value(config, gpio);
            action_delay(data);
        }
    }
    else {
        event_enqueue(config, gpio, MYGPIOD_EVENT_RISING, timestamp);
        if (data->action_rising.length > 0) {
            if (data->event_request == GPIOD_LINE_EDGE_RISING ||
                data->event_request == GPIOD_LINE_EDGE_BOTH)
            {
                action_execute(config, &data->action_rising);
            }
        }
        // long press
        if (data->long_press_event == GPIOD_LINE_EDGE_RISING &&
            data->long_press_timeout_ms > 0)
        {
            data->long_press_value = gpio_get_value(config, gpio);
            action_delay(data);
        }
    }
}

/**
 * Checks if the gpio value has not changed since the initial event 
 * and executes the defined action.
 * @param gpio the gpio number
 * @param data gpio config data
 * @param config config
 */
void action_execute_delayed(unsigned gpio, struct t_gpio_in_data *data, struct t_config *config) {
    // check if gpio value has not changed
    if (gpio_get_value(config, gpio) == data->long_press_value) {
        struct timespec ts;
        switch (data->event_clock) {
            case GPIOD_LINE_CLOCK_REALTIME:
                clock_gettime(CLOCK_REALTIME, &ts);
                break;
            case GPIOD_LINE_CLOCK_HTE:
                //TODO: howto handle this?
            case GPIOD_LINE_CLOCK_MONOTONIC:
                clock_gettime(CLOCK_MONOTONIC, &ts);
                break;
        }
        uint64_t timestamp = (uint64_t)(ts.tv_sec * 1000000000 + ts.tv_nsec);
        MYGPIOD_LOG_INFO("Event: \"long_press\" gpio: \"%u\" timestamp: \"%llu ns\"",
            gpio, (long long unsigned)timestamp);
        event_enqueue(config, gpio, MYGPIOD_EVENT_LONG_PRESS, timestamp);
        action_execute(config, &data->long_press_action);
        if (data->event_request == GPIOD_LINE_EDGE_BOTH) {
            // ignore the release event
            MYGPIOD_LOG_DEBUG("Set next event for gpio %u to ignore", gpio);
            data->ignore_event = true;
        }
        if (data->long_press_interval_ms > 0) {
            return;
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
    MYGPIOD_LOG_DEBUG("Removing action delay timer");
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
    data->timer_fd = timer_new(data->long_press_timeout_ms, data->long_press_interval_ms);
}

/**
 * Executes the action
 * @param actions list of actions to execute
 * @param option option for the action
 */
static void action_execute(struct t_config *config, struct t_list *actions) {
    struct t_list_node *current = actions->head;
    while (current != NULL) {
        struct t_action *action = (struct t_action *)current->data;
        MYGPIOD_LOG_INFO("Executing %s: \"%s\"", lookup_action(action->action), action->option);
        switch(action->action) {
            case MYGPIOD_ACTION_SYSTEM:
                action_system(action->option);
                break;
            case MYGPIOD_ACTION_GPIO_SET:
                action_gpioset(config, action->option);
                break;
            case MYGPIOD_ACTION_GPIO_TOGGLE:
                action_gpiotoggle(config, action->option);
                break;
            case MYGPIOD_ACTION_GPIO_BLINK:
                action_gpioblink(config, action->option);
                break;
        #ifdef MYGPIOD_ENABLE_ACTION_MPC
            case MYGPIOD_ACTION_MPC:
                action_mpc(config, action->option);
                break;
        #endif
        #ifdef MYGPIOD_ENABLE_ACTION_HTTP
            case MYGPIOD_ACTION_HTTP:
                action_http(action->option);
                break;
            case MYGPIOD_ACTION_MYMPD:
                action_mympd(action->option);
                break;
        #endif
            case MYGPIOD_ACTION_UNKNOWN:
                MYGPIOD_LOG_ERROR("Invalid action");
                break;
        }
        current = current->next;
    }
}
