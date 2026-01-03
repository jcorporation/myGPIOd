/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_SERVER_HTTPD_UTIL_H
#define MYGPIOD_SERVER_HTTPD_UTIL_H

enum http_method {
    HTTP_UNKNOWN = -1,
    HTTP_GET,
    HTTP_OPTIONS,
    HTTP_POST,
};

enum http_method parse_method(const char *method);

#endif
