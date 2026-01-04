/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server_http/httpd.h"

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
 * @param cls Not used
 * @param connection HTTP connection
 * @param url URL
 * @param method_str HTTP method
 * @param version HTTP version
 * @param upload_data POST data
 * @param upload_data_size POST data size
 * @param ptr Pointer to config
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
    (void)upload_data_size;
    static int dummy; // Used to detect second call

    // The first call is with headers only, do not respond
    if (&dummy != *con_cls) {
        *con_cls = &dummy;
        return MHD_YES;
    }

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

    // Second call - process the request
    MYGPIOD_LOG_DEBUG("HTTP: %s %s", method_str, url);
    struct t_config *config = (struct t_config *)cls;
    // REST API
    if (strncmp(url, "/api/", 5) == 0) {
        return rest_api_handler(connection, url, method, config);
    }
    // Serve embedded files
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
                         MHD_ALLOW_UPGRADE;
    
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
                            MHD_OPTION_END);
}
