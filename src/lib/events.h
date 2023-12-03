/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_EVENTS_H
#define MYGPIOD_EVENTS_H

#include "src/lib/config.h"

/**
 * myGPIOD event types
 */
enum mygpiod_event_types {
    MYGPIOD_EVENT_FALLING,
    MYGPIOD_EVENT_RISING,
    MYGPIOD_EVENT_LONG_PRESS
};

/**
 * Event data
 */
struct t_event_data {
    enum mygpiod_event_types mygpiod_event_type;  //!< the myGPIOd event type
    struct timespec ts;                     //!< timestamp of the event
};

void event_enqueue(struct t_config *config, unsigned gpio, enum mygpiod_event_types event_type,
        const struct timespec *ts);
void event_data_clear(struct t_list_node *node);
const char *mygpiod_event_name(enum mygpiod_event_types event_type);

#endif
