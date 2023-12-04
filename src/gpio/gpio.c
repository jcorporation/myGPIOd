/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "src/gpio/gpio.h"

#include "src/gpio/action.h"
#include "src/lib/config.h"
#include "src/lib/list.h"
#include "src/lib/log.h"
#include "src/lib/util.h"

#include <assert.h>
#include <errno.h>
#include <gpiod.h>
#include <string.h>

// private definitions

static struct t_list_node *get_node_by_gpio_fd(struct t_list *gpios_in, int *gpio_fd);

// public functions

/**
 * Opens the gpio chip.
 * @param config pointer to config
 * @return true on success, else false
 */
bool gpio_open_chip(struct t_config *config) {
    MYGPIOD_LOG_INFO("Opening chip \"%s\"", config->chip_path);
    config->chip = gpiod_chip_open(config->chip_path);
    if (config->chip == NULL) {
        MYGPIOD_LOG_ERROR("Error opening chip");
        return false;
    }
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
    struct t_list_node *current = config->gpios_out.head;
    while (current != NULL) {
        struct t_gpio_out_data *data = (struct t_gpio_out_data *)current->data;
        gpio_set_output(config->chip, current->id, data);
        current = current->next;
    }
    return true;
}

/**
 * Configures a gpio as output
 * @param chip gpio chip
 * @param gpio gpio number
 * @param data gpio configuration data
 * @return true on success, else false
 */
bool gpio_set_output(struct gpiod_chip *chip, unsigned gpio, struct t_gpio_out_data *data) {
    MYGPIOD_LOG_INFO("Setting gpio \"%u\" as output to value \"%s\"",
        gpio, lookup_gpio_value(data->value));
    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (settings == NULL) {
        MYGPIOD_LOG_ERROR("Unable to allocate line settings");
        assert(settings);
    }
    gpiod_line_settings_set_bias(settings, data->bias);
    gpiod_line_settings_set_drive(settings, data->drive);
    gpiod_line_settings_set_active_low(settings, data->active_low);
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);

    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    if (req_cfg == NULL) {
        MYGPIOD_LOG_ERROR("Unable to allocate the request config structure");
        assert(req_cfg);
    }
    gpiod_request_config_set_consumer(req_cfg, MYGPIOD_NAME);

    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    if (line_cfg == NULL) {
        MYGPIOD_LOG_ERROR("Unable to allocate the line config structure");
        assert(line_cfg);
    }
    gpiod_line_config_reset(line_cfg);
    unsigned offsets[1];
    offsets[0] = gpio;
    enum gpiod_line_value values[1];
    values[0] = data->value;

    bool rc = true;
    if (gpiod_line_config_add_line_settings(line_cfg, offsets, 1, settings) == -1 ||
        gpiod_line_config_set_output_values(line_cfg, values, 1) == -1)
    {
        MYGPIOD_LOG_ERROR("Unable to add line setting and value");
        rc = false;
    }
    else {
        data->request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
    }
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    return rc;
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
    struct t_list_node *current = config->gpios_in.head;
    while (current != NULL) {
        struct t_gpio_in_data *data = (struct t_gpio_in_data *)current->data;
        if (gpio_request_input(config->chip, current->id, data) == true) {
            event_poll_fd_add(poll_fds, data->gpio_fd, PFD_TYPE_GPIO, POLLIN | POLLPRI);
        }
        current = current->next;
    }
    return true;
}

bool gpio_request_input(struct gpiod_chip *chip, unsigned gpio, struct t_gpio_in_data *data) {
    MYGPIOD_LOG_INFO("Setting gpio \"%u\" as input, monitoring event: %s",
            gpio, lookup_event_request(data->request_event));
    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (settings == NULL) {
        MYGPIOD_LOG_ERROR("Unable to allocate line settings");
        assert(settings);
    }
    gpiod_line_settings_set_bias(settings, data->bias);
    gpiod_line_settings_set_active_low(settings, data->active_low);
    gpiod_line_settings_set_debounce_period_us(settings, data->debounce_period_us);
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_edge_detection(settings, data->request_event);

    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    if (req_cfg == NULL) {
        MYGPIOD_LOG_ERROR("Unable to allocate the request config structure");
        assert(req_cfg);
    }
    gpiod_request_config_set_consumer(req_cfg, MYGPIOD_NAME);

    data->event_buffer = gpiod_edge_event_buffer_new(GPIO_EVENT_BUF_SIZE);
    if (data->event_buffer == NULL) {
        MYGPIOD_LOG_ERROR("Unable to allocate the line event buffer");
        assert(data->event_buffer);
    }

    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    if (line_cfg == NULL) {
        MYGPIOD_LOG_ERROR("Unable to allocate the line config structure");
        assert(line_cfg);
    }
    gpiod_line_config_reset(line_cfg);
    unsigned offsets[1];
    offsets[0] = gpio;

    bool rc = true;
    if (gpiod_line_config_add_line_settings(line_cfg, offsets, 1, settings) == -1) {
        MYGPIOD_LOG_ERROR("Unable to add line setting and value");
        rc = false;
    }
    else {
        data->request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
        if (data->request != NULL) {
            data->gpio_fd = gpiod_line_request_get_fd(data->request);
        }
        else {
            MYGPIOD_LOG_ERROR("Unable to request line");
        }
    }
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    return rc;
}

/**
 * Handles a gpio event.
 * @param config pointer to config
 * @param fd file descriptor
 * @return true on success, else false
 */
bool gpio_handle_event(struct t_config *config, int *fd) {
    struct t_list_node *node = get_node_by_gpio_fd(&config->gpios_in, fd);
    if (node == NULL) {
        MYGPIOD_LOG_ERROR("Could not gpio node");
        return false;
    }
    MYGPIOD_LOG_DEBUG("Gpio event detected %u", node->id);
    struct t_gpio_in_data *data = (struct t_gpio_in_data *)node;

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

/**
 * Gets the current line value of a input gpio
 * @param config pointer to config
 * @param gpio gpio to get the value
 * @return the active state
 */
enum gpiod_line_value gpio_get_value(struct t_config *config, unsigned gpio) {
    struct t_list_node *node = list_node_by_id(&config->gpios_in, gpio);
    struct t_gpio_in_data *data = (struct t_gpio_in_data *)node->data;
    return gpiod_line_request_get_value(data->request, gpio);
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
