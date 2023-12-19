/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/gpio/event.h"

#include "mygpiod/gpio/action.h"
#include "mygpiod/lib/config.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"

#include <gpiod.h>
#include <string.h>

// private definitions

static struct t_list_node *get_node_by_gpio_fd(struct t_list *gpios_in, int *gpio_fd);

// public functions

/**
 * Handles a gpio event.
 * @param config pointer to config
 * @param fd file descriptor
 * @return true on success, else false
 */
bool gpio_handle_event(struct t_config *config, int *fd) {
    struct t_list_node *node = get_node_by_gpio_fd(&config->gpios_in, fd);
    if (node == NULL) {
        MYGPIOD_LOG_ERROR("Could not get gpio node");
        return false;
    }
    MYGPIOD_LOG_DEBUG("Event detected for GPIO %u", node->id);
    struct t_gpio_in_data *data = (struct t_gpio_in_data *)node->data;

    int ret = gpiod_line_request_read_edge_events(data->request,
                data->event_buffer, GPIO_EVENT_BUF_SIZE);
    if (ret < 0) {
        MYGPIOD_LOG_ERROR("Error reading line events");
        return false;
    }

    for (int j = 0; j < ret; j++) {
        struct gpiod_edge_event *event = gpiod_edge_event_buffer_get_event(data->event_buffer, (unsigned long)j);
        if (event == NULL) {
            MYGPIOD_LOG_ERROR("Unable to retrieve event from buffer");
            continue;
        }
        action_delay_abort(data);
        action_handle(config, node->id, gpiod_edge_event_get_timestamp_ns(event),
            gpiod_edge_event_get_event_type(event), data);
    }
    return true;
}

// private functions

/**
 * Gets the client node by gpio fd
 * @param clients list of clients
 * @param fd gpio fd
 * @return the list node or NULL on error
 */
static struct t_list_node *get_node_by_gpio_fd(struct t_list *gpios_in, int *gpio_fd) {
    struct t_list_node *current = gpios_in->head;
    while (current != NULL) {
        struct t_gpio_in_data *data = (struct t_gpio_in_data *)current->data;
        if (data->gpio_fd == *gpio_fd) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}
