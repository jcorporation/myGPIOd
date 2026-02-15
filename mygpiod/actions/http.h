/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief HTTP actions
 */

#ifndef MYGPIOD_ACTIONS_HTTP_H
#define MYGPIOD_ACTIONS_HTTP_H

#include "mygpiod/lib/action.h"

#include <stdbool.h>

bool action_http(struct t_action *action);
bool action_http2(const char *method, const char *uri, const char *content_type, const char *postdata);

#endif
