/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/server_http/util.h"

#include <string.h>

/**
 * Parses a string to a HTTP method
 * @param method HTTP method string to parse
 * @return enum http_method 
 */
enum http_method parse_method(const char *method) {
    if (strcmp(method, "GET") == 0) {
        return HTTP_GET;
    }
    if (strcmp(method, "OPTIONS") == 0) {
        return HTTP_OPTIONS;
    }
    if (strcmp(method, "POST") == 0) {
        return HTTP_POST;
    }
    return HTTP_UNKNOWN;
}
