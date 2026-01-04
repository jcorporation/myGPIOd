/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_SERVER_HTTPD_REST_API_RASPBERRY_H
#define MYGPIOD_SERVER_HTTPD_REST_API_RASPBERRY_H

#include "dist/sds/sds.h"

#include <microhttpd.h>
#include <stdbool.h>

sds rest_api_raspberry_vcio(sds buffer,
                            const char *command,
                            bool *rc);
sds rest_api_raspberry_vcio_all(sds buffer,
                            bool *rc);

#endif
