/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/input/input_event_code.h"

#include <linux/input-event-codes.h>
#include <stddef.h>
#include <strings.h>

/**
 * Struct to map event code to names
 */
struct t_input_event_code_name {
    unsigned short event_code;  //!< Event type
    const char *name;           //!< Name
};

/**
 * Maps input event key/btn codes to names
 */
static const struct t_input_event_code_name input_event_key_code_names[] = {
    #include "mygpiod/event_key_code_table.inc"
    { KEY_MAX, NULL }
};

/**
 * Maps input event rel codes to names
 */
static const struct t_input_event_code_name input_event_rel_code_names[] = {
    #include "mygpiod/event_rel_code_table.inc"
    { KEY_MAX, NULL }
};

/**
 * Maps input event abs codes to names
 */
static const struct t_input_event_code_name input_event_abs_code_names[] = {
    #include "mygpiod/event_abs_code_table.inc"
    { KEY_MAX, NULL }
};

/**
 * Maps input event sw codes to names
 */
static const struct t_input_event_code_name input_event_sw_code_names[] = {
    #include "mygpiod/event_sw_code_table.inc"
    { KEY_MAX, NULL }
};

/**
 * Returns the name of an input event code
 * @param event_type Input event code
 * @return const char* Name of input event code
 */
const char *input_event_code_name(unsigned short event_type, unsigned short event_code) {
    const struct t_input_event_code_name *p = NULL;
    switch (event_type) {
        case EV_KEY:
            for (p = input_event_key_code_names; p->name != NULL; p++) {
                if (p->event_code == event_code) {
                    return p->name;
                }
            }
            break;
        case EV_REL:
            for (p = input_event_rel_code_names; p->name != NULL; p++) {
                if (p->event_code == event_code) {
                    return p->name;
                }
            }
            break;
        case EV_ABS:
            for (p = input_event_abs_code_names; p->name != NULL; p++) {
                if (p->event_code == event_code) {
                    return p->name;
                }
            }
            break;
        case EV_SW:
            for (p = input_event_sw_code_names; p->name != NULL; p++) {
                if (p->event_code == event_code) {
                    return p->name;
                }
            }
            break;
    }
    return NULL;
}

/**
 * Parses the string to a input event code
 * @param name String to parse
 * @return unsigned short or KEY_MAX if it can not be parsed
 */
unsigned short input_event_code_parse(const char *name) {
    const struct t_input_event_code_name *p = NULL;
    for (p = input_event_key_code_names; p->name != NULL; p++) {
        if (strcasecmp(name, p->name) == 0) {
            break;
        }
    }
    for (p = input_event_rel_code_names; p->name != NULL; p++) {
        if (strcasecmp(name, p->name) == 0) {
            break;
        }
    }
    for (p = input_event_abs_code_names; p->name != NULL; p++) {
        if (strcasecmp(name, p->name) == 0) {
            break;
        }
    }
    for (p = input_event_sw_code_names; p->name != NULL; p++) {
        if (strcasecmp(name, p->name) == 0) {
            break;
        }
    }
    return p->event_code;
}
