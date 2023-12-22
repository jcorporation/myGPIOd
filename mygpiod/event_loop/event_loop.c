/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/event_loop/event_loop.h"

#include "mygpiod/gpio/event.h"
#include "mygpiod/gpio/timer.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/timer.h"
#include "mygpiod/server/socket.h"

#include <fcntl.h>
#include <sys/socket.h>

/**
 * Adds a fd to the list of fds to poll
 * @param poll_fds struct to add the new fd
 * @param fd fd to add
 * @param pfd_type type of poll fd
 * @return true on success, else false
 */
bool event_poll_fd_add(struct t_poll_fds *poll_fds, int fd, int pfd_type, short events) {
    if (poll_fds->len == MAX_FDS) {
        MYGPIOD_LOG_ERROR("Maximum number of poll fds reached");
        return false;
    }
    MYGPIOD_LOG_DEBUG("Adding poll fd#%u of type %d", poll_fds->len, pfd_type);
    poll_fds->fd[poll_fds->len].fd = fd;
    poll_fds->fd[poll_fds->len].events = events;
    poll_fds->type[poll_fds->len] = pfd_type;
    poll_fds->len++;
    return true;
}

/**
 * Adds the timer fds from input GPIOs to the poll_fds
 * @param config pointer to config
 * @param poll_fds t_poll_fds struct to populate
 */
void event_add_gpio_in_timer_fds(struct t_config *config, struct t_poll_fds *poll_fds) {
    struct t_list_node *current = config->gpios_in.head;
    while (current != NULL) {
        struct t_gpio_in_data *data = (struct t_gpio_in_data *)current->data;
        if (data->timer_fd > 0) {
            event_poll_fd_add(poll_fds, data->timer_fd, PFD_TYPE_GPIO_IN_TIMER, POLLIN | POLLPRI);
        }
        current = current->next;
    }
}

/**
 * Adds the timer fds from output GPIOs to the poll_fds
 * @param config pointer to config
 * @param poll_fds t_poll_fds struct to populate
 */
void event_add_gpio_out_timer_fds(struct t_config *config, struct t_poll_fds *poll_fds) {
    struct t_list_node *current = config->gpios_out.head;
    while (current != NULL) {
        struct t_gpio_out_data *data = (struct t_gpio_out_data *)current->data;
        if (data->timer_fd > 0) {
            event_poll_fd_add(poll_fds, data->timer_fd, PFD_TYPE_GPIO_OUT_TIMER, POLLIN | POLLPRI);
        }
        current = current->next;
    }
}

/**
 * Adds the timer fds to the poll_fds
 * @param config pointer to config
 * @param poll_fds t_poll_fds struct to populate
 */
void event_add_client_fds(struct t_config *config, struct t_poll_fds *poll_fds) {
    struct t_list_node *current = config->clients.head;
    while (current != NULL) {
        struct t_client_data *data = (struct t_client_data *)current->data;
        if (data->fd > 0) {
            event_poll_fd_add(poll_fds, data->fd, PFD_TYPE_CLIENT, data->events);
        }
        if (data->timeout_fd > 0) {
            event_poll_fd_add(poll_fds, data->timeout_fd, PFD_TYPE_CLIENT_TIMEOUT, POLLIN | POLLPRI);
            timer_log_next_expire(data->timeout_fd);
        }
        current = current->next;
    }
}

/**
 * Delegates the read events by type
 * @param config pointer to config
 * @param poll_fds t_poll_fds struct to populate
 */
bool event_read_delegate(struct t_config *config, struct t_poll_fds *poll_fds) {
    for (unsigned i = 0; i < poll_fds->len; i++) {
        if (poll_fds->fd[i].revents) {
            MYGPIOD_LOG_DEBUG("%u: Event detected of type %d", i, poll_fds->type[i]);
            switch(poll_fds->type[i]) {
                case PFD_TYPE_GPIO:
                    gpio_handle_event(config, &poll_fds->fd[i].fd);
                    return true;
                case PFD_TYPE_GPIO_IN_TIMER:
                    gpio_in_timer_handle_event(config, &poll_fds->fd[i].fd);
                    return true;
                case PFD_TYPE_GPIO_OUT_TIMER:
                    gpio_out_timer_handle_event(config, &poll_fds->fd[i].fd);
                    return true;
                case PFD_TYPE_SIGNAL:
                    return false;
                case PFD_TYPE_CONNECT:
                    server_client_connection_accept(config, &poll_fds->fd[i].fd);
                    return true;
                case PFD_TYPE_CLIENT:
                    server_client_connection_handle(config, &poll_fds->fd[i]);
                    return true;
                case PFD_TYPE_CLIENT_TIMEOUT:
                    server_client_timeout(&config->clients, &poll_fds->fd[i].fd);
                    return true;
            }
        }
    }
    return true;
}
