/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_EVENTS_H
#define MYGPIOD_EVENTS_H

#include "mygpiod/lib/config.h"
#include <inttypes.h>

/**
 * myGPIOD event types
 */
enum mygpiod_event_types {
    MYGPIOD_EVENT_FALLING,
    MYGPIOD_EVENT_RISING,
    MYGPIOD_EVENT_LONG_PRESS,
    MYGPIOD_EVENT_LONG_PRESS_RELEASE
};

/**
 * Event data
 */
struct t_event_data {
    enum mygpiod_event_types mygpiod_event_type;  //!< the myGPIOd event type
    uint64_t timestamp;                           //!< timestamp of the event in nanoseconds
};

void event_enqueue(struct t_config *config, unsigned gpio, enum mygpiod_event_types event_type,
        uint64_t timestamp);
void event_data_clear(struct t_list_node *node);
const char *mygpiod_event_name(enum mygpiod_event_types event_type);

#endif
