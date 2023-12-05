/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server/gpio.h"

#include "mygpiod/gpio/input.h"
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
    server_response_append(client_data, "%s", DEFAULT_OK_MSG_PREFIX);
    struct t_list_node *current = config->gpios_in.head;
    while (current != NULL) {
        server_response_append(client_data, "gpio:%u\n", current->id);
        server_response_append(client_data, "mode:in\n");
        current = current->next;
    }
    current = config->gpios_out.head;
    while (current != NULL) {
        server_response_append(client_data, "gpio:%u\n", current->id);
        server_response_append(client_data, "mode:out\n");
        current = current->next;
    }
    server_response_append(client_data, "%s", DEFAULT_END_MSG);
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
        server_response_send(client_data, DEFAULT_ERROR_MSG_PREFIX "Invalid number of arguments\n" DEFAULT_END_MSG);
        return false;
    }
    unsigned gpio;
    if (parse_uint(options->args[1], &gpio, NULL, 0, GPIOS_MAX) == false) {
        server_response_send(client_data, DEFAULT_ERROR_MSG_PREFIX "Invalid gpio number\n" DEFAULT_END_MSG);
        return false;
    }
    enum gpiod_line_value value = gpio_get_value(config, gpio);
    if (value == GPIOD_LINE_VALUE_ERROR) {
        server_response_send(client_data, DEFAULT_ERROR_MSG_PREFIX "Getting gpio value failed\n" DEFAULT_END_MSG);
        return false;
    }
    server_response_start(client_data);
    server_response_append(client_data, "%s", DEFAULT_OK_MSG_PREFIX);
    server_response_append(client_data, "value:%d\n", value);
    server_response_append(client_data, "%s", DEFAULT_END_MSG);
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
        server_response_send(client_data, DEFAULT_ERROR_MSG_PREFIX "Invalid number of arguments\n" DEFAULT_END_MSG);
        return false;
    }
    unsigned gpio;
    if (parse_uint(options->args[1], &gpio, NULL, 0, GPIOS_MAX) == false) {
        server_response_send(client_data, DEFAULT_ERROR_MSG_PREFIX "Invalid gpio number\n" DEFAULT_END_MSG);
        return false;
    }
    errno = 0;
    enum gpiod_line_value value = parse_gpio_value(options->args[2]);
    if (errno == EINVAL) {
        server_response_send(client_data, DEFAULT_ERROR_MSG_PREFIX "Invalid value\n" DEFAULT_END_MSG);
        return false;
    }
    if (gpio_set_value(config, gpio, value) == true) {
        server_response_send(client_data, DEFAULT_OK_MSG_PREFIX DEFAULT_END_MSG);
        return true;
    }
    server_response_send(client_data, DEFAULT_ERROR_MSG_PREFIX "Setting gpio value failed\n" DEFAULT_END_MSG);
    return false;
}
