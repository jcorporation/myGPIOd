/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/gpio/input.h"

#include "mygpiod/lib/config.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/util.h"

#include <assert.h>
#include <errno.h>
#include <gpiod.h>
#include <string.h>

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
        if (gpio_request_input(config->chip, current->id, data) == false) {
            return false;
        }
        event_poll_fd_add(poll_fds, data->gpio_fd, PFD_TYPE_GPIO, POLLIN | POLLPRI);
        current = current->next;
    }
    return true;
}

/**
 * Requests an input gpio
 * @param chip gpio chip
 * @param gpio gpio number
 * @param data gpio configuration data
 * @return true on success, else false
 */
bool gpio_request_input(struct gpiod_chip *chip, unsigned gpio, struct t_gpio_in_data *data) {
    MYGPIOD_LOG_INFO("Setting gpio \"%u\" as input, monitoring event: %s",
            gpio, lookup_event_request(data->request_event));
    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (settings == NULL) {
        MYGPIOD_LOG_ERROR("Unable to allocate line settings");
        assert(settings);
    }
    if (gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT) == -1) {
        MYGPIOD_LOG_WARN("Unable to set direction");
    }
    if (gpiod_line_settings_set_bias(settings, data->bias) == -1) {
        MYGPIOD_LOG_WARN("Unable to set bias");
    }
    if (gpiod_line_settings_set_event_clock(settings, data->event_clock) == -1) {
        MYGPIOD_LOG_WARN("Unable to set event clock");
    }
    if (gpiod_line_settings_set_edge_detection(settings, data->request_event) == -1) {
        MYGPIOD_LOG_WARN("Unable to set edge detection");
    }
    gpiod_line_settings_set_active_low(settings, data->active_low);
    gpiod_line_settings_set_debounce_period_us(settings, data->debounce_period_us);

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
        MYGPIOD_LOG_ERROR("Unable to add line setting");
        rc = false;
    }
    else {
        data->request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
        if (data->request != NULL) {
            data->gpio_fd = gpiod_line_request_get_fd(data->request);
        }
        else {
            MYGPIOD_LOG_ERROR("Unable to request line %u", gpio);
            rc = false;
        }
    }
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    return rc;
}
