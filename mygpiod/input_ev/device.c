/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Input device event handling
 */

#include "compile_time.h"
#include "mygpiod/input_ev/device.h"

#include "mygpiod/config/config.h"
#include "mygpiod/config/input_ev.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>

/**
 * Opens the input devices and add's it to the poll fds array
 * @param config Pointer to config
 * @param poll_fds Pointer to poll_fds array
 * @return true on success, else false
 */
bool input_device_open(struct t_config *config, struct t_poll_fds *poll_fds) {
    if (config->input_devices.length == 0) {
        MYGPIOD_LOG_INFO("No inputs for monitoring configured");
        return true;
    }
    MYGPIOD_LOG_INFO("Requesting inputs");
    struct t_list_node *current = config->input_devices.head;
    while (current != NULL) {
        struct t_input_device *device = (struct t_input_device *)current->data;
        errno = 0;
        MYGPIOD_LOG_INFO("Opening input %s", device->name);
        device->fd = open(device->name, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (device->fd < 0) {
            MYGPIOD_LOG_ERROR("Failure opening %s", device->name);
            MYGPIOD_LOG_ERRNO(errno);
        }
        else {
            event_poll_fd_add(poll_fds, device->fd, PFD_TYPE_INPUT, POLLIN | POLLPRI);
        }
        current = current->next;
    }
    return true;
}

/**
 * Get the input data by fd object
 * @param inputs Pointer to inputs list
 * @param fd fd to find
 * @return struct t_input_device* or NULL if not found
 */
struct t_input_device *input_device_get_by_fd(struct t_list *inputs, int *fd) {
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

/**
 * Gets the input device struct by device name
 * @param input_devices Pointer to list of input devices
 * @param device Device name
 * @return struct t_input_data* or NULL if not found
 */
struct t_input_device *input_device_get_by_name(struct t_list *input_devices, const char *device) {
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
