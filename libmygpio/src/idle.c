/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "libmygpio/include/libmygpio/idle.h"
#include "libmygpio/src/idle.h"
#include "libmygpio/src/pair.h"
#include "libmygpio/src/protocol.h"
#include "mygpio-common/util.h"

#include <assert.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>

// private definitions

static enum mygpio_event libmygpio_parse_event(const char *str);

// public functions

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
    if (events > 0) {
        return libmygpio_recv_response_status(connection);
    }
    return false;
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

    struct t_mygpio_pair *pair = mygpio_recv_pair(connection);
    if (pair == NULL ||
        strcmp(pair->name, "gpio") != 0 ||
        mygpio_parse_uint(pair->value, &gpio, NULL, 0, 99) == false)
    {
        if (pair != NULL) {
            mygpio_free_pair(pair);
        }
        return NULL;
    }
    mygpio_free_pair(pair);

    pair = mygpio_recv_pair(connection);
    if (pair == NULL ||
        strcmp(pair->name, "mode") != 0 ||
        (event = libmygpio_parse_event(pair->value)) == MYGPIO_EVENT_UNKNOWN)
    {
        if (pair != NULL) {
            mygpio_free_pair(pair);
        }
        return NULL;
    }
    mygpio_free_pair(pair);

    pair = mygpio_recv_pair(connection);
    if (pair == NULL ||
        strcmp(pair->name, "time") != 0 ||
        mygpio_parse_uint64(pair->value, &timestamp, NULL, 0, UINT64_MAX) == false)
    {
        if (pair != NULL) {
            mygpio_free_pair(pair);
        }
        return NULL;
    }
    mygpio_free_pair(pair);

    struct t_mygpio_idle_event *gpio_event = malloc(sizeof(struct t_mygpio_idle_event));
    assert(gpio_event);
    gpio_event->gpio = gpio;
    gpio_event->event = event;
    gpio_event->timestamp = timestamp;
    return gpio_event;
}

/**
 * Returns the GPIO number from an idle event.
 * @param event Pointer to struct t_mygpio_idle_event.
 * @return GPIO number.
 */
unsigned mygpio_idle_event_get_gpio(struct t_mygpio_idle_event *event) {
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
 * Returns the timestamp from an idle event.
 * @param event Pointer to struct t_mygpio_idle_event.
 * @return The timestamp in nanoseconds
 */
uint64_t mygpio_idle_event_get_timestamp(struct t_mygpio_idle_event *event) {
    return event->timestamp;
}

/**
 * Frees the idle event struct
 * @param event struct to free
 */
void mygpio_free_idle_event(struct t_mygpio_idle_event *event) {
    free(event);
}

// privat functions

/**
 * Parses a string to a mygpio_event
 * @param str string to parse
 * @return mygpio event
 */
static enum mygpio_event libmygpio_parse_event(const char *str) {
    if (strcmp(str, "falling") == 0) {
        return MYGPIO_EVENT_FALLING;
    }
    if (strcmp(str, "rising") == 0) {
        return MYGPIO_EVENT_RISING;
    }
    if (strcmp(str, "long_press") == 0) {
        return MYGPIO_EVENT_LONG_PRESS;
    }
    return MYGPIO_EVENT_UNKNOWN;
}
