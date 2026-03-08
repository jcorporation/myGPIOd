/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief HTTP server REST API
 */

#include "compile_time.h"
#include "mygpiod/server_http/rest_api.h"

#include "dist/sds/sds.h"
#include "mygpio-common/util.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/sds_extras.h"
#include "mygpiod/server_http/rest_api_gpio.h"
#include "mygpiod/server_http/rest_api_raspberry.h"
#include "mygpiod/server_http/rest_api_timerev.h"

#include <microhttpd.h>
#include <string.h>

/**
 * Parses REST API request url
 * @param url URL
 * @param pattern Pattern to match 
 * @param gpio Pointer to GPIO variable to populate
 * @return true on success, else false
 */
static bool match_url_gpio(const char *url,
                           const char *pattern,
                           unsigned *gpio)
{
    size_t pattern_len = strlen(pattern);
    // Match prefix
    size_t i = 0;
    for (; i < pattern_len; i++) {
        if (pattern[i] == '*') {
            break;
        }
        if (pattern[i] != url[i]) {
            return false;
        }
    }
    // Catch gpio number
    sds match = sdsempty();
    size_t j = i;
    for (; url[j] != '\0' && url[j] != '/'; j++) {
        match = sds_catchar(match, url[j]);
    }
    if (mygpio_parse_uint(match, gpio, NULL, 1, GPIOS_MAX) == false) {
        FREE_SDS(match);
        return false;
    }
    FREE_SDS(match);
    // Match suffix
    i++;
    for (; i < pattern_len; i++, j++) {
        if (pattern[i] != url[j]) {
            return false;
        }
    }
    return true;
}

/**
 * Handler for REST API Requests
 * @param connection HTTP connection
 * @param http_conn_id HTTP connection id
 * @param url URL
 * @param method HTTP method
 * @param config Pointer to config
 * @return enum MHD_Result 
 */
enum MHD_Result rest_api_handler(struct MHD_Connection *connection,
                                 unsigned http_conn_id,
                                 const char *url,
                                 enum http_method method,
                                 struct t_config *config)
{
    sds buffer = sdsempty();
    bool rc = false;
    unsigned gpio_nr = UINT_MAX;
    if (method == HTTP_GET && strcmp(url, "/api/v1/gpio") == 0) {
        buffer = rest_api_gpio_get(config, buffer, &rc);
    }
    else if (method == HTTP_GET && match_url_gpio(url, "/api/v1/gpio/*", &gpio_nr)) {
        buffer = rest_api_gpio_gpio_get(config, buffer, gpio_nr, &rc);
    }
    else if (method == HTTP_OPTIONS && match_url_gpio(url, "/api/v1/gpio/*", &gpio_nr)) {
        buffer = rest_api_gpio_gpio_options(config, buffer, gpio_nr, &rc);
    }
    else if (method == HTTP_PATCH && match_url_gpio(url, "/api/v1/gpio/*/blink", &gpio_nr)) {
        buffer = rest_api_gpio_gpio_blink(config, buffer, gpio_nr, connection, &rc);
    }
    else if (method == HTTP_PATCH && match_url_gpio(url, "/api/v1/gpio/*/set", &gpio_nr)) {
        buffer = rest_api_gpio_gpio_set(config, buffer, gpio_nr, connection, &rc);
    }
    else if (method == HTTP_PATCH && match_url_gpio(url, "/api/v1/gpio/*/toggle", &gpio_nr)) {
        buffer = rest_api_gpio_gpio_toggle(config, buffer, gpio_nr, &rc);
    }
    else if (method == HTTP_GET && strcmp(url, "/api/v1/vcio") == 0) {
        buffer = rest_api_raspberry_vcio_all(buffer, &rc);
    }
    else if (method == HTTP_GET && strcmp(url, "/api/v1/vcio/temp") == 0) {
        buffer = rest_api_raspberry_vcio(buffer, "measure_temp", &rc);
    }
    else if (method == HTTP_GET && strcmp(url, "/api/v1/vcio/volts") == 0) {
        buffer = rest_api_raspberry_vcio(buffer, "measure_volts core", &rc);
    }
    else if (method == HTTP_GET && strcmp(url, "/api/v1/vcio/clock") == 0) {
        buffer = rest_api_raspberry_vcio(buffer, "measure_clock arm", &rc);
    }
    else if (method == HTTP_GET && strcmp(url, "/api/v1/vcio/throttled") == 0) {
        buffer = rest_api_raspberry_vcio(buffer, "get_throttled", &rc);
    }
    else if (method == HTTP_GET && strcmp(url, "/api/v1/timerev") == 0) {
        buffer = rest_api_timerev_list(config, buffer, &rc);
    }
    else {
        // Request was not handled
        MYGPIOD_LOG_ERROR("HTTP connection %u: Invalid API request: %s %s", http_conn_id, http_lookup_method(method), url);
        rc = false;
        buffer = sdscat(buffer,"{\"error\":\"Invalid API request\"}");
    }

    unsigned http_response_code = rc == false
        ? MHD_HTTP_INTERNAL_SERVER_ERROR
        : MHD_HTTP_OK;
    struct MHD_Response *response = MHD_create_response_from_buffer_with_free_callback(sdslen(buffer), (void *)buffer, http_response_free);
    MHD_add_response_header(response, "Content-Type", "application/json");
    enum MHD_Result result = MHD_queue_response(connection, http_response_code, response);
    MHD_destroy_response(response);
    return result;
}
