/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server/gpio.h"

#include "mygpio-common/util.h"
#include "mygpiod/gpio/gpio.h"
#include "mygpiod/gpio/output.h"
#include "mygpiod/lib/util.h"
#include "mygpiod/server/response.h"
#include "mygpiod/server/socket.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

/**
 * Handles the gpiolist command
 * @param config pointer to config
 * @param client_node client
 * @return true on success, else false
 */
bool handle_gpiolist(struct t_config *config, struct t_list_node *client_node) {
    struct t_client_data *client_data = (struct t_client_data *)client_node->data;
    server_response_start(client_data);
    server_response_append(client_data, "%s", DEFAULT_MSG_OK);
    struct t_list_node *current = config->gpios_in.head;
    while (current != NULL) {
        server_response_append(client_data, "gpio:%u", current->id);
        server_response_append(client_data, "direction:in");
        server_response_append(client_data, "value:%s", lookup_gpio_value(gpio_get_value(config, current->id)));
        current = current->next;
    }
    current = config->gpios_out.head;
    while (current != NULL) {
        server_response_append(client_data, "gpio:%u", current->id);
        server_response_append(client_data, "direction:out");
        server_response_append(client_data, "value:%s", lookup_gpio_value(gpio_get_value(config, current->id)));
        current = current->next;
    }
    server_response_append(client_data, "%s", DEFAULT_MSG_END);
    server_response_end(client_data);
    return true;
}

/**
 * Handles the gpioinfo command
 * @param options client command
 * @param config pointer to config
 * @param client_node client
 * @return true on success, else false
 */
bool handle_gpioinfo(struct t_cmd_options *options, struct t_config *config, struct t_list_node *client_node) {
    struct t_client_data *client_data = (struct t_client_data *)client_node->data;
    if (options->len != 2) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid number of arguments");
        return false;
    }
    unsigned gpio;
    if (mygpio_parse_uint(options->args[1], &gpio, NULL, 0, GPIOS_MAX) == false) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid GPIO number");
        return false;
    }

    enum gpiod_line_direction gpio_direction = GPIOD_LINE_DIRECTION_INPUT;
    struct t_list_node *node = list_node_by_id(&config->gpios_in, gpio);
    if (node == NULL) {
        node = list_node_by_id(&config->gpios_out, gpio);
        gpio_direction = GPIOD_LINE_DIRECTION_OUTPUT;
    }
    if (node == NULL) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "GPIO not configured");
        return false;
    }

    server_response_start(client_data);
    server_response_append(client_data, "%s", DEFAULT_MSG_OK);
    server_response_append(client_data, "gpio:%u", gpio);
    if (gpio_direction == GPIOD_LINE_DIRECTION_INPUT) {
        struct gpiod_line_info *info = gpiod_chip_get_line_info(config->chip, node->id);
        if (info != NULL) {
            server_response_append(client_data, "direction:in");
            server_response_append(client_data, "value:%s", lookup_gpio_value(gpio_get_value(config, gpio)));
            server_response_append(client_data, "active_low:%s", mygpio_bool_to_str(gpiod_line_info_is_active_low(info)));
            server_response_append(client_data, "bias:%s", lookup_bias(gpiod_line_info_get_bias(info)));
            server_response_append(client_data, "event_request:%s", lookup_event_request(gpiod_line_info_get_edge_detection(info)));
            server_response_append(client_data, "is_debounced:%s", mygpio_bool_to_str(gpiod_line_info_is_debounced(info)));
            server_response_append(client_data, "debounce_period_us:%lu", gpiod_line_info_get_debounce_period_us(info));
            server_response_append(client_data, "event_clock:%s", lookup_event_clock(gpiod_line_info_get_event_clock(info)));
            gpiod_line_info_free(info);
        }
    }
    else if (gpio_direction == GPIOD_LINE_DIRECTION_OUTPUT) {
        struct gpiod_line_info *info = gpiod_chip_get_line_info(config->chip, node->id);
        if (info != NULL) {
            server_response_append(client_data, "direction:out");
            server_response_append(client_data, "value:%s", lookup_gpio_value(gpio_get_value(config, gpio)));
            server_response_append(client_data, "drive:%s", lookup_drive(gpiod_line_info_get_drive(info)));
            gpiod_line_info_free(info);
        }
    }
    server_response_append(client_data, "%s", DEFAULT_MSG_END);
    server_response_end(client_data);
    return true;
}

/**
 * Handles the gpioget command
 * @param options client command
 * @param config pointer to config
 * @param client_node client
 * @return true on success, else false
 */
bool handle_gpioget(struct t_cmd_options *options, struct t_config *config, struct t_list_node *client_node) {
    struct t_client_data *client_data = (struct t_client_data *)client_node->data;
    if (options->len != 2) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid number of arguments");
        return false;
    }
    unsigned gpio;
    if (mygpio_parse_uint(options->args[1], &gpio, NULL, 0, GPIOS_MAX) == false) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid GPIO number");
        return false;
    }
    enum gpiod_line_value value = gpio_get_value(config, gpio);
    if (value == GPIOD_LINE_VALUE_ERROR) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Getting GPIO value failed");
        return false;
    }
    server_response_start(client_data);
    server_response_append(client_data, "%s", DEFAULT_MSG_OK);
    server_response_append(client_data, "value:%s", lookup_gpio_value(value));
    server_response_append(client_data, "%s", DEFAULT_MSG_END);
    server_response_end(client_data);
    return true;
}

/**
 * Handles the gpioset command
 * @param options client command
 * @param config pointer to config
 * @param client_node client
 * @return true on success, else false
 */
bool handle_gpioset(struct t_cmd_options *options, struct t_config *config, struct t_list_node *client_node) {
    struct t_client_data *client_data = (struct t_client_data *)client_node->data;
    if (options->len != 3) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid number of arguments");
        return false;
    }
    unsigned gpio;
    if (mygpio_parse_uint(options->args[1], &gpio, NULL, 0, GPIOS_MAX) == false) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid GPIO number");
        return false;
    }
    errno = 0;
    enum gpiod_line_value value = parse_gpio_value(options->args[2]);
    if (errno == EINVAL) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid value");
        return false;
    }
    if (gpio_set_value(config, gpio, value) == true) {
        server_response_send(client_data, DEFAULT_MSG_OK "\n" DEFAULT_MSG_END);
        return true;
    }
    server_response_send(client_data, DEFAULT_MSG_ERROR "Setting GPIO value failed");
    return false;
}

/**
 * Handles the gpiotoggle command
 * @param options client command
 * @param config pointer to config
 * @param client_node client
 * @return true on success, else false
 */
bool handle_gpiotoggle(struct t_cmd_options *options, struct t_config *config, struct t_list_node *client_node) {
    struct t_client_data *client_data = (struct t_client_data *)client_node->data;
    if (options->len != 2) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid number of arguments");
        return false;
    }
    unsigned gpio;
    if (mygpio_parse_uint(options->args[1], &gpio, NULL, 0, GPIOS_MAX) == false) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid GPIO number");
        return false;
    }
    if (gpio_toggle_value(config, gpio) == true) {
        server_response_send(client_data, DEFAULT_MSG_OK "\n" DEFAULT_MSG_END);
        return true;
    }
    server_response_send(client_data, DEFAULT_MSG_ERROR "Setting GPIO value failed");
    return false;
}

/**
 * Handles the gpioblink command
 * @param options client command
 * @param config pointer to config
 * @param client_node client
 * @return true on success, else false
 */
bool handle_gpioblink(struct t_cmd_options *options, struct t_config *config, struct t_list_node *client_node) {
    struct t_client_data *client_data = (struct t_client_data *)client_node->data;
    if (options->len != 4) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid number of arguments");
        return false;
    }
    unsigned gpio;
    if (mygpio_parse_uint(options->args[1], &gpio, NULL, 0, GPIOS_MAX) == false) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid GPIO number");
        return false;
    }
    int timeout;
    if (mygpio_parse_int(options->args[2], &timeout, NULL, 0, TIMEOUT_MS_MAX) == false) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid timeout");
        return false;
    }
    int interval;
    if (mygpio_parse_int(options->args[3], &interval, NULL, 0, TIMEOUT_MS_MAX) == false) {
        server_response_send(client_data, DEFAULT_MSG_ERROR "Invalid interval");
        return false;
    }
    if (gpio_blink(config, gpio, timeout, interval) == true) {
        server_response_send(client_data, DEFAULT_MSG_OK "\n" DEFAULT_MSG_END);
        return true;
    }
    server_response_send(client_data, DEFAULT_MSG_ERROR "Setting GPIO to blinking");
    return false;
}
