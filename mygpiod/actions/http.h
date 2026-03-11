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

#include "mygpiod/actions/actions.h"

#include <stdbool.h>

bool action_http_async(struct t_action *action);
bool action_http2_async(const char *method, const char *uri, const char *content_type, const char *postdata);

#endif
