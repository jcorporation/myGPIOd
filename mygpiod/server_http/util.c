/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server_http/util.h"

#include "dist/sds/sds.h"
#include "lib/events.h"
#include "lib/log.h"

#include <stdlib.h>
#include <string.h>

/**
 * Callback of type #include "dist/sds/sds.h"
 * @param cls SDS buffer to free
 */
void http_response_free(void *cls) {
    sdsfree((sds)cls);
}

/**
 * Simple HTTP response
 * @param connection MHD connection
 * @param status_code HTTP status code
 * @param content_type HTTP Content-type
 * @param message HTTP body
 * @return enum MHD_Result
 */
enum MHD_Result http_respond(struct MHD_Connection *connection,
                             unsigned int status_code,
                             const char *content_type,
                             const char *message)
{
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(message), (void *)message, MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Content-Type", content_type);
    enum MHD_Result result = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    return result;
}

/**
 * Parses a string to a HTTP method
 * @param method HTTP method string to parse
 * @return enum http_method 
 */
enum http_method http_parse_method(const char *method) {
    if (strcmp(method, "GET") == 0) {
        return HTTP_GET;
    }
    if (strcmp(method, "OPTIONS") == 0) {
        return HTTP_OPTIONS;
    }
    if (strcmp(method, "PATCH") == 0) {
        return HTTP_PATCH;
    }
    return HTTP_UNKNOWN;
}

/**
 * Returns the string representation of a HTTP method
 * @param method HTTP Method
 * @return const char* or NULL on error
 */
const char *http_lookup_method(enum http_method method) {
    switch(method) {
        case HTTP_GET:     return "GET";
        case HTTP_OPTIONS: return "OPTIONS";
        case HTTP_PATCH:   return "PATCH";
        case HTTP_UNKNOWN: return NULL;
    }
    return NULL;
}

/**
 * Crates the response message and resumes a suspended connection for the long poll endpoint
 * @param connection User data from a MHD connection
 * @param gpio GPIO number
 * @param event_type GPIO event
 * @param timestamp Event timestamp
 */
void http_connection_resume(struct t_request_data *request_data,
                            unsigned gpio,
                            enum mygpiod_event_types event_type,
                            uint64_t timestamp)
{
    const union MHD_ConnectionInfo *conn_info = MHD_get_connection_info(request_data->connection, MHD_CONNECTION_INFO_CONNECTION_SUSPENDED);
    if (conn_info == MHD_NO) {
        MYGPIOD_LOG_ERROR("Connection is not suspended");
        return;
    }
    request_data->resume_buffer = sdscatprintf(sdsempty(), "{\"gpio\":%u,\"event\":\"%s\",\"timestamp_ms\":%llu}",
        gpio,
        mygpiod_event_name(event_type),
        (long long unsigned)(timestamp / 1000000)
    );
    MHD_resume_connection(request_data->connection);
}

/**
 * Callback for connection close
 * @param cls User data
 * @param connection Connection
 * @param req_cls Connection specific user data
 * @param toe Request termination code
 */
void http_connection_done(void *cls,
                          struct MHD_Connection *connection,
                          void **req_cls,
                          enum MHD_RequestTerminationCode toe)
{
    (void)cls;
    (void)connection;
    (void)toe;
    struct t_request_data *request_data = *req_cls;
    sdsfree(request_data->resume_buffer);
    free(request_data);
}
