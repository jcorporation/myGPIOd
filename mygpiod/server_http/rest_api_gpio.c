/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server_http/rest_api_gpio.h"

#include "mygpio-common/util.h"
#include "mygpiod/gpio/gpio.h"
#include "mygpiod/gpio/output.h"
#include "mygpiod/gpio/util.h"
#include "mygpiod/lib/json_print.h"

#include <errno.h>
#include <stdlib.h>

/**
 * Handles the REST API request for GET /api/gpio
 * @param config pointer to config
 * @param buffer already allocated buffer to populate with the response
 * @param rc pointer to bool to set the result code
 * @return sds pointer to buffer
 */
sds rest_api_gpio_get(struct t_config *config,
                      sds buffer,
                      bool *rc)
{
    buffer = sdscat(buffer, "{\"data\":[");
    unsigned i = 0;
    struct t_list_node *current = config->gpios_in.head;
    while (current != NULL) {
        if (i++) {
            buffer = sdscatlen(buffer, ",", 1);
        }
        buffer = sdscatlen(buffer, "{", 1);
        buffer = sdscatfmt(buffer, "\"gpio\":%u,", current->id);
        buffer = sdscat(buffer, "\"direction\":\"in\",");
        buffer = sdscat(buffer, "\"value\":");
        buffer = sds_catjson(buffer, lookup_gpio_value(gpio_get_value(config, current->id)));
        buffer = sdscatlen(buffer, "}", 1);
        current = current->next;
    }
    current = config->gpios_out.head;
    while (current != NULL) {
        if (i++) {
            buffer = sdscatlen(buffer, ",", 1);
        }
        buffer = sdscatlen(buffer, "{", 1);
        buffer = sdscatfmt(buffer, "\"gpio\":%u,", current->id);
        buffer = sdscat(buffer, "\"direction\":\"out\",");
        buffer = sdscat(buffer, "\"value\":");
        buffer = sds_catjson(buffer, lookup_gpio_value(gpio_get_value(config, current->id)));
        buffer = sdscatlen(buffer, "}", 1);
        current = current->next;
    }
    buffer = sdscatfmt(buffer, "],\"entries\":%u}", i);
    *rc = true;
    return buffer;
}

/**
 * Handles the REST API request for GET /api/gpio/{gpio_nr}
 * @param config pointer to config
 * @param buffer already allocated buffer to populate with the response
 * @param gpio_nr gpio number
 * @param rc pointer to bool to set the result code
 * @return sds pointer to buffer
 */
sds rest_api_gpio_gpio_get(struct t_config *config,
                           sds buffer,
                           unsigned gpio_nr,
                           bool *rc)
{
    enum gpiod_line_value value = gpio_get_value(config, gpio_nr);
    if (value == GPIOD_LINE_VALUE_ERROR) {
        *rc = false;
        return sdscat(buffer,"{\"error\":\"Getting GPIO value failed\"}");
    }

    buffer = sdscatlen(buffer, "{", 1);
    buffer = sdscatfmt(buffer, "\"gpio\": %u,", gpio_nr);
    buffer = sdscatfmt(buffer, "\"value\":\"%s\"", lookup_gpio_value(value));
    buffer = sdscatlen(buffer, "}", 1);
    *rc = true;
    return buffer;
}

/**
 * Handles the REST API request for OPTIONS /api/gpio/{gpio_nr}
 * @param config pointer to config
 * @param buffer already allocated buffer to populate with the response
 * @param gpio_nr gpio number
 * @param rc pointer to bool to set the result code
 * @return sds pointer to buffer
 */
sds rest_api_gpio_gpio_options(struct t_config *config,
                               sds buffer,
                               unsigned gpio_nr,
                               bool *rc)
{
    enum gpiod_line_direction gpio_direction = GPIOD_LINE_DIRECTION_INPUT;
    struct t_list_node *node = list_node_by_id(&config->gpios_in, gpio_nr);
    if (node == NULL) {
        node = list_node_by_id(&config->gpios_out, gpio_nr);
        gpio_direction = GPIOD_LINE_DIRECTION_OUTPUT;
    }
    if (node == NULL) {
        *rc = false;
        return  sdscat(buffer,"{\"error\":\"GPIO not configured\"}");
    }

    struct gpiod_line_info *info = gpiod_chip_get_line_info(config->chip, node->id);
    if (info == NULL) {
        *rc = false;
        return sdscat(buffer,"{\"error\":\"Failure geeting GPIO info\"}");
    }

    buffer = sdscat(buffer, "{\"data\":");
    buffer = sdscatlen(buffer, "{", 1);
    buffer = sdscatfmt(buffer, "\"gpio\":%u", gpio_nr);
    buffer = sdscatlen(buffer, ",", 1);
    buffer = sdscat(buffer, "\"value\":");
    buffer = sds_catjson(buffer, lookup_gpio_value(gpio_get_value(config, gpio_nr)));
    buffer = sdscatlen(buffer, ",", 1);
    if (gpio_direction == GPIOD_LINE_DIRECTION_INPUT) {
        buffer = sdscat(buffer, "\"direction\":\"in\",");
        buffer = sdscatfmt(buffer, "\"active_low\":%s", bool_to_str(gpiod_line_info_is_active_low(info)));
        buffer = sdscatlen(buffer, ",", 1);
        buffer = sdscat(buffer, "\"bias\":");
        buffer = sds_catjson(buffer, lookup_bias(gpiod_line_info_get_bias(info)));
        buffer = sdscatlen(buffer, ",", 1);
        buffer = sdscat(buffer, "\"event_request\":");
        buffer = sds_catjson(buffer, lookup_event_request(gpiod_line_info_get_edge_detection(info)));
        buffer = sdscatlen(buffer, ",", 1);
        buffer = sdscatfmt(buffer, "\"is_debounced\":%s", bool_to_str(gpiod_line_info_is_debounced(info)));
        buffer = sdscatlen(buffer, ",", 1);
        buffer = sdscatfmt(buffer, "\"debounce_period_us\":%I", (int64_t)gpiod_line_info_get_debounce_period_us(info));
        buffer = sdscatlen(buffer, ",", 1);
        buffer = sdscat(buffer, "\"event_clock\":");
        buffer = sds_catjson(buffer, lookup_event_clock(gpiod_line_info_get_event_clock(info)));
    }
    else if (gpio_direction == GPIOD_LINE_DIRECTION_OUTPUT) {
        buffer = sdscat(buffer, "\"direction\":\"out\",");
        buffer = sdscat(buffer, "\"drive\":");
        buffer = sds_catjson(buffer, lookup_drive(gpiod_line_info_get_drive(info)));
    }
    buffer = sdscatlen(buffer, "}}", 2);
    *rc = true;
    return buffer;
}

/**
 * Handles the REST API request for PATCH /api/gpio/{gpio_nr}/blink
 * @param config pointer to config
 * @param buffer already allocated buffer to populate with the response
 * @param gpio_nr gpio number
 * @param rc pointer to bool to set the result code
 * @return sds pointer to buffer
 */
sds rest_api_gpio_gpio_blink(struct t_config *config,
                             sds buffer,
                             unsigned gpio_nr,
                             struct MHD_Connection *connection,
                             bool *rc)
{

    const char *timeout = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "timeout");
    const char *interval = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "interval");
    int timeout_i;
    int interval_i;
    if (timeout == NULL ||
        interval == NULL ||
        mygpio_parse_int(timeout, &timeout_i, NULL, 0, INT_MAX) == false ||
        mygpio_parse_int(interval, &interval_i, NULL, 0, INT_MAX) == false)
    {
        *rc = false;
        return sdscat(buffer,"{\"error\":\"Parameter \"timeout\" or \"interval\" not found or invalid\"}");
    }

    *rc = gpio_blink(config, gpio_nr, timeout_i, interval_i);
    if (*rc == true) {
        buffer = sdscat(buffer, "{\"message\":\"OK\"}");
    }
    else {
        buffer = sdscat(buffer, "{\"error\":\"Blinking GPIO failed\"}");
    }
    return buffer;
}

/**
 * Handles the REST API request for PATCH /api/gpio/{gpio_nr}/set
 * @param config pointer to config
 * @param buffer already allocated buffer to populate with the response
 * @param gpio_nr gpio number
 * @param upload_data HTTP post data
 * @param rc pointer to bool to set the result code
 * @return sds pointer to buffer
 */
sds rest_api_gpio_gpio_set(struct t_config *config,
                            sds buffer,
                            unsigned gpio_nr,
                            struct MHD_Connection *connection,
                            bool *rc)
{
    const char *value = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value");
    if (value == NULL) {
        *rc = false;
        return sdscat(buffer,"{\"error\":\"Parameter \"value\" not found\"}");
    }

    errno = 0;
    enum gpiod_line_value line_value = parse_gpio_value(value);
    if (errno == EINVAL) {
        *rc = false;
        return sdscat(buffer,"{\"error\":\"Invalid value\"}");
    }

    *rc = gpio_set_value(config, gpio_nr, line_value);
    if (*rc == true) {
        buffer = sdscat(buffer, "{\"message\":\"OK\"}");
    }
    else {
        buffer = sdscat(buffer, "{\"error\":\"Setting GPIO failed\"}");
    }
    return buffer;
}

/**
 * Handles the REST API request for PATCH /api/gpio/{gpio_nr}/toggle
 * @param config pointer to config
 * @param buffer already allocated buffer to populate with the response
 * @param gpio_nr gpio number
 * @param action action for the gpio, one of blink, set, toggle
 * @param rc pointer to bool to set the result code
 * @return sds pointer to buffer
 */
sds rest_api_gpio_gpio_toggle(struct t_config *config,
                              sds buffer,
                              unsigned gpio_nr,
                              bool *rc)
{
    *rc = gpio_toggle_value(config, gpio_nr);
    if (*rc == true) {
        buffer = sdscat(buffer, "{\"message\":\"OK\"}");
    }
    else {
        buffer = sdscat(buffer, "{\"error\":\"Toggling GPIO failed\"}");
    }
    return buffer;
}
