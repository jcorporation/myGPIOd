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
#include "mygpiod/input_ev/device.h"
#include "mygpiod/input_ev/event_type.h"
#include "mygpiod/lib/log.h"

#include <errno.h>
#include <linux/input-event-codes.h>
#include <string.h>
#include <unistd.h>

/**
 * Reads the event data from an input event
 * @param config Pointer to config
 * @param fd Pointer to file descriptor with data to read
 * @returns true on success, else false
 */
bool input_ev_handle_event(struct t_config *config, int *fd) {
    struct t_mygpiod_input_event input_event;
    input_event.device = input_device_get_by_fd(&config->input_devices, fd);
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
            input_ev_action_handle(config, &input_event);
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
