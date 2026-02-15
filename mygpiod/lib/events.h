/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief General event (enqueue) functions
 */

#ifndef MYGPIOD_EVENTS_H
#define MYGPIOD_EVENTS_H

#include "mygpiod/config/config.h"
#include "mygpiod/lib/event_types.h"

#include "mygpiod/input/input_event.h"

#include <inttypes.h>

/**
 * Event data
 */
struct t_event_data {
    enum mygpiod_event_types mygpiod_event_type;  //!< The myGPIOd event type
    uint64_t timestamp_ns;                        //!< Timestamp of the event in nanoseconds
    // Input event
    struct t_mygpiod_input_event input_event;     //!< Input event struct
};

void event_enqueue_gpio(struct t_config *config, unsigned gpio, enum mygpiod_event_types event_type,
        uint64_t timestamp);
void event_enqueue_input(struct t_config *config, struct t_mygpiod_input_event *input_event);
void event_data_clear(struct t_list_node *node);
const char *mygpiod_event_name(enum mygpiod_event_types event_type);

#endif
