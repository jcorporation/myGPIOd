/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Input device action handling handling
 */

#include "compile_time.h"
#include "mygpiod/input/action.h"

#include "mygpiod/actions/execute.h"
#include "mygpiod/input/event_code.h"
#include "mygpiod/input/event_type.h"
#include "mygpiod/lib/events.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"

// Private definitions
static bool check_event(struct t_input_event_actions *event, struct t_input_event *input_data);

// Public functions

/**
 * Handles the configured actions for an input event and notifies the clients.
 * @param config Pointer to config
 * @param input_event Event data
 */
void input_action_handle(struct t_config *config, struct t_mygpiod_input_event *input_event) {
    MYGPIOD_LOG_INFO("%s: time=%ld.%06lu type=%s (%hu) code=%s (%hu) value=%u",
        input_event->device->name,
        input_event->data.time.tv_sec,
        input_event->data.time.tv_usec,
        input_event_type_name(input_event->data.type),
        input_event->data.type,
        input_event_code_name(input_event->data.type, input_event->data.code),
        input_event->data.code,
        input_event->data.value
    );

    // Check if read input data matches configured event
    // Execute actions and notify clients
    bool subscribed = false;
    if (input_event->device->event_actions.length == 0) {
        MYGPIOD_LOG_DEBUG("No actions configured for device %s", input_event->device->name);
    }
    else {
        struct t_list_node *current = input_event->device->event_actions.head;
        while (current != NULL) {
            struct t_input_event_actions *event = (struct t_input_event_actions *)current->data;
            if (check_event(event, &input_event->data) == true) {
                action_execute(config, &event->action);
                subscribed = true;
            }
            current = current->next;
        }
    }
    if (subscribed == true) {
        event_enqueue_input(config, input_event);
    }
    else {
        MYGPIOD_LOG_DEBUG("No action configured for type=%s (%hu) code=%s (%hu) value=%u",
            input_event_type_name(input_event->data.type),
            input_event->data.type,
            input_event_code_name(input_event->data.type, input_event->data.code),
            input_event->data.code,
            input_event->data.value
        );
    }
}

// Private functions

/**
 * Checks if configured event is the same as the read input event data
 * @param event Event configuration
 * @param input_data Read input data
 * @return true if it matches, else false
 */
static bool check_event(struct t_input_event_actions *event, struct t_input_event *input_data) {
    if (event->type != input_data->type) {
        return false;
    }
    if (event->code_match == MATCH_VALUE &&
        event->code != input_data->code)
    {
        return false;
    }
    if (event->value_match == MATCH_VALUE &&
        event->value != input_data->value)
    {
        return false;
    }
    return true;
}
