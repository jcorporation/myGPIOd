/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server_http/httpd.h"

#include "mygpiod/lib/log.h"
#include "mygpiod/server_http/rest_api.h"
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
 * @param method HTTP method
 * @param version HTTP version
 * @param upload_data POST data
 * @param upload_data_size POST data size
 * @param ptr Pointer to config
 * @return enum MHD_Result 
 */
static enum MHD_Result request_handler(void *cls,
                                       struct MHD_Connection *connection,
                                       const char *url,
                                       const char *method,
                                       const char *version,
                                       const char *upload_data,
                                       size_t *upload_data_size,
                                       void **ptr)
{
    (void)cls;
    (void)version;
    (void)upload_data;
    (void)upload_data_size;
    struct t_config *config =  (struct t_config *)ptr;

    MYGPIOD_LOG_DEBUG("HTTP request: %s %s", method, url);
    enum MHD_Result rc = MHD_NO;
    if (strncmp(url, "/api/", 5) == 0) {
        rc = rest_api_handler(connection, url, method, config);
    }
    else {
        rc = webui_handler(connection, url);
    }
    return rc;
}

void error_log(void *arg, const char *fmt, va_list ap) {
    (void)arg;
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
    MYGPIOD_LOG_ERROR(fmt, ap);
    #pragma GCC diagnostic pop
}

/**
 * Creates the microhttpd server
 * @param config Pointer to config
 * @return struct MHD_Daemon* 
 */
struct MHD_Daemon *httpd_start(struct t_config *config) {
    MYGPIOD_LOG_INFO("Listening on port %u for http requests.", config->http_port);
    unsigned mhd_flags = MHD_USE_EPOLL | \
                         MHD_USE_ERROR_LOG | \
                         MHD_USE_PEDANTIC_CHECKS | \
                         MHD_USE_TCP_FASTOPEN | \
                         MHD_ALLOW_UPGRADE;
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)config->http_port);            // Convert port number to network byte order
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
