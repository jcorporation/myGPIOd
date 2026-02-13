/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/input/action.h"

#include "mygpiod/input/input_event_code.h"
#include "mygpiod/input/input_event_type.h"
#include "mygpiod/lib/log.h"

/**
 * Handles the configured actions for an input event
 * and notifies the clients.
 * @param config pointer to config
 * @param input_data Input event data
 */
void input_action_handle(struct t_config *config, const char *device, struct t_input_event *input_data) {
    MYGPIOD_LOG_INFO("%s: time=%ld.%06lu type=%s (%hu) code=%s (%hu) value=%u",
        device,
        input_data->time.tv_sec,
        input_data->time.tv_usec,
        input_event_type_name(input_data->type),
        input_data->type,
        input_event_code_name(input_data->type, input_data->code),
        input_data->code,
        input_data->value
    );
    (void) config;
    /*
    event_enqueue(config, gpio, MYGPIOD_EVENT_LONG_PRESS_RELEASE, timestamp_ns);
    action_execute(config, &data->long_press_release_action);
    */
}
