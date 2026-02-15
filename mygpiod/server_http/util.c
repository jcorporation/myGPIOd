/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief HTTP server utility functions
 */

#include "compile_time.h"
#include "mygpiod/server_http/util.h"

#include "dist/sds/sds.h"
#include "mygpiod/input/event_code.h"
#include "mygpiod/input/event_type.h"
#include "mygpiod/lib/events.h"

#include <stdlib.h>
#include <string.h>

/**
 * Callback to free http response
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
 * Creates the response message and resumes a suspended connection
 * for the long poll endpoint and an GPIO event
 * @param request_data User data from a MHD connection
 * @param gpio GPIO number
 * @param event_type GPIO event
 * @param timestamp Event timestamp
 */
void http_connection_resume_gpio(struct t_request_data *request_data,
                                 unsigned gpio,
                                 enum mygpiod_event_types event_type,
                                 uint64_t timestamp)
{
    request_data->resume_buffer = sdscatprintf(sdsempty(),
        "{"
          "\"event\":\"%s\","
          "\"gpio\":%u,"
          "\"timestamp_ms\":%llu"
        "}",
        mygpiod_event_name(event_type),
        gpio,
        (long long unsigned)(timestamp / 1000000)
    );
    MHD_resume_connection(request_data->connection);
}

/**
 * Creates the response message and resumes a suspended connection
 * for the long poll endpoint and an input event
 * @param request_data User data from a MHD connection
 * @param device Input event device
 * @param input_data Input event data
 */
void http_connection_resume_input(struct t_request_data *request_data,
                                  const char *device,
                                  struct t_input_event *input_data)
{
    uint64_t timestamp_ms = (uint64_t)(input_data->time.tv_sec * 1000) + (uint64_t)(input_data->time.tv_usec);

    request_data->resume_buffer = sdscatprintf(sdsempty(),
        "{"
          "\"event\":\"%s\","
          "\"device\":\"%s\","
          "\"timestamp_ms\":%llu,"
          "\"type\":\"%s\","
          "\"code\":\"%s\","
          "\"value\":%u"
        "}",
        mygpiod_event_name(MYGPIOD_EVENT_INPUT),
        device,
        (long long unsigned)(timestamp_ms),
        input_event_type_name(input_data->type),
        input_event_code_name(input_data->type, input_data->code),
        input_data->value
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
