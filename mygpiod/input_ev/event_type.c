/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Input device event type parsing
 */

#include "compile_time.h"
#include "mygpiod/input_ev/event_type.h"

#include "mygpiod/lib/log.h"

#include <linux/input-event-codes.h>
#include <stddef.h>
#include <strings.h>

/**
 * Struct to map event type to names
 */
struct t_input_event_type_name {
    unsigned short event_type;  //!< Event type
    const char *name;           //!< Name
};

/**
 * Maps input event types to names
 */
static const struct t_input_event_type_name input_event_type_names[] = {
    { EV_SYN, "EV_SYN" },
    { EV_KEY, "EV_KEY" },
    { EV_REL, "EV_REL" },
    { EV_ABS, "EV_ABS" },
    { EV_MSC, "EV_MSC" },
    { EV_SW, "EV_SW" },
    { EV_LED, "EV_LED" },
    { EV_SND, "EV_SND" },
    { EV_REP, "EV_REP" },
    { EV_FF, "EV_FF" },
    { EV_PWR, "EV_PWR" },
    { EV_FF_STATUS, "EV_FF_STATUS" },
    { EV_MAX, NULL }
};

/**
 * Returns the name of an input event type
 * @param event_type Input event type
 * @return const char* Name of input event type or NULL if not found
 */
const char *input_event_type_name(unsigned short event_type) {
    const struct t_input_event_type_name *p = NULL;
    for (p = input_event_type_names; p->name != NULL; p++) {
        if (p->event_type == event_type) {
            break;
        }
    }
    if (p == NULL ||
        p->name == NULL)
    {
        MYGPIOD_LOG_WARN("Unknown event type \"%hu\"", event_type);
    }
    return p->name;
}

/**
 * Parses the string to a input event type
 * @param name String to parse
 * @return unsigned short or EV_MAX if it can not be parsed
 */
unsigned short input_event_type_parse(const char *name) {
    const struct t_input_event_type_name *p = NULL;
    for (p = input_event_type_names; p->name != NULL; p++) {
        if (strcasecmp(name, p->name) == 0) {
            break;
        }
    }
    if (p->event_type == EV_MAX) {
        MYGPIOD_LOG_WARN("Unknown event type \"%s\"", name);
    }
    return p->event_type;
}
