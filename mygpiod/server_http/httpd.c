/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server_http/httpd.h"

#include <microhttpd.h>
#include <string.h>

static enum MHD_Result request_handler(void *cls,
                                       struct MHD_Connection *connection,
                                       const char *url,
                                       const char *method,
                                       const char *version,
                                       const char *upload_data,
                                       size_t *upload_data_size,
                                       void **ptr)
{
    static int aptr;
    struct MHD_Response *response;

    (void)cls;
    (void)url;
    (void)method;
    (void)version;
    (void)upload_data;
    (void)upload_data_size;
    
    struct t_config *config =  (struct t_config *)ptr;
    (void)config;

    if (&aptr != *ptr) {
        /* do never respond on first call */
        *ptr = &aptr;
        return MHD_YES;
    }
    *ptr = NULL; /* reset when done */

    response = MHD_create_response_from_buffer(strlen("test"),
                                               (void *)"test",
                                               MHD_RESPMEM_PERSISTENT);
    enum MHD_Result rc = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response (response);
    return rc;
}

struct MHD_Daemon *httpd_start(struct t_config *config) {
    return MHD_start_daemon(MHD_NO_FLAG,
                            (uint16_t)config->http_port,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            &request_handler,
                            config,
                            MHD_OPTION_END);
}
