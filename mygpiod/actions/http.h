/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_ACTIONS_HTTP_H
#define MYGPIOD_ACTIONS_HTTP_H

#include <stdbool.h>

bool action_http(const char *cmd);
bool action_http2(const char *method, const char *uri, const char *content_type, const char *postdata);

#endif
