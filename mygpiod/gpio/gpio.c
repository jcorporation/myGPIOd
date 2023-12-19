/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/gpio/gpio.h"

#include "mygpiod/lib/list.h"
#include "mygpiod/lib/log.h"

/**
 * Gets the current line value of an input gpio
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
