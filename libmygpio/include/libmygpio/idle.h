/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

// Do not include this file directly, use libmygpio.h

#ifndef LIBMYGPIO_IDLE_H
#define LIBMYGPIO_IDLE_H

#include <stdbool.h>
#include <stdint.h>

struct t_mygpio_connection;

/**
 * Possible event types
 */
enum mygpio_event {
    MYGPIO_EVENT_UNKNOWN = -1,  //<! unknown
    MYGPIO_EVENT_FALLING,       //<! falling
    MYGPIO_EVENT_RISING,        //<! rising
    MYGPIO_EVENT_LONG_PRESS     //<! long_press
};

/**
 * Parses a string to the event type.
 * @param str String to parse
 * @return enum mygpio_event
 */
enum mygpio_event libmygpio_parse_event(const char *str);

/**
 * Lookups the name for the event.
 * @param event event type
 * @return Event name as string
 */
const char *libmygpio_lookup_event(enum mygpio_event event);

/**
 * Opaque struct holding the event information received by mygpio_recv_idle_event.
 */
struct t_mygpio_idle_event;

/**
 * Enters the myGPIOd idle mode to get notifications about events.
 * Retrieve the list of events with mygpio_recv_idle_event.
 * In this mode no commands but mygpio_send_noidle are allowed.
 * All timeouts are disabled.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @return true on success, else false
 */
bool mygpio_send_idle(struct t_mygpio_connection *connection);

/**
 * Exits the myGPIOd idle mode.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @return true on success, else false
 */
bool mygpio_send_noidle(struct t_mygpio_connection *connection);

/**
 * Waits until an event occurs or the timeout expires.
 * It returns instantly if events had occurred while not in idle mode.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @param timeout Timeout in milliseconds, -1 for no timeout
 * @return true if an event has occurred, false on timeout or error.
 */
bool mygpio_wait_idle(struct t_mygpio_connection *connection, int timeout);

/**
 * Receives a list element of the waiting idle events.
 * Free it with mygpio_free_idle_event.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @return Allocated struct t_mygpio_idle_event or NULL on list end or error.
 */
struct t_mygpio_idle_event *mygpio_recv_idle_event(struct t_mygpio_connection *connection);

/**
 * Returns the GPIO number from an idle event.
 * @param event Pointer to struct t_mygpio_idle_event.
 * @return GPIO number.
 */
unsigned mygpio_idle_event_get_gpio(struct t_mygpio_idle_event *event);

/**
 * Returns the event type from an idle event.
 * @param event Pointer to struct t_mygpio_idle_event.
 * @return The event type, one of enum mygpio_event.
 */
enum mygpio_event mygpio_idle_event_get_event(struct t_mygpio_idle_event *event);

/**
 * Returns the event type name from an idle event.
 * @param event Pointer to struct t_mygpio_idle_event.
 * @return The event type name
 */
const char *mygpio_idle_event_get_event_name(struct t_mygpio_idle_event *event);

/**
 * Returns the timestamp from an idle event.
 * @param event Pointer to struct t_mygpio_idle_event.
 * @return The timestamp in milliseconds.
 */
uint64_t mygpio_idle_event_get_timestamp_ms(struct t_mygpio_idle_event *event);

/**
 * Frees the struct received by mygpio_recv_idle_event
 * @param event Pointer to struct t_mygpio_idle_event.
 */
void mygpio_free_idle_event(struct t_mygpio_idle_event *event);

#endif
