/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LIBMYGPIO_SRC_IDLE_H
#define LIBMYGPIO_SRC_IDLE_H

#include "libmygpio/include/libmygpio/libmygpio_idle.h"

/**
 * Struct holding the event information received by mygpio_recv_idle_event.
 */
struct t_mygpio_idle_event {
    enum mygpio_event event;         //!< the event
    uint64_t timestamp_ms;           //!< timestamp in milliseconds
    // GPIO event data
    unsigned gpio;                   //!< GPIO number
    // Input event data
    const char *input_event_device;  //!< Input event device
    const char *input_event_type;    //!< Input event type
    const char *input_event_code;    //!< Input event code
    unsigned input_event_value;      //!< INput event value
};

#endif
