/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief HTTP server hooks
 */

#include "compile_time.h"
#include "mygpiod/server_http/hook.h"

#include "dist/sds/sds.h"
#include "mygpiod/hook/action.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/server_http/util.h"

#include <microhttpd.h>

/**
 * Handler for REST API Requests
 * @param connection HTTP connection
 * @param http_conn_id HTTP connection id
 * @param url URL
 * @param config Pointer to config
 * @return enum MHD_Result 
 */
enum MHD_Result hook_handler(struct MHD_Connection *connection,
                                 unsigned http_conn_id,
                                 const char *url,
                                 struct t_config *config)
{
    sds buffer = sdsempty();
    bool rc = false;
    // Ignore /hook prefix from url
    const char *hook_name = url + 6;

    if (hook_action_handler(config, hook_name) == true) {
        buffer = sdscat(buffer,"{\"message\":\"Triggered\"}");
    }
    else {
        // Request was not handled
        MYGPIOD_LOG_ERROR("HTTP connection %u: Invalid hook: %s", http_conn_id, url);
        rc = false;
        buffer = sdscat(buffer,"{\"error\":\"Invalid hook\"}");
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
