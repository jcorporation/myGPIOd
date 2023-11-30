/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "gpio.h"

#include "action.h"
#include "log.h"
#include "util.h"

#include <errno.h>
#include <gpiod.h>
#include <string.h>

// private definitions
static int line_request_flags(bool active_low, int bias);

// public functions

/**
 * Opens the gpio chip.
 * @param config pointer to config
 * @return true on success, else false
 */
bool gpio_open_chip(struct t_config *config) {
    MYGPIOD_LOG_INFO("Opening chip \"%s\"", config->chip_name);
    config->chip = gpiod_chip_open_lookup(config->chip_name);
    if (config->chip == NULL) {
        MYGPIOD_LOG_ERROR("Error opening chip");
        return false;
    }
    return true;
}

/**
 * Handles a gpio event.
 * @param config pointer to config
 * @param idx offset for bulk_in
 * @return true on success, else false
 */
bool gpio_handle_event(struct t_config *config, unsigned idx) {
    MYGPIOD_LOG_DEBUG("%u: Gpio event detected", idx);
    struct gpiod_line *line = gpiod_line_bulk_get_line(config->bulk_in, idx);
    struct gpiod_line_event event;
    int rv = gpiod_line_event_read(line, &event);
    if (rv < 0) {
        return false;
    }
    struct t_list_node *node = list_node_at(&config->gpios_in, idx);
    struct t_gpio_node_in *data = (struct t_gpio_node_in *)node->data;
    // abort pending long press event
    action_delay_abort(data);
    action_handle(node->gpio, &event.ts, event.event_type, data);
    return true;
}

/**
 * Sets the output gpios in bulk.
 * @param config pointer to config
 * @return true on success, else false
 */
bool gpio_set_outputs(struct t_config *config) {
    if (config->gpios_out.length == 0) {
        MYGPIOD_LOG_INFO("No output gpios configured");
        return true;
    }
    MYGPIOD_LOG_INFO("Setting output gpios");
    int gpios_out_values[GPIOD_LINE_BULK_MAX_LINES];
    struct gpiod_line_bulk bulk_out;
    gpiod_line_bulk_init(&bulk_out);
    struct t_list_node *current = config->gpios_out.head;
    unsigned i = 0;
    struct gpiod_line *line;
    while (current != NULL) {
        line = gpiod_chip_get_line(config->chip, current->gpio);
        if (line == NULL) {
            MYGPIOD_LOG_ERROR("Error getting gpio \"%u\"", current->gpio);
            return false;
        }
        struct t_gpio_node_out *data = (struct t_gpio_node_out *)current->data;
        MYGPIOD_LOG_INFO("Setting gpio \"%u\" as output to value \"%s\"",
            current->gpio, lookup_gpio_value(data->value));
        gpiod_line_bulk_add(&bulk_out, line);
        gpios_out_values[i] = data->value;
        current = current->next;
        i++;
    }

    // set values
    int req_flags = line_request_flags(config->active_low, 0);
    errno = 0;
    int rv = gpiod_line_request_bulk_output_flags(&bulk_out, MYGPIOD_NAME,
        req_flags, gpios_out_values);
    if (rv == -1) {
        MYGPIOD_LOG_ERROR("Error setting gpios: %s", strerror(errno));
        return false;
    }
    return true;
}

/**
 * Request the input gpios.
 * @param config pointer to config
 * @return true on success, else false
 */
bool gpio_request_inputs(struct t_config *config, struct t_poll_fds *poll_fds) {
    if (config->gpios_in.length == 0) {
        MYGPIOD_LOG_INFO("No gpios for monitoring configured");
        return true;
    }
    MYGPIOD_LOG_INFO("Requesting input gpios");
    struct gpiod_line_bulk bulk_in;
    gpiod_line_bulk_init(&bulk_in);
    config->bulk_in = &bulk_in;
    struct gpiod_line *line;
    struct t_list_node *current = config->gpios_in.head;
    while (current != NULL) {
        line = gpiod_chip_get_line(config->chip, current->gpio);
        if (line == NULL) {
            MYGPIOD_LOG_ERROR("Error getting gpio \"%u\"", current->gpio);
            return false;
        }
        struct t_gpio_node_in *data = (struct t_gpio_node_in *)current->data;
        MYGPIOD_LOG_INFO("Setting gpio \"%u\" as input, monitoring event: %s",
            current->gpio, lookup_event_request(data->request_event));
        gpiod_line_bulk_add(&bulk_in, line);
        current = current->next;
    }

    // set request flags
    struct gpiod_line_request_config conf;
    conf.flags = line_request_flags(config->active_low, config->bias);
    conf.consumer = MYGPIOD_NAME;
    conf.request_type = config->event_request;

    // request the gpios
    errno = 0;
    int rv = gpiod_line_request_bulk(&bulk_in, &conf, NULL);
    if (rv == -1) {
        MYGPIOD_LOG_ERROR("Error requesting gpios: %s", strerror(errno));
        return false;
    }

    // get fds
    current= config->gpios_in.head;
    unsigned i = 0;
    while (current != NULL) {
        struct t_gpio_node_in *data = (struct t_gpio_node_in *)current->data;
        line = gpiod_line_bulk_get_line(&bulk_in, i);
        data->fd = gpiod_line_event_get_fd(line);
        event_poll_fd_add(poll_fds, data->fd, PFD_TYPE_GPIO);
        current = current->next;
        i++;
    }
    return true;
}

// private functions

/**
 * Sets the line requests flags from active_low and bias.
 * @param active_low active_low settings
 * @param bias bias
 * @return the line request flags
 */
static int line_request_flags(bool active_low, int bias) {
    int req_flags = 0;

    if (active_low == true) {
        req_flags |= GPIOD_LINE_REQUEST_FLAG_ACTIVE_LOW;
    }
    if (bias == GPIOD_LINE_BIAS_DISABLE) {
        req_flags |= GPIOD_LINE_REQUEST_FLAG_BIAS_DISABLE;
    }
    else if (bias == GPIOD_LINE_BIAS_PULL_DOWN) {
        req_flags |= GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_DOWN;
    }
    else if (bias == GPIOD_LINE_BIAS_PULL_UP) {
        req_flags |= GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP;
    }

    return req_flags;
}
