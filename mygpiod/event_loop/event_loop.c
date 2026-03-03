/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Central event loop
 */

#include "compile_time.h"
#include "mygpiod/event_loop/event_loop.h"

#include "mygpiod/config/gpio.h"
#include "mygpiod/gpio/event.h"
#include "mygpiod/gpio/timer.h"
#include "mygpiod/input_ev/event.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/timer.h"
#include "mygpiod/server_socket/socket.h"
#include "mygpiod/timer_ev/event.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

/**
 * Global variable that indicates if the poll fds array must be updated
 */
bool update_pollfds;

/**
 * Lookups the poll fds event type as string
 * @param type poll fd type to lookup
 * @return poll fd type as string
 */
const char *lookup_pfd_type(enum pfd_types type) {
    switch(type) {
        case PFD_TYPE_GPIO:
            return "gpio";
        case PFD_TYPE_GPIO_IN_TIMER:
            return "gpio_in_timer";
        case PFD_TYPE_GPIO_OUT_TIMER:
            return "gpio_out_timer";
        case PFD_TYPE_SIGNAL:
            return "signal";
        case PFD_TYPE_CONNECT:
            return "server socket";
        case PFD_TYPE_CLIENT:
            return "client socket";
        case PFD_TYPE_CLIENT_TIMEOUT:
            return "timeout";
    #ifdef MYGPIOD_ENABLE_HTTPD
        case PFD_TYPE_HTTPD:
            return "httpd";
    #endif
        case PFD_TYPE_INPUT:
            return "input";
        case PFD_TYPE_TIMER_EV:
            return "timer_ev";
    }
    return "unknown";
}

/**
 * Adds a fd to the list of fds to poll
 * @param poll_fds Struct to add the new fd
 * @param fd File descriptor to add
 * @param pfd_type Type of poll fd
 * @param events Events to poll for
 * @return true on success, else false
 */
bool event_poll_fd_add(struct t_poll_fds *poll_fds, int fd, enum pfd_types pfd_type, short events) {
    if (poll_fds->len == MAX_FDS) {
        MYGPIOD_LOG_ERROR("Maximum number of poll fds reached");
        return false;
    }
    MYGPIOD_LOG_DEBUG("Adding poll fd#%u of type \"%s\"", poll_fds->len, lookup_pfd_type(pfd_type));
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
 * Adds the socket client fds to the poll_fds
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
            timer_log_next_expire("Client timeout", data->timeout_fd);
        }
        current = current->next;
    }
}

/**
 * Closes an open file descriptor.
 * Checks if it is open and sets it to -1.
 * @param fd file descriptor to close
 */
void close_fd(int *fd) {
    if (*fd > -1) {
        close(*fd);
        *fd = -1;
        update_pollfds = true;
    }
}

/**
 * Delegates the read events by type
 * @param config pointer to config
 * @param poll_fds t_poll_fds struct to populate
 * @returns true, false to signal to exit the event loop
 */
bool event_read_delegate(struct t_config *config, struct t_poll_fds *poll_fds) {
    for (unsigned i = 0; i < poll_fds->len; i++) {
        if (poll_fds->fd[i].revents) {
            MYGPIOD_LOG_DEBUG("%u: Event detected of type \"%s\": %d", i, lookup_pfd_type(poll_fds->type[i]), poll_fds->fd[i].revents);
            switch(poll_fds->type[i]) {
                case PFD_TYPE_GPIO:
                    gpio_handle_event(config, &poll_fds->fd[i].fd);
                    return true;
                case PFD_TYPE_GPIO_IN_TIMER:
                    if (poll_fds->fd[i].revents & POLLIN) {
                        gpio_in_timer_handle_event(config, &poll_fds->fd[i].fd);
                    }
                    return true;
                case PFD_TYPE_GPIO_OUT_TIMER:
                    if (poll_fds->fd[i].revents & POLLIN) {
                        gpio_out_timer_handle_event(config, &poll_fds->fd[i].fd);
                    }
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
            #ifdef MYGPIOD_ENABLE_HTTPD
                case PFD_TYPE_HTTPD:
                    // MHD is called in each poll loop iteration, no need to do it here explicitly
                    return true;
            #endif
                case PFD_TYPE_INPUT:
                    input_ev_handle_event(config, &poll_fds->fd[i].fd);
                    return true;
                case PFD_TYPE_TIMER_EV:
                    timer_ev_handle_event(config, &poll_fds->fd[i].fd);
                    return true;
            }
        }
    }
    return true;
}
