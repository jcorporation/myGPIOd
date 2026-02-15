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
#include "mygpiod/config/config.h"
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
struct t_input_device *get_device(struct t_config *config, const char *device);
static void node_data_input_event_actions_clear(struct t_list_node *node);

// Public functions

/**
 * Parses an input_ev config line
 * @param config Pointer to config
 * @param config_value value to parse
 * @return true on success, else false
 */
bool parse_input_ev(struct t_config *config, sds config_value) {
    // Format is: device:type:code:value:action:options
    sds device_str = sds_getvalue(config_value, ':');
    sds type_str = sds_getvalue(config_value, ':');
    sds code_str = sds_getvalue(config_value, ':');
    sds value_str = sds_getvalue(config_value, ':');
    sds action_str = sds_getvalue(config_value, ':');

    struct t_input_device *device = get_device(config, device_str);
    unsigned short type = input_event_type_parse(type_str);
    unsigned short code = input_event_code_parse(code_str);
    unsigned int value;
    bool value_parsed = true;
    if (strcmp(value_str, "UINT_MAX") == 0) {
        value = UINT_MAX;
    }
    else {
        value_parsed = mygpio_parse_uint(value_str, &value, NULL, 0, UINT_MAX);
        if (value_parsed == false) {
            MYGPIOD_LOG_WARN("Invalid value \"%s\"", value_str);
        }
    }
    enum mygpiod_actions action = parse_action(action_str);
    FREE_SDS(device_str);
    FREE_SDS(type_str);
    FREE_SDS(code_str);
    FREE_SDS(value_str);
    FREE_SDS(action_str);
    if (device == NULL ||
        type == EV_MAX ||
        code == KEY_MAX ||
        value_parsed == false ||
        action == MYGPIOD_ACTION_UNKNOWN)
    {
        return false;
    }

    struct t_input_event_actions *data = malloc_assert(sizeof(struct t_input_event_actions));
    data->event_type = type;
    data->event_code = code;
    data->event_value = value;
    data->action.action = action;
    data->action.options = sdssplitargs(config_value, &data->action.options_count);
    return list_push(&device->event_actions, 0, data);
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param data input data to clear
 */
void input_data_clear(struct t_input_device *data) {
    close_fd(&data->fd);
    sdsfree(data->device);
    list_clear(&data->event_actions, node_data_input_event_actions_clear);
}

/**
 * Frees pointers and closes file descriptors from this node.
 * @param node input config node to clear
 */
void input_node_data_clear(struct t_list_node *node) {
    struct t_input_device *data = (struct t_input_device *)node->data;
    input_data_clear(data);
}

// Private functions

/**
 * Gets the input device struct by device name
 * @param config Pointer to config
 * @param device Device name
 * @return struct t_input_data* or NULL if not found
 */
struct t_input_device *get_device(struct t_config *config, const char *device) {
    struct t_list_node *current = config->inputs.head;
    while (current != NULL) {
        struct t_input_device *data = (struct t_input_device *)current->data;
        if (strcmp(data->device, device) == 0) {
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
