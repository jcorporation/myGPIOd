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
    sds input_event_device;                       //!< Input device
    unsigned short input_event_type;              //!< Is for example EV_REL for relative moment, EV_KEY for a keypress or release.
    unsigned short input_event_code;              //!< Event code, for example REL_X or KEY_BACKSPACE
    unsigned int input_event_value;               //!< The value the event carries.
};

void event_enqueue_gpio(struct t_config *config, unsigned gpio, enum mygpiod_event_types event_type,
        uint64_t timestamp);
void event_enqueue_input(struct t_config *config, const char *device, struct t_input_event *input_data);
void event_data_clear(struct t_list_node *node);
const char *mygpiod_event_name(enum mygpiod_event_types event_type);

#endif
