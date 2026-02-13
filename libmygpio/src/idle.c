/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"

#include "libmygpio/include/libmygpio/libmygpio_idle.h"
#include "libmygpio/src/idle.h"
#include "libmygpio/src/pair.h"
#include "libmygpio/src/protocol.h"
#include "mygpio-common/util.h"

#include <assert.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>

/**
 * Parses a string to the event type.
 * @param str String to parse
 * @return enum mygpio_event
 */
enum mygpio_event mygpio_parse_event(const char *str) {
    if (strcmp(str, "gpio_falling") == 0) {
        return MYGPIO_EVENT_GPIO_FALLING;
    }
    if (strcmp(str, "gpio_rising") == 0) {
        return MYGPIO_EVENT_GPIO_RISING;
    }
    if (strcmp(str, "gpio_long_press") == 0) {
        return MYGPIO_EVENT_GPIO_LONG_PRESS;
    }
    if (strcmp(str, "gpio_long_press_release") == 0) {
        return MYGPIO_EVENT_GPIO_LONG_PRESS_RELEASE;
    }
    if (strcmp(str, "gpio_input") == 0) {
        return MYGPIO_EVENT_INPUT;
    }
    return MYGPIO_EVENT_UNKNOWN;
}

/**
 * Lookups the name for the event.
 * @param event event type
 * @return Event name as string
 */
const char *mygpio_lookup_event(enum mygpio_event event) {
    switch(event) {
        case MYGPIO_EVENT_GPIO_FALLING:
            return "gpio_falling";
        case MYGPIO_EVENT_GPIO_RISING:
            return "gpio_rising";
        case MYGPIO_EVENT_GPIO_LONG_PRESS:
            return "gpio_long_press";
        case MYGPIO_EVENT_GPIO_LONG_PRESS_RELEASE:
            return "gpio_long_press_release";
        case MYGPIO_EVENT_INPUT:
            return "input";
        case MYGPIO_EVENT_UNKNOWN:
            return "unknown";
    }
    return "unknown";
}

/**
 * Sends the idle command
 * @param connection connection struct
 * @return true on success, else false
 */
bool mygpio_send_idle(struct t_mygpio_connection *connection) {
    return libmygpio_send_line(connection, "idle");
}

/**
 * Sends the noidle command
 * @param connection connection struct
 * @return true on success, else false
 */
bool mygpio_send_noidle(struct t_mygpio_connection *connection) {
    return libmygpio_send_line(connection, "noidle") &&
        libmygpio_recv_response_status(connection);
}

/**
 * Waits for an idle event
 * @param connection connection struct
 * @param timeout timeout in ms, use -1 to wait without a timeout
 * @return true if events are waiting, false on timeout or polling has failed
 */
bool mygpio_wait_idle(struct t_mygpio_connection *connection, int timeout) {
    struct pollfd pfds[1];
    pfds[0].fd = mygpio_connection_get_fd(connection);
    pfds[0].events = POLLIN;
    int events = poll(pfds, 1, timeout);
    return events > 0 ? true : false;
}

/**
 * Receives an idle event
 * @param connection connection struct
 * @return idle event or NULL on error or list end
 */
struct t_mygpio_idle_event *mygpio_recv_idle_event(struct t_mygpio_connection *connection) {
    unsigned gpio;
    enum mygpio_event event;
    uint64_t timestamp;
    struct t_mygpio_pair *pair;
    char *input_event_device = NULL;
    char *input_event_type = NULL;
    char *input_event_code = NULL;
    unsigned input_event_value;

    if ((pair = mygpio_recv_pair_name(connection, "event")) == NULL) {
        return NULL;
    }
    if ((event = mygpio_parse_event(pair->value)) == MYGPIO_EVENT_UNKNOWN) {
        mygpio_free_pair(pair);
        return NULL;
    }
    mygpio_free_pair(pair);

    if ((pair = mygpio_recv_pair_name(connection, "timestamp_ms")) == NULL ||
        mygpio_parse_uint64(pair->value, &timestamp, NULL, 0, UINT64_MAX) == false)
    {
        mygpio_free_pair(pair);
        return NULL;
    }
    mygpio_free_pair(pair);

    if (event == MYGPIO_EVENT_INPUT) {
        if ((pair = mygpio_recv_pair_name(connection, "device")) == NULL) {
            return NULL;
        }
        input_event_device = strdup(pair->value);
        mygpio_free_pair(pair);

        if ((pair = mygpio_recv_pair_name(connection, "type")) == NULL) {
            free(input_event_device);
            return NULL;
        }
        input_event_type = strdup(pair->value);
        mygpio_free_pair(pair);

        if ((pair = mygpio_recv_pair_name(connection, "code")) == NULL) {
            free(input_event_device);
            free(input_event_type);
            return NULL;
        }
        input_event_code = strdup(pair->value);
        mygpio_free_pair(pair);

        if ((pair = mygpio_recv_pair_name(connection, "value")) == NULL ||
            mygpio_parse_uint(pair->value, &input_event_value, NULL, 0, UINT_MAX) == false)
        {
            free(input_event_device);
            free(input_event_type);
            free(input_event_code);
            mygpio_free_pair(pair);
            return NULL;
        }
        mygpio_free_pair(pair);
    }
    else {
        if ((pair = mygpio_recv_pair_name(connection, "gpio")) == NULL) {
            return NULL;
        }
        if (mygpio_parse_uint(pair->value, &gpio, NULL, 0, GPIOS_MAX) == false) {
            mygpio_free_pair(pair);
            return NULL;
        }
        mygpio_free_pair(pair);
    }

    struct t_mygpio_idle_event *gpio_event = malloc(sizeof(struct t_mygpio_idle_event));
    assert(gpio_event);
    gpio_event->event = event;
    gpio_event->timestamp_ms = timestamp;
    if (event == MYGPIO_EVENT_INPUT) {
        gpio_event->input_event_device = input_event_device;
        gpio_event->input_event_type = input_event_type;
        gpio_event->input_event_code = input_event_code;
        gpio_event->input_event_value = input_event_value;
    }
    else {
        gpio_event->gpio = gpio;
    }
    return gpio_event;
}

/**
 * Returns the GPIO number from an idle event.
 * @param event Pointer to struct t_mygpio_idle_event.
 * @return GPIO number.
 */
unsigned mygpio_idle_event_get_gpio(struct t_mygpio_idle_event *event) {
    assert(event->event != MYGPIO_EVENT_INPUT);
    return event->gpio;
}

/**
 * Returns the event type from an idle event.
 * @param event Pointer to struct t_mygpio_idle_event.
 * @return The event type, one of enum mygpio_event.
 */
enum mygpio_event mygpio_idle_event_get_event(struct t_mygpio_idle_event *event) {
    return event->event;
}

/**
 * Returns the event type name from an idle event.
 * @param event Pointer to struct t_mygpio_idle_event.
 * @return The event type name
 */
const char *mygpio_idle_event_get_event_name(struct t_mygpio_idle_event *event) {
    return mygpio_lookup_event(event->event);
}

/**
 * Returns the timestamp from an idle event.
 * @param event Pointer to struct t_mygpio_idle_event.
 * @return The timestamp in milliseconds.
 */
uint64_t mygpio_idle_event_get_timestamp_ms(struct t_mygpio_idle_event *event) {
    return event->timestamp_ms;
}

/**
 * Returns the input event device
 * @param event Pointer to struct t_mygpio_idle_event.
 * @return char * or NULL if not an input event
 */
const char *mygpio_idle_event_get_input_device(struct t_mygpio_idle_event *event) {
    assert(event->event == MYGPIO_EVENT_INPUT);
    return event->input_event_device;
}

/**
 * Returns the input event type
 * @param event Pointer to struct t_mygpio_idle_event.
 * @return char * or NULL if not an input event
 */
const char *mygpio_idle_event_get_input_type(struct t_mygpio_idle_event *event) {
    assert(event->event == MYGPIO_EVENT_INPUT);
    return event->input_event_type;
}

/**
 * Returns the input event code
 * @param event Pointer to struct t_mygpio_idle_event.
 * @return char * or NULL if not an input event
 */
const char *mygpio_idle_event_get_input_code(struct t_mygpio_idle_event *event) {
    assert(event->event == MYGPIO_EVENT_INPUT);
    return event->input_event_code;
}

/**
 * Returns the input event value
 * @param event Pointer to struct t_mygpio_idle_event.
 * @return char * or NULL if not an input event
 */
unsigned mygpio_idle_event_get_input_value(struct t_mygpio_idle_event *event) {
    assert(event->event == MYGPIO_EVENT_INPUT);
    return event->input_event_value;
}

/**
 * Frees the idle event struct
 * @param event struct to free
 */
void mygpio_free_idle_event(struct t_mygpio_idle_event *event) {
    if (event->event == MYGPIO_EVENT_INPUT) {
        if (event->input_event_device != NULL) {
            free((char *)event->input_event_device);
        }
        if (event->input_event_type != NULL) {
            free((char *)event->input_event_type);
        }
        if (event->input_event_device != NULL) {
            free((char *)event->input_event_code);
        }
    }
    free(event);
}
