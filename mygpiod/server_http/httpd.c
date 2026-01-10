/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server_http/httpd.h"

#include "lib/mem.h"
#include "lib/sds_extras.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/server_http/rest_api.h"
#include "mygpiod/server_http/util.h"
#include "mygpiod/server_http/webui.h"

#include <arpa/inet.h>
#include <microhttpd.h>
#include <netinet/in.h>
#include <string.h>

/**
 * Central HTTP Request Handler
 * @param cls User data
 * @param connection HTTP connection
 * @param url URL
 * @param method_str HTTP method
 * @param version HTTP version
 * @param upload_data POST data
 * @param upload_data_size POST data size
 * @param con_cls Connection specific user data
 * @return enum MHD_Result
 */
static enum MHD_Result request_handler(void *cls,
                                       struct MHD_Connection *connection,
                                       const char *url,
                                       const char *method_str,
                                       const char *version,
                                       const char *upload_data,
                                       size_t *upload_data_size,
                                       void **con_cls)
{
    (void)version;
    (void)upload_data;
    struct t_config *config = (struct t_config *)cls;

    // The first call is with headers only, do not respond,
    // but allocate connection specific user data
    if (*con_cls == NULL) {
        config->http_conn_id++;
        struct t_request_data *request_data = malloc_assert(sizeof(struct t_request_data));
        request_data->connection = connection;
        request_data->resume_buffer = NULL;
        request_data->conn_id = config->http_conn_id;
        *con_cls = request_data;
        MYGPIOD_LOG_DEBUG("HTTP connection %u: Headers received for %s %s", request_data->conn_id, method_str, url);
        return MHD_YES;
    }
    struct t_request_data *request_data = (struct t_request_data *)*con_cls;

    // Ignoring post data
    if (*upload_data_size) {
        MYGPIOD_LOG_DEBUG("HTTP connection %u: Ignoring POST data for %s %s (%lu bytes)",
                request_data->conn_id, method_str, url, (unsigned long)upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    }

    // Resumed connection
    if (request_data->resume_buffer != NULL) {
        MYGPIOD_LOG_DEBUG("HTTP connection %u: Resuming connection for %s %s", request_data->conn_id, method_str, url);
        struct MHD_Response *response = MHD_create_response_from_buffer(sdslen(request_data->resume_buffer),
                (void *)request_data->resume_buffer, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "application/json");
        enum MHD_Result result = MHD_queue_response(connection, 200, response);
        MHD_destroy_response(response);
        return result;
    }

    // Second call - process the request
    enum http_method method = http_parse_method(method_str);
    // Restrict allowed HTTP methods
    switch(method) {
        case HTTP_GET:
        case HTTP_OPTIONS:
        case HTTP_PATCH:
            break;
        default: {
            return http_respond(connection, 405, "text/plain; charset=utf-8", "405 Method Not Allowed");
        }
    }

    MYGPIOD_LOG_DEBUG("HTTP connection %u: %s %s", request_data->conn_id, method_str, url);
    // REST-API
    if (strncmp(url, "/api/", 5) == 0) {
        MYGPIOD_LOG_DEBUG("HTTP connection %u: Calling REST-API handler for %s %s", request_data->conn_id, method_str, url);
        return rest_api_handler(connection, request_data->conn_id, url, method, config);
    }
    // Long polling: Suspend connection until an GPIO event occurs
    if (strcmp(url, "/poll") == 0) {
        MYGPIOD_LOG_DEBUG("HTTP connection %u: Suspending connection for %s %s", request_data->conn_id, method_str, url);
        MHD_set_connection_option(connection, MHD_CONNECTION_OPTION_TIMEOUT, 0);
        MHD_suspend_connection(connection);
        list_push(&config->http_suspended, 0, request_data);
        return MHD_YES;
    }
    // Serve embedded files
    MYGPIOD_LOG_DEBUG("HTTP connection %u: Serve embedded files for %s %s", request_data->conn_id, method_str, url);
    return webui_handler(connection, url);
}

/**
 * Errorlog handler
 * @param arg Not used
 * @param fmt Format string
 * @param ap Variadic values
 */
static void error_log(void *arg, const char *fmt, va_list ap) {
    (void)arg;
    sds error = sdscatvprintf(sdsempty(), fmt, ap);
    MYGPIOD_LOG_ERROR("HTTP: %s", error);
    FREE_SDS(error);
}

/**
 * Creates the microhttpd server
 * @param config Pointer to config
 * @return struct MHD_Daemon* 
 */
struct MHD_Daemon *httpd_start(struct t_config *config) {
    MYGPIOD_LOG_INFO("HTTP: Listening on %s:%u", config->http_ip, config->http_port);
    unsigned mhd_flags = MHD_USE_EPOLL | \
                         MHD_USE_ERROR_LOG | \
                         MHD_USE_PEDANTIC_CHECKS | \
                         MHD_USE_TCP_FASTOPEN | \
                         MHD_ALLOW_SUSPEND_RESUME;
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)config->http_port);  // Convert port number to network byte order
    inet_pton(AF_INET, config->http_ip, &server_addr.sin_addr); // Specify IP address

    return MHD_start_daemon(mhd_flags,
                            (uint16_t)config->http_port,
                            NULL,
                            NULL,
                            &request_handler,
                            config,
                            MHD_OPTION_EXTERNAL_LOGGER, &error_log, NULL,
                            MHD_OPTION_SOCK_ADDR, &server_addr,
                            MHD_OPTION_NOTIFY_COMPLETED, &http_connection_done, NULL,
                            MHD_OPTION_END);
}
