/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_SERVER_HTTPD_UTIL_H
#define MYGPIOD_SERVER_HTTPD_UTIL_H

#include "dist/sds/sds.h"
#include "mygpiod/lib/event_types.h"

#include <inttypes.h>
#include <microhttpd.h>

/**
 * Used HTTP methods
 */
enum http_method {
    HTTP_UNKNOWN = -1,
    HTTP_GET,
    HTTP_OPTIONS,
    HTTP_PATCH,
};

/**
 * MHD connection specific data
 */
struct t_request_data {
    sds resume_buffer;                  //!< Message buffer for resumed connections
    struct MHD_Connection *connection;  //!< Pointer to MHD connection
    unsigned conn_id;              //!< Uniq connection id
};

void http_response_free(void *cls);
enum MHD_Result http_respond(struct MHD_Connection *connection,
                             unsigned int status_code,
                             const char *content_type,
                             const char *message);
enum http_method http_parse_method(const char *method);
const char *http_lookup_method(enum http_method method);

void http_connection_resume(struct t_request_data *request_data,
                            unsigned gpio,
                            enum mygpiod_event_types event_type,
                            uint64_t timestamp);
 void http_connection_done(void *cls,
                           struct MHD_Connection *connection,
                           void **req_cls,
                           enum MHD_RequestTerminationCode toe);

#endif
