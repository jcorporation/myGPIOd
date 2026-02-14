/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
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
 * Handles the configured actions for an input event
 * and notifies the clients.
 * @param config pointer to config
 * @param input_data Input event data
 */
void input_action_handle(struct t_config *config, struct t_input_device *device, struct t_input_event *input_data) {
    MYGPIOD_LOG_INFO("%s: time=%ld.%06lu type=%s (%hu) code=%s (%hu) value=%u",
        device->device,
        input_data->time.tv_sec,
        input_data->time.tv_usec,
        input_event_type_name(input_data->type),
        input_data->type,
        input_event_code_name(input_data->type, input_data->code),
        input_data->code,
        input_data->value
    );

    // Check if read input data matches configured event
    // Execute actions and notify clients
    bool subscribed = false;
    if (device->event_actions.length == 0) {
        MYGPIOD_LOG_DEBUG("No actions configured for device %s", device->device);
    }
    else {
        struct t_list_node *current = device->event_actions.head;
        while (current != NULL) {
            struct t_input_event_actions *event = (struct t_input_event_actions *)current->data;
            if (check_event(event, input_data) == true) {
                action_execute(config, &event->action);
                subscribed = true;
            }
            current = current->next;
        }
    }
    if (subscribed == true) {
        event_enqueue_input(config, device->device, input_data);
    }
    else {
        MYGPIOD_LOG_DEBUG("No action configured for type=%s (%hu) code=%s (%hu) value=%u",
            input_event_type_name(input_data->type),
            input_data->type,
            input_event_code_name(input_data->type, input_data->code),
            input_data->code,
            input_data->value
        );
    }
}

// Private functions

/**
 * Checks if configured event is the same as the read input data
 * @param event Event configuration
 * @param input_data Read input data
 * @return true if it matches, else false
 */
static bool check_event(struct t_input_event_actions *event, struct t_input_event *input_data) {
    if (event->event_type != input_data->type) {
        return false;
    }
    if (event->event_code != input_data->code) {
        return false;
    }
    if (event->event_value == UINT_MAX) {
        // UINT_MAX means any value
        return true;
    }
    if (event->event_value != input_data->value) {
        return false;
    }
    return true;
}
