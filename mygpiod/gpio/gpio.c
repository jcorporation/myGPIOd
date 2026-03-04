/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief General GPIO functions
 */

#include "compile_time.h"
#include "mygpiod/gpio/gpio.h"

#include "mygpiod/config/gpio.h"
#include "mygpiod/gpio/chip.h"
#include "mygpiod/gpio/input.h"
#include "mygpiod/gpio/output.h"
#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"

/**
 * Initializes the GPIO chip and lines
 * @param config Pointer to config
 * @param poll_fds Pointer to poll_fds array
 * @return true on success, else false
 */
bool gpio_init(struct t_config *config, struct t_poll_fds *poll_fds) {
    if (sdslen(config->chip_path) > 0) {
        if (gpio_open_chip(config) == false ||
            gpio_set_outputs(config) == false ||
            gpio_request_inputs(config, poll_fds) == false)
        {
            return false;
        }
    }
    else {
        MYGPIOD_LOG_INFO("No GPIO chip configured");
        gpios_config_clear(&config->gpios_in, &config->gpios_out);
    }
    return true;
}

/**
 * Gets the current line value of a gpio
 * @param config pointer to config
 * @param gpio gpio to get the value
 * @return the active state
 */
enum gpiod_line_value gpio_get_value(struct t_config *config, unsigned gpio) {
    // is it an input gpio?
    struct t_list_node *node = list_node_by_id(&config->gpios_in, gpio);
    if (node != NULL) {
        struct t_gpio_in_data *data = (struct t_gpio_in_data *)node->data;
        return gpiod_line_request_get_value(data->request, gpio);
    }
    // is it an output gpio?
    node = list_node_by_id(&config->gpios_out, gpio);
    if (node != NULL) {
        struct t_gpio_out_data *data = (struct t_gpio_out_data *)node->data;
        return gpiod_line_request_get_value(data->request, gpio);
    }
    // not found in configuration
    MYGPIOD_LOG_ERROR("GPIO %u is not configured", gpio);
    return GPIOD_LINE_VALUE_ERROR;
}
