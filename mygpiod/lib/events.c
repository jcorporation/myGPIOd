/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief General event (enqueue) functions
 */

#include "compile_time.h"
#include "mygpiod/lib/events.h"

#include "mygpiod/config/config.h"
#include "mygpiod/input/input_event.h"
#include "mygpiod/lib/event_types.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"
#include "mygpiod/server_http/util.h"
#include "mygpiod/server_socket/idle.h"
#include "mygpiod/server_socket/socket.h"

#include <stdint.h>
#include <string.h>

// private functions

static struct t_event_data *event_data_new_gpio(enum mygpiod_event_types mygpiod_event_type,
        uint64_t timestamp_ns);
static struct t_event_data *event_data_new_input(struct t_mygpiod_input_event *input_event);

// public functions

/**
 * Enqueues a GPIO event for all client connections - socket and http
 * @param config pointer to config
 * @param gpio gpio number of the event
 * @param event_type the mygpiod event type
 * @param timestamp event timestamp in nanoseconds
 */
void event_enqueue_gpio(struct t_config *config, unsigned gpio, enum mygpiod_event_types event_type,
        uint64_t timestamp)
{
    // Socket clients
    struct t_list_node *current = config->clients.head;
    while (current != NULL) {
        struct t_client_data *data = (struct t_client_data *)current->data;
        struct t_event_data *event_data = event_data_new_gpio(event_type, timestamp);
        MYGPIOD_LOG_DEBUG("Enqueuing event %s at gpio %u for client#%u", mygpiod_event_name(event_type), gpio, current->id);
        list_push(&data->waiting_events, gpio, event_data);
        if (data->state == CLIENT_SOCKET_STATE_IDLE) {
            send_idle_events(current, false);
        }
        else if (data->waiting_events.length > WAITING_EVENTS_MAX) {
            struct t_list_node *first = list_shift(&data->waiting_events);
            list_node_free(first, event_data_clear);
        }
        current = current->next;
    }

    // HTTP long polling
    current = config->http_suspended.head;
    while (current != NULL) {
        http_connection_resume_gpio((struct t_request_data *)current->data, gpio, event_type, timestamp);
        current = current->next;
    }
    list_clear(&config->http_suspended, NULL);
}

/**
 * Enqueues an input device event for all client connections - socket and http
 * @param config Pointer to config
 * @param input_event Input event
 */
void event_enqueue_input(struct t_config *config, struct t_mygpiod_input_event *input_event) {
    // Socket clients
    struct t_list_node *current = config->clients.head;
    while (current != NULL) {
        struct t_client_data *data = (struct t_client_data *)current->data;
        struct t_event_data *event_data = event_data_new_input(input_event);
        MYGPIOD_LOG_DEBUG("Enqueuing event %s for %s for client#%u",
            mygpiod_event_name(MYGPIOD_EVENT_INPUT),
            input_event->device->name, current->id
        );
        list_push(&data->waiting_events, 0, event_data);
        if (data->state == CLIENT_SOCKET_STATE_IDLE) {
            send_idle_events(current, false);
        }
        else if (data->waiting_events.length > WAITING_EVENTS_MAX) {
            struct t_list_node *first = list_shift(&data->waiting_events);
            list_node_free(first, event_data_clear);
        }
        current = current->next;
    }

    // HTTP long polling
    current = config->http_suspended.head;
    while (current != NULL) {
        http_connection_resume_input((struct t_request_data *)current->data, input_event);
        current = current->next;
    }
    list_clear(&config->http_suspended, NULL);
}

/**
 * Clears the event data.
 * @param node pointer to node holding the data to clear
 */
void event_data_clear(struct t_list_node *node) {
    struct t_event_data *data = (struct t_event_data *)node->data;
    (void)data;
}

/**
 * Returns the mygpiod event type as string.
 * @param event_type the event type
 * @return Event type name or empty on error
 */
const char *mygpiod_event_name(enum mygpiod_event_types event_type) {
    switch(event_type) {
        case MYGPIOD_EVENT_GPIO_FALLING:
            return "gpio_falling";
        case MYGPIOD_EVENT_GPIO_RISING:
            return "gpio_rising";
        case MYGPIOD_EVENT_GPIO_LONG_PRESS:
            return "gpio_long_press";
        case MYGPIOD_EVENT_GPIO_LONG_PRESS_RELEASE:
            return "gpio_long_press_release";
        case MYGPIOD_EVENT_INPUT:
            return "input";
    }
    return "";
}

// private functions

/**
 * Creates the event data for a GPIO
 * @param mygpiod_event_type event data type
 * @param timestamp_ns event timestamp in nanoseconds
 * @return Newly allocated struct
 */
static struct t_event_data *event_data_new_gpio(enum mygpiod_event_types mygpiod_event_type,
        uint64_t timestamp_ns)
{
    struct t_event_data *event_data = malloc_assert(sizeof(struct t_event_data));
    event_data->mygpiod_event_type = mygpiod_event_type;
    event_data->timestamp_ns = timestamp_ns;
    return event_data;
}

/**
 * Creates the event data for an input device
 * @param input_event Input event
 * @return Newly allocated struct
 */
static struct t_event_data *event_data_new_input(struct t_mygpiod_input_event *input_event) {
    struct t_event_data *event_data = malloc_assert(sizeof(struct t_event_data));
    event_data->mygpiod_event_type = MYGPIOD_EVENT_INPUT;
    event_data->timestamp_ns = (uint64_t)(input_event->data.time.tv_sec * 1000000) + (uint64_t)(input_event->data.time.tv_usec * 1000);
    event_data->input_event.device = input_event->device;
    memcpy(&event_data->input_event.data, &input_event->data, sizeof(struct t_input_event));
    return event_data;
}
