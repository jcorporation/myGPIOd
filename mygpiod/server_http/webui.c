/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server_http/webui.h"

#include "embedded_files.c"

#include <microhttpd.h>
#include <string.h>

/**
 * Serves the files for the WebUI
 * @param connection HTTP connection
 * @param url URL
 * @return enum MHD_Result 
 */
enum MHD_Result webui_handler(struct MHD_Connection *connection, const char *url) {
    unsigned http_response_code = MHD_HTTP_OK;
    struct MHD_Response *response;
    if (strcmp(url, "/bootstrap-native.min.js") == 0) {
        response = MHD_create_response_from_buffer(bootstrap_native_min_js_size, (void *)bootstrap_native_min_js_data, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "application/javascript");
    }
    else if (strcmp(url, "/bootstrap.min.css") == 0) {
        response = MHD_create_response_from_buffer(bootstrap_min_css_size, (void *)bootstrap_min_css_data, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "text/css");
    }
    else if (strcmp(url, "/index.html") == 0 ||
             strcmp(url, "/") == 0)
    {
        response = MHD_create_response_from_buffer(index_html_size, (void *)index_html_data, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "text/html; charset=utf-8");
    }
    else if (strcmp(url, "/mygpiod.css") == 0) {
        response = MHD_create_response_from_buffer(mygpiod_css_size, (void *)mygpiod_css_data, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "text/css");
    }
    else if (strcmp(url, "/mygpiod.js") == 0) {
        response = MHD_create_response_from_buffer(mygpiod_js_size, (void *)mygpiod_js_data, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "application/javascript");
    }
    else {
        http_response_code = MHD_HTTP_NOT_FOUND;
        const char *error = "File not found.";
        response = MHD_create_response_from_buffer(strlen(error), (void *)error, MHD_RESPMEM_MUST_COPY);
        MHD_add_response_header(response, "Content-Type", "text/plain; charset=utf-8");
    }
    enum MHD_Result result = MHD_queue_response(connection, http_response_code, response);
    MHD_destroy_response(response);
    return result;
}
