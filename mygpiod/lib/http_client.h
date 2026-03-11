/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief HTTP client implementation
 */

#ifndef MYGPIOD_HTTP_CLIENT_H
#define MYGPIOD_HTTP_CLIENT_H

#include "dist/sds/sds.h"

#include <stdbool.h>

bool http_client(const char *method, const char *uri, const char *content_type, const char *postdata,
        sds *response_header, sds *response_body);

#endif
