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

#include <microhttpd.h>
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
    (void)upload_data_size;
    struct t_config *config =  (struct t_config *)ptr;

    MYGPIOD_LOG_DEBUG("HTTP request: %s %s", method, url);
    enum MHD_Result rc = MHD_NO;
    if (strncmp(url, "/api/", 5) == 0) {
        rc = rest_api_handler(connection, url, method, upload_data, config);
    }
    else {
        rc = webui_handler(connection, url);
    }
    return rc;
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
                         MHD_USE_DUAL_STACK | \
                         MHD_USE_PEDANTIC_CHECKS | \
                         MHD_USE_TCP_FASTOPEN | \
                         MHD_ALLOW_UPGRADE;
    return MHD_start_daemon(mhd_flags,
                            (uint16_t)config->http_port,
                            NULL,
                            NULL,
                            &request_handler,
                            config,
                            MHD_OPTION_END);
}
