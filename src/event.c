/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "event.h"

#include "gpio.h"
#include "list.h"
#include "log.h"
#include "timer.h"

/**
 * Adds a fd to the list of fds to poll
 * @param poll_fds struct to add the new fd
 * @param fd fd to add
 * @param pfd_type type of poll fd
 * @return true on success, else false
 */
bool event_poll_fd_add(struct t_poll_fds *poll_fds, int fd, int pfd_type) {
    if (poll_fds->len == MAX_FDS) {
        MYGPIOD_LOG_ERROR("Maximum number of poll fds reached");
        return false;
    }
    poll_fds->fd[poll_fds->len].fd = fd;
    poll_fds->fd[poll_fds->len].events = POLLIN | POLLPRI;
    poll_fds->type[poll_fds->len] = pfd_type;
    poll_fds->len++;
    return true;
}

/**
 * Adds the timer fds to the poll_fds
 * @param config pointer to config
 * @param poll_fds t_poll_fds struct to populate
 */
void event_add_timer_fds(struct t_config *config, struct t_poll_fds *poll_fds) {
    struct t_list_node *current = config->gpios_in.head;
    while (current != NULL) {
        struct t_gpio_node_in *data = (struct t_gpio_node_in *)current->data;
        if (data->timer_fd > 0) {
            event_poll_fd_add(poll_fds, data->timer_fd, PFD_TYPE_TIMER);
        }
        current = current->next;
    }
}

/**
 * Adds the timer fds to the poll_fds
 * @param config pointer to config
 * @param poll_fds t_poll_fds struct to populate
 */
bool event_read_delegate(struct t_config *config, struct t_poll_fds *poll_fds) {
    for (unsigned i = 0; i < poll_fds->len; i++) {
        if (poll_fds->fd[i].revents) {
            switch(poll_fds->type[i]) {
                case PFD_TYPE_GPIO:
                    return gpio_handle_event(config, i);
                case PFD_TYPE_TIMER:
                    return timer_handle_event(&poll_fds->fd[i].fd, config, i);
                case PFD_TYPE_SIGNAL:
                    MYGPIOD_LOG_DEBUG("%u: Signal event detected", i);
                    return false;
            }
        }
    }
    return false;
}
