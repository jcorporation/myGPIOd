/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server_http/webui.h"

#include "embedded_files.c"
#include "server_http/util.h"

#include <microhttpd.h>
#include <string.h>

/**
 * Creates a MHD response object for an embedded file
 * @param url URL
 * @return struct MHD_Response* 
 */
static struct MHD_Response *serve_embedded_file(const char *url) {
    struct MHD_Response *response = NULL;
    if (strcmp(url, "/bootstrap-native.min.js") == 0) {
        response = MHD_create_response_from_buffer(bootstrap_native_min_js_size, (void *)bootstrap_native_min_js_data, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "application/javascript");
    }
    else if (strcmp(url, "/bootstrap.min.css") == 0) {
        response = MHD_create_response_from_buffer(bootstrap_min_css_size, (void *)bootstrap_min_css_data, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "text/css");
    }
    else if (strcmp(url, "/favicon.svg") == 0) {
        response = MHD_create_response_from_buffer(favicon_svg_size, (void *)favicon_svg_data, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "image/svg+xml");
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
    else if (strcmp(url, "/openapi.yml") == 0) {
        response = MHD_create_response_from_buffer(openapi_yml_size, (void *)openapi_yml_data, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "text/yaml");
    }
    return response;
}

/**
 * Request handler for the WebUI
 * @param connection HTTP connection
 * @param url URL
 * @return enum MHD_Result 
 */
enum MHD_Result webui_handler(struct MHD_Connection *connection, const char *url) {
    unsigned http_response_code = MHD_HTTP_OK;
    struct MHD_Response *response = serve_embedded_file(url);
    if (response != NULL) {
        enum MHD_Result result = MHD_queue_response(connection, http_response_code, response);
        MHD_destroy_response(response);
        return result;
    }
    return http_respond(connection, MHD_HTTP_NOT_FOUND, "text/plain; charset=utf-8", "File not found.");
}
