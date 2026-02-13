/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/gpio/action.h"

#include "mygpiod/actions/execute.h"
#include "mygpiod/event_loop/event_loop.h"
#include "mygpiod/gpio/gpio.h"
#include "mygpiod/gpio/util.h"

#include "mygpiod/lib/config.h"
#include "mygpiod/lib/events.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/timer.h"

#include <gpiod.h>
#include <sys/timerfd.h>
#include <unistd.h>

// private definitions
static void gpio_action_delay(struct t_gpio_in_data *data);

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
void gpio_action_handle(struct t_config *config, unsigned gpio, uint64_t timestamp_ns,
        enum gpiod_edge_event_type event_type, struct t_gpio_in_data *data)
{
    if (data->ignore_event == true) {
        data->ignore_event = false;
        // long_press event has fired for this gpio
        MYGPIOD_LOG_INFO("Event: \"long_press_release\" gpio: \"%u\" timestamp: \"%llu ns\"",
            gpio, (long long unsigned)timestamp_ns);
        event_enqueue_gpio(config, gpio, MYGPIOD_EVENT_GPIO_LONG_PRESS_RELEASE, timestamp_ns);
        action_execute(config, &data->long_press_release_action);
        return;
    }

    MYGPIOD_LOG_INFO("Event: \"%s\" gpio: \"%u\" timestamp: \"%llu ns\"",
        lookup_event_type(event_type), gpio, (long long unsigned)timestamp_ns);

    if (event_type == GPIOD_EDGE_EVENT_FALLING_EDGE) {
        event_enqueue_gpio(config, gpio, MYGPIOD_EVENT_GPIO_FALLING, timestamp_ns);
        if (data->action_falling.length > 0) {
            action_execute(config, &data->action_falling);
        }
        else {
            MYGPIOD_LOG_DEBUG("No action configured");
        }
        // long press
        if (data->long_press_event == GPIOD_LINE_EDGE_FALLING &&
            data->long_press_timeout_ms > 0)
        {
            data->long_press_value = gpio_get_value(config, gpio);
            gpio_action_delay(data);
        }
    }
    else {
        event_enqueue_gpio(config, gpio, MYGPIOD_EVENT_GPIO_RISING, timestamp_ns);
        if (data->action_rising.length > 0) {
            action_execute(config, &data->action_rising);
        }
        else {
            MYGPIOD_LOG_DEBUG("No action configured");
        }
        // long press
        if (data->long_press_event == GPIOD_LINE_EDGE_RISING &&
            data->long_press_timeout_ms > 0)
        {
            data->long_press_value = gpio_get_value(config, gpio);
            gpio_action_delay(data);
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
void gpio_action_execute_delayed(unsigned gpio, struct t_gpio_in_data *data, struct t_config *config) {
    // check if gpio value has not changed
    if (gpio_get_value(config, gpio) == data->long_press_value) {
        uint64_t timestamp_ns = get_timestamp_ns(data->event_clock);
        MYGPIOD_LOG_INFO("Event: \"long_press\" gpio: \"%u\" timestamp: \"%llu ns\"",
            gpio, (long long unsigned)timestamp_ns);
        event_enqueue_gpio(config, gpio, MYGPIOD_EVENT_GPIO_LONG_PRESS, timestamp_ns);
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
    gpio_action_delay_abort(data);
}

/**
 * Closes a timerfd for a delayed action
 * @param node pointer to node
 */
void gpio_action_delay_abort(struct t_gpio_in_data *data) {
    MYGPIOD_LOG_DEBUG("Removing action delay timer");
    close_fd(&data->timer_fd);
}

//private functions

/**
 * Creates a timerfd for the long press action
 * @param node gpio config data
 * @param event_type the event type
 */
static void gpio_action_delay(struct t_gpio_in_data *data) {
    if (data->timer_fd > -1) {
        gpio_action_delay_abort(data);
    }
    data->timer_fd = timer_new(data->long_press_timeout_ms, data->long_press_interval_ms);
}
