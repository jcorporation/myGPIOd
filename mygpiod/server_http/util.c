/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server_http/util.h"

#include "dist/sds/sds.h"

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
        case HTTP_PATCH:   return "POST";
        case HTTP_UNKNOWN: return NULL;
    }
    return NULL;
}
