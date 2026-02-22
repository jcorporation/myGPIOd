/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Input device configuration
 */

#include "compile_time.h"
#include "mygpiod/config/input.h"

#include "mygpio-common/util.h"
#include "mygpiod/event_loop/event_loop.h"
#include "mygpiod/input/event_code.h"
#include "mygpiod/input/event_type.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"
#include "mygpiod/lib/sds_extras.h"

#include <linux/input-event-codes.h>
#include <string.h>

// Private definitions
static struct t_input_device *new_device(sds device_name);
static struct t_input_device *get_device(struct t_list *input_devices, const char *device);
static void node_data_input_event_actions_clear(struct t_list_node *node);

// Public functions

/**
 * Parses an input_ev config line
 * @param input_devices Pointer to list of input devices
 * @param config_value value to parse
 * @return true on success, else false
 */
bool parse_input_ev(struct t_list *input_devices, sds config_value) {
    // Format is: device:type:code:value:action:options
    sds device_str = sds_getvalue(config_value, ':');
    sds type_str = sds_getvalue(config_value, ':');
    sds code_str = sds_getvalue(config_value, ':');
    sds value_str = sds_getvalue(config_value, ':');
    sds action_str = sds_getvalue(config_value, ':');

    unsigned short type = input_event_type_parse(type_str);
    unsigned short code;
    enum input_event_match code_match = MATCH_VALUE;
    if (strcmp(code_str, "*") == 0) {
        code = KEY_CNT;
        code_match = MATCH_ALL;
    }
    else {
        code = input_event_code_parse(code_str);
    }
    unsigned int value;
    bool value_parsed = true;
    enum input_event_match value_match = MATCH_VALUE;
    if (strcmp(value_str, "*") == 0) {
        value = UINT_MAX;
        value_match = MATCH_ALL;
    }
    else {
        value_parsed = mygpio_parse_uint(value_str, &value, NULL, 0, UINT_MAX);
        if (value_parsed == false) {
            MYGPIOD_LOG_WARN("Invalid value \"%s\"", value_str);
        }
    }
    enum mygpiod_actions action = parse_action(action_str);

    // Check if device is already added, else add if
    struct t_input_device *device = get_device(input_devices, device_str);
    if (device == NULL) {
        device = new_device(device_str);
        list_push(input_devices, 0, device);
    }
    // Free all parsed strings
    FREE_SDS(device_str);
    FREE_SDS(type_str);
    FREE_SDS(code_str);
    FREE_SDS(value_str);
    FREE_SDS(action_str);
    // Check if all strings could be parsed
    if (type == EV_MAX ||
        code == KEY_MAX ||
        value_parsed == false ||
        action == MYGPIOD_ACTION_UNKNOWN)
    {
        return false;
    }
    // Add it to the input event actions list
    struct t_input_event_actions *data = malloc_assert(sizeof(struct t_input_event_actions));
    data->type = type;
    data->code = code;
    data->code_match = code_match;
    data->value = value;
    data->value_match = value_match;
    data->action.action = action;
    if (action != MYGPIOD_ACTION_NONE) {
        data->action.options = sdssplitargs(config_value, &data->action.options_count);
    }
    else {
        data->action.options = NULL;
        data->action.options_count = 0;
    }
    data->state = 0;
    return list_push(&device->event_actions, 0, data);
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param device input data to clear
 */
void input_data_clear(struct t_input_device *device) {
    close_fd(&device->fd);
    sdsfree(device->name);
    list_clear(&device->event_actions, node_data_input_event_actions_clear);
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param node input config node to clear
 */
void input_node_data_clear(struct t_list_node *node) {
    struct t_input_device *device = (struct t_input_device *)node->data;
    input_data_clear(device);
}

// Private functions

/**
 * Mallocs and initializes a new input device struct
 * @param device_name Input device path
 * @return struct t_input_device* 
 */
static struct t_input_device *new_device(sds device_name) {
    struct t_input_device *device = malloc_assert(sizeof(struct t_input_device));
    device->fd = -1;
    device->name = sdsdup(device_name);
    list_init(&device->event_actions);
    return device;
}

/**
 * Gets the input device struct by device name
 * @param input_devices Pointer to list of input devices
 * @param device Device name
 * @return struct t_input_data* or NULL if not found
 */
static struct t_input_device *get_device(struct t_list *input_devices, const char *device) {
    struct t_list_node *current = input_devices->head;
    while (current != NULL) {
        struct t_input_device *data = (struct t_input_device *)current->data;
        if (strcmp(data->name, device) == 0) {
            return data;
        }
        current = current->next;
    }
    MYGPIOD_LOG_WARN("Device \"%s\" not configured", device);
    return NULL;
}

/**
 * Clears the input event actions node data
 * @param node pointer to client
 */
static void node_data_input_event_actions_clear(struct t_list_node *node) {
    struct t_input_event_actions *data = (struct t_input_event_actions *)node->data;
    sdsfreesplitres(data->action.options, data->action.options_count);
}
