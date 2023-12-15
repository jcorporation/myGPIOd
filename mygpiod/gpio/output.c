/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/gpio/output.h"

#include "mygpiod/lib/config.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/util.h"

#include <assert.h>
#include <errno.h>
#include <gpiod.h>
#include <string.h>

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
        if (gpio_set_output(config->chip, current->id, data) == false) {
            return false;
        }
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
    if (gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT) == -1) {
        MYGPIOD_LOG_WARN("Unable to set direction");
    }
    if (gpiod_line_settings_set_bias(settings, data->bias) == -1) {
        MYGPIOD_LOG_WARN("Unable to set bias");
    }
    if (gpiod_line_settings_set_drive(settings, data->drive) == -1) {
        MYGPIOD_LOG_WARN("Unable to set drive");
    }
    gpiod_line_settings_set_active_low(settings, data->active_low);

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
        if (data->request == NULL) {
            MYGPIOD_LOG_ERROR("Unable to request line %u", gpio);
            rc = false;
        }
    }
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    return rc;
}

/**
 * Sets the current line value of an output gpio
 * @param config pointer to config
 * @param gpio gpio to set the value
 * @param value value to set
 * @return true on success, else false
 */
bool gpio_set_value(struct t_config *config, unsigned gpio, enum gpiod_line_value value) {
    struct t_list_node *node = list_node_by_id(&config->gpios_out, gpio);
    if (node == NULL) {
        MYGPIOD_LOG_ERROR("GPIO %u is not configured as output", gpio);
        return false;
    }
    struct t_gpio_out_data *data = (struct t_gpio_out_data *)node->data;
    return gpiod_line_request_set_value(data->request, gpio, value);
}

/**
 * Toggles the current line value of an output gpio
 * @param config pointer to config
 * @param gpio gpio to set the value
 * @return true on success, else false
 */
bool gpio_toggle_value(struct t_config *config, unsigned gpio) {
    struct t_list_node *node = list_node_by_id(&config->gpios_out, gpio);
    if (node == NULL) {
        MYGPIOD_LOG_ERROR("GPIO %u is not configured as output", gpio);
        return false;
    }
    struct t_gpio_out_data *data = (struct t_gpio_out_data *)node->data;
    // get and toggle the value
    enum gpiod_line_value value = gpiod_line_request_get_value(data->request, gpio) == GPIOD_LINE_VALUE_INACTIVE
        ? GPIOD_LINE_VALUE_ACTIVE
        : GPIOD_LINE_VALUE_INACTIVE;
    return gpiod_line_request_set_value(data->request, gpio, value);
}
