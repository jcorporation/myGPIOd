/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/input/input.h"

#include "mygpiod/input/action.c"
#include "mygpiod/input/event_type.h"
#include "mygpiod/config/config.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <string.h>
#include <unistd.h>

// Private definitions

static struct t_input_data *get_input_data_by_fd(struct t_list *inputs, int *fd);

// Public functions

/**
 * Opens the input devices and add's it to the poll fds array
 * @param config pointer to config
 * @return true on success, else false
 */
bool inputs_open(struct t_config *config, struct t_poll_fds *poll_fds) {
    if (config->inputs.length == 0) {
        MYGPIOD_LOG_INFO("No inputs for monitoring configured");
        return true;
    }
    MYGPIOD_LOG_INFO("Requesting inputs");
    struct t_list_node *current = config->inputs.head;
    while (current != NULL) {
        struct t_input_data *data = (struct t_input_data *)current->data;
        errno = 0;
        MYGPIOD_LOG_INFO("Opening input %s", data->name);
        data->fd = open(data->name, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (data->fd < 0) {
            MYGPIOD_LOG_ERROR("Failure opening %s", data->name);
            MYGPIOD_LOG_ERRNO(errno);
        }
        else {
            event_poll_fd_add(poll_fds, data->fd, PFD_TYPE_INPUT, POLLIN | POLLPRI);
        }
        current = current->next;
    }
    return true;
}

/**
 * Reads the event data from an input event
 * @param config Pointer to config
 * @param fd fd with data to read
 */
bool input_handle_event(struct t_config *config, int *fd) {
    struct t_input_data *data = get_input_data_by_fd(&config->inputs, fd);
    if (data == NULL) {
        MYGPIOD_LOG_ERROR("Data for fd not found");
        return false;
    }
    const int input_size = sizeof(struct t_input_event);
    struct t_input_event input_data;
    memset(&input_data, 0, input_size);
    errno = 0;
    ssize_t nread = read(*fd, &input_data, input_size);
    if (nread < 0) {
        MYGPIOD_LOG_ERROR("Failure reading from input device %s", data->name);
        MYGPIOD_LOG_ERRNO(errno);
        return false;
    }
    switch (input_data.type) {
        case EV_KEY:
        case EV_REL:
        case EV_ABS:
        case EV_SW:
            input_action_handle(config, data, &input_data);
            break;
        default:
            MYGPIOD_LOG_DEBUG("%s: Ignoring event type %s (%hu) with code=%hu value=%u",
                data->name,
                input_event_type_name(input_data.type),
                input_data.type,
                input_data.code,
                input_data.value
            );
    }
    return true;
}

// Private functions

/**
 * Get the input data by fd object
 * @param inputs Pointer to inputs list
 * @param fd fd to find
 * @return struct t_input_data* or NULL if not found
 */
static struct t_input_data *get_input_data_by_fd(struct t_list *inputs, int *fd) {
    struct t_list_node *current = inputs->head;
    while (current != NULL) {
        struct t_input_data *data = (struct t_input_data *)current->data;
        if (data->fd == *fd) {
            return data;
        }
        current = current->next;
    }
    return NULL;
}
