/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_SERVER_HTTPD_REST_API_RASPBERRY_H
#define MYGPIOD_SERVER_HTTPD_REST_API_RASPBERRY_H

#include "mygpiod/lib/config.h"

#include <microhttpd.h>

sds rest_api_raspberry_vcio(sds buffer,
                            const char *command,
                            bool *rc);

#endif
