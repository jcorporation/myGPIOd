/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_SERVER_HTTPD_UTIL_H
#define MYGPIOD_SERVER_HTTPD_UTIL_H

#include <microhttpd.h>

enum http_method {
    HTTP_UNKNOWN = -1,
    HTTP_GET,
    HTTP_OPTIONS,
    HTTP_PATCH,
};

enum MHD_Result http_respond(struct MHD_Connection *connection,
                             unsigned int status_code,
                             const char *content_type,
                             const char *message);
enum http_method http_parse_method(const char *method);
const char *http_lookup_method(enum http_method method);

#endif
