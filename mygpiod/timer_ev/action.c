/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Timer event action handling
 */

#include "compile_time.h"
#include "mygpiod/timer_ev/action.h"

#include "mygpiod/actions/execute.h"
#include "mygpiod/config/timer_ev.h"
#include "mygpiod/lib/log.h"

#include <stddef.h>
#include <time.h>

/**
 * Handles the configured actions for an timer event.
 * @param config Pointer to config
 * @param timer_definition Timer event configuration
 */
void timer_ev_action_handle(struct t_config *config, struct t_timer_definition *timer_definition) {
    // Weekday check
    time_t t = time(NULL);
    struct tm now;
    if (localtime_r(&t, &now) == NULL) {
        MYGPIOD_LOG_ERROR("Localtime is NULL");
        return;
    }
    int wday = now.tm_wday;
    wday = wday > 0 ? wday - 1 : 6;
    if (timer_definition->weekdays[wday] == false) {
        MYGPIOD_LOG_DEBUG("Skipping timer \"%s\" it is not enabled on this weekday.", timer_definition->name);
        return;
    }
    MYGPIOD_LOG_INFO("Timer \"%s\" triggered.", timer_definition->name);
    // Execute action
    action_execute(config, &timer_definition->action);
}
