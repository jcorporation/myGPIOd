/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Input device event handling
 */

#include "compile_time.h"
#include "mygpiod/input_ev/event.h"

#include "mygpiod/config/config.h"
#include "mygpiod/input_ev/action.h"
#include "mygpiod/input_ev/event_type.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"

#include <errno.h>
#include <linux/input-event-codes.h>
#include <string.h>
#include <unistd.h>

// Private definitions

static struct t_input_device *get_input_device_by_fd(struct t_list *inputs, int *fd);

// Public functions

/**
 * Reads the event data from an input event
 * @param config Pointer to config
 * @param fd Pointer to file descriptor with data to read
 * @returns true on success, else false
 */
bool input_handle_event(struct t_config *config, int *fd) {
    struct t_mygpiod_input_event input_event;
    input_event.device = get_input_device_by_fd(&config->input_devices, fd);
    if (input_event.device == NULL) {
        MYGPIOD_LOG_ERROR("Data for fd not found");
        return false;
    }
    const int input_size = sizeof(struct t_input_event);
    memset(&input_event.data, 0, input_size);
    errno = 0;
    ssize_t nread = read(*fd, &input_event.data, input_size);
    if (nread < 0) {
        MYGPIOD_LOG_ERROR("Failure reading from input device %s", input_event.device->name);
        MYGPIOD_LOG_ERRNO(errno);
        return false;
    }
    switch (input_event.data.type) {
        case EV_KEY:
        case EV_REL:
        case EV_ABS:
        case EV_SW:
            input_action_handle(config, &input_event);
            break;
        default:
            MYGPIOD_LOG_DEBUG("%s: Ignoring event type %s (%hu) with code=%hu value=%u",
                input_event.device->name,
                input_event_type_name(input_event.data.type),
                input_event.data.type,
                input_event.data.code,
                input_event.data.value
            );
    }
    return true;
}

// Private functions

/**
 * Get the input data by fd object
 * @param inputs Pointer to inputs list
 * @param fd fd to find
 * @return struct t_input_device* or NULL if not found
 */
static struct t_input_device *get_input_device_by_fd(struct t_list *inputs, int *fd) {
    struct t_list_node *current = inputs->head;
    while (current != NULL) {
        struct t_input_device *data = (struct t_input_device *)current->data;
        if (data->fd == *fd) {
            return data;
        }
        current = current->next;
    }
    return NULL;
}
