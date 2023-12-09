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

enum mygpio_event {
    MYGPIO_EVENT_UNKNOWN = -1,  //<! unknown
    MYGPIO_EVENT_FALLING,       //<! falling
    MYGPIO_EVENT_RISING,        //<! rising
    MYGPIO_EVENT_LONG_PRESS     //<! long_press
};

struct t_mygpio_idle_event {
    unsigned gpio;            //<! gpio number
    enum mygpio_event event;  //<! the event 
    uint64_t timestamp;       //<! timestamp in nanoseconds
};

bool mygpio_send_idle(struct t_mygpio_connection *connection);
bool mygpio_send_noidle(struct t_mygpio_connection *connection);
bool mygpio_wait_idle(struct t_mygpio_connection *connection, int timeout);
struct t_mygpio_idle_event *mygpio_recv_idle_event(struct t_mygpio_connection *connection);
void mygpio_free_idle_event(struct t_mygpio_idle_event *event);

#endif
