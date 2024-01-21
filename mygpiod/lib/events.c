/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/lib/events.h"

#include "mygpiod/lib/config.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/mem.h"
#include "mygpiod/server/idle.h"
#include "mygpiod/server/socket.h"

#include <stdint.h>

// private functions

static struct t_event_data *event_data_new(enum mygpiod_event_types mygpiod_event_type,
        uint64_t timestamp);

// public functions

/**
 * Enqueues a GPIO event for all client connections
 * @param config pointer to config
 * @param gpio gpio number of the event
 * @param event_type the mygpiod event type
 * @param timestamp event timestamp in nanoseconds
 */
void event_enqueue(struct t_config *config, unsigned gpio, enum mygpiod_event_types event_type,
        uint64_t timestamp)
{
    struct t_list_node *current = config->clients.head;
    while (current != NULL) {
        struct t_client_data *data = (struct t_client_data *)current->data;
        struct t_event_data *event_data = event_data_new(event_type, timestamp);
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
 * @return Event type name
 */
const char *mygpiod_event_name(enum mygpiod_event_types event_type) {
    switch(event_type) {
        case MYGPIOD_EVENT_FALLING:
            return "falling";
        case MYGPIOD_EVENT_RISING:
            return "rising";
        case MYGPIOD_EVENT_LONG_PRESS:
            return "long_press";
        case MYGPIOD_EVENT_LONG_PRESS_RELEASE:
            return "long_press_release";
    }
    return "";
}

// private functions

/**
 * Creates the event data
 * @param mygpiod_event_type event data type
 * @param timestamp event timestamp in nanoseconds
 * @return Newly allocated struct
 */
static struct t_event_data *event_data_new(enum mygpiod_event_types mygpiod_event_type,
        uint64_t timestamp)
{
    struct t_event_data *event_data = malloc_assert(sizeof(struct t_event_data));
    event_data->mygpiod_event_type = mygpiod_event_type;
    event_data->timestamp = timestamp;
    return event_data;
}
