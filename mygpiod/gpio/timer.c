/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/gpio/timer.h"

#include "mygpiod/event_loop/event_loop.h"
#include "mygpiod/gpio/action.h"
#include "mygpiod/gpio/output.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/timer.h"

#include <gpiod.h>
#include <stdint.h>
#include <unistd.h>

// privat definitions
static bool read_from_timerfd(int *fd);
static struct t_list_node *get_node_by_gpio_in_timerfd(struct t_list *gpios_in, int *fd);
static struct t_list_node *get_node_by_gpio_out_timerfd(struct t_list *gpios_out, int *fd);

// public functions

/**
 * Handles a timer event for an input GPIO.
 * @param fd triggered fd
 * @param config pointer to config
 * @return true on success, else false
 */
bool gpio_in_timer_handle_event(struct t_config *config, int *fd) {
    if (read_from_timerfd(fd) == false) {
        return false;
    }
    timer_log_next_expire(*fd);
    struct t_list_node *node = get_node_by_gpio_in_timerfd(&config->gpios_in, fd);
    if (node == NULL) {
        MYGPIOD_LOG_ERROR("Error getting node for timer_fd");
        return false;
    }
    MYGPIOD_LOG_INFO("Long press event for gpio \"%u\"", node->id);
    struct t_gpio_in_data *data = (struct t_gpio_in_data *)node->data;
    action_execute_delayed(node->id, data, config);
    return true;
}

/**
 * Handles a timer event for an output GPIO.
 * This handles currently only blink.
 * @param fd triggered fd
 * @param config pointer to config
 * @return true on success, else false
 */
bool gpio_out_timer_handle_event(struct t_config *config, int *fd) {
    if (read_from_timerfd(fd) == false) {
        return false;
    }
    timer_log_next_expire(*fd);
    struct t_list_node *node = get_node_by_gpio_out_timerfd(&config->gpios_out, fd);
    if (node == NULL) {
        MYGPIOD_LOG_ERROR("Error getting node for timer_fd");
        return false;
    }
    MYGPIOD_LOG_INFO("Blink event for gpio \"%u\"", node->id);
    struct t_gpio_out_data *data = (struct t_gpio_out_data *)node->data;
    gpio_toggle_value_by_line_request(config, data->request, node->id);
    if (timer_repeat(*fd) == false) {
        // Blink only once
        close_fd(&data->timer_fd);
    }
    return true;
}

// private functions

/**
 * Reads the uint64_t value from a timer fd
 * @param fd fd to read from
 * @return true on success, else false
 */
static bool read_from_timerfd(int *fd) {
    uint64_t exp;
    ssize_t s = read(*fd, &exp, sizeof(uint64_t));
    if (s != sizeof(uint64_t)) {
        MYGPIOD_LOG_ERROR("Unable reading from timer_fd");
        return false;
    }
    return true;
}

/**
 * Gets the gpio in node by timer fd
 * @param gpios_in list of input GPIOs
 * @param fd timer_fd
 * @return the list node or NULL on error
 */
static struct t_list_node *get_node_by_gpio_in_timerfd(struct t_list *gpios_in, int *fd) {
    struct t_list_node *current = gpios_in->head;
    while (current != NULL) {
        struct t_gpio_in_data *data = (struct t_gpio_in_data *)current->data;
        if (data->timer_fd == *fd) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * Gets the gpio out node by timer fd
 * @param gpios_out list of output GPIOs
 * @param fd timer_fd
 * @return the list node or NULL on error
 */
static struct t_list_node *get_node_by_gpio_out_timerfd(struct t_list *gpios_out, int *fd) {
    struct t_list_node *current = gpios_out->head;
    while (current != NULL) {
        struct t_gpio_out_data *data = (struct t_gpio_out_data *)current->data;
        if (data->timer_fd == *fd) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}
