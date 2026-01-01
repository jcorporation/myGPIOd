/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_SERVER_HTTPD_H
#define MYGPIOD_SERVER_HTTPD_H

#include "mygpiod/lib/config.h"

#include <microhttpd.h>

struct MHD_Daemon *httpd_start(struct t_config *config);

#endif
