/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Timer event configuration
 */

#include "compile_time.h"
#include "mygpiod/config/timer_ev.h"

#include "mygpio-common/util.h"
#include "mygpiod/event_loop/event_loop.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"
#include "mygpiod/lib/sds_extras.h"

// Private definitions
static struct t_timer_definition *new_definition(void);
bool parse_interval(sds interval_str, int *interval_sec);
bool parse_weekdays(sds weekdays_str, bool weekdays[7]);

// Public functions

/**
 * Parses an timer_ev config line
 * @param timer_definitions Pointer to list of timer definitions
 * @param config_value value to parse
 * @return true on success, else false
 */
bool parse_timer_ev(struct t_list *timer_definitions, sds config_value) {
    // Format is: hour:minute:interval:weekdays:action:options
    sds hour_str = sds_getvalue(config_value, ':');
    sds min_str = sds_getvalue(config_value, ':');
    sds interval_str = sds_getvalue(config_value, ':');
    sds weekdays_str = sds_getvalue(config_value, ':');
    sds action_str = sds_getvalue(config_value, ':');

    bool parsed = true;
    int hour;
    if (mygpio_parse_int(hour_str, &hour, NULL, 0, 23) == false) {
        MYGPIOD_LOG_WARN("Invalid hour \"%s\"", hour_str);
        parsed = false;
    }
    
    int minute;
    if (mygpio_parse_int(min_str, &minute, NULL, 0, 59) == false) {
        MYGPIOD_LOG_WARN("Invalid minute \"%s\"", min_str);
        parsed = false;
    }

    int interval;
    if (parse_interval(interval_str, &interval) == false) {
        MYGPIOD_LOG_WARN("Invalid interval \"%s\"", interval_str);
        parsed = false;
    }

    bool weekdays[7];
    if (parse_weekdays(weekdays_str, weekdays) == false) {
        MYGPIOD_LOG_WARN("Invalid interval \"%s\"", interval_str);
        parsed = false;
    }

    enum mygpiod_actions action = parse_action(action_str);

    // Free all parsed strings
    FREE_SDS(hour_str);
    FREE_SDS(min_str);
    FREE_SDS(interval_str);
    FREE_SDS(weekdays_str);
    FREE_SDS(action_str);

    // Check if all strings could be parsed
    if (parsed == false ||
        action == MYGPIOD_ACTION_UNKNOWN)
    {
        return false;
    }

    // Add it to the timer event list
    struct t_timer_definition *definition = new_definition();
    definition->action.action = action;
    definition->start_hour = hour;
    definition->start_minute = minute;
    definition->interval = interval;
    if (action != MYGPIOD_ACTION_NONE) {
        definition->action.options = sdssplitargs(config_value, &definition->action.options_count);
    }
    else {
        definition->action.options = NULL;
        definition->action.options_count = 0;
    }
    return list_push(timer_definitions, 0, definition);
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param device input data to clear
 */
void timer_definition_data_clear(struct t_timer_definition *definition) {
    close_fd(&definition->fd);
    sdsfreesplitres(definition->action.options, definition->action.options_count);
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param node input config node to clear
 */
void timer_node_definition_data_clear(struct t_list_node *node) {
    struct t_timer_definition *definition = (struct t_timer_definition *)node->data;
    timer_definition_data_clear(definition);
}

// Private functions

/**
 * Mallocs and initializes a new input device struct
 * @param device_name Input device path
 * @return struct t_input_device* 
 */
static struct t_timer_definition *new_definition(void) {
    struct t_timer_definition *definition = malloc_assert(sizeof(struct t_timer_definition));
    definition->fd = -1;
    return definition;
}

/**
 * Parses an interval with unit (s, m, h, d, w)
 * @param interval_str String to parse
 * @param interval_sec Pointer to integer for the result
 * @return true on success, else false
 */
bool parse_interval(sds interval_str, int *interval_sec) {
    int interval;
    char *unit;
    if (mygpio_parse_int(interval_str, &interval, &unit, 0, 604800) == false) {
        return false;
    }
    int quantifier;
    switch (*unit) {
        case 's': quantifier = 1; break;
        case 'm': quantifier = 60; break;
        case 'h': quantifier = 3600; break;
        case 'd': quantifier = 86400; break;
        case 'w': quantifier = 604800; break;
        default:
            return false;
    }

    if (INT_MAX / quantifier > interval) {
        // Prevent overflow
        return false;
    }

    *interval_sec = interval * quantifier;
    return true;
}

/**
 * Parses an weekday string.
 * Comma separated list of weekdays or * for each weekday.
 * 0 or 7 = Sunday
 * @param weekdays_str String to parse
 * @param weekdays Boolean array for weekdays
 * @return true on success, else false
 */
bool parse_weekdays(sds weekdays_str, bool weekdays[7]) {
    if (weekdays_str[0] == '*') {
        weekdays[0] = true;
        weekdays[1] = true;
        weekdays[2] = true;
        weekdays[3] = true;
        weekdays[4] = true;
        weekdays[5] = true;
        weekdays[6] = true;
        return true;
    }

    size_t len = sdslen(weekdays_str);
    for (size_t i = 0; i < len; i++) {
        switch (weekdays_str[i]) {
            case 0: weekdays[0] = true; break;
            case 1: weekdays[1] = true; break;
            case 2: weekdays[2] = true; break;
            case 3: weekdays[3] = true; break;
            case 4: weekdays[4] = true; break;
            case 5: weekdays[5] = true; break;
            case 6: weekdays[6] = true; break;
            case 7: weekdays[0] = true; break;
            case ',':
            case ' ': break;
            default:
                return false;
        }
    }
    return true;
}
