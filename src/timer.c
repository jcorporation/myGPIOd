/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "timer.h"

#include "action.h"
#include "log.h"

#include <stdint.h>
#include <unistd.h>

// privat definitions
static struct t_list_node *get_node_by_timerfd(struct t_list *gpios_in, int *fd);

// public functions

/**
 * Handles a timer event
 * @param fd triggered fd
 * @param config pointer to config
 * @param idx 
 * @return true on success, else false
 */
bool timer_handle_event(int *fd, struct t_config *config, unsigned idx) {
    MYGPIOD_LOG_DEBUG("%u: Long press timer event detected", idx);
    uint64_t exp;
    ssize_t s = read(*fd, &exp, sizeof(uint64_t));
    if (s == sizeof(uint64_t) && exp > 1) {
        struct t_list_node *node = get_node_by_timerfd(&config->gpios_in, fd);
        if (fd == NULL) {
            MYGPIOD_LOG_ERROR("Error getting node for timer_fd");
            return false;
        }
        struct t_gpio_node_in *data = (struct t_gpio_node_in *)node->data;
        action_execute_delayed(node->gpio, data, config);
        return true;
    }
    return false;
}

// private functions

/**
 * Gets the gpio in node by timerfd
 * @param gpios_in list of in gpios
 * @param fd timer_fd
 * @return the list node or NULL on error
 */
static struct t_list_node *get_node_by_timerfd(struct t_list *gpios_in, int *fd) {
    struct t_list_node *current = gpios_in->head;
    while (current != NULL) {
        struct t_gpio_node_in *data = (struct t_gpio_node_in *)current->data;
        if (data->timer_fd == *fd) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}
