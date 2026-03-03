/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Timer event handling
 */

#include "compile_time.h"
#include "mygpiod/timer_ev/event.h"

#include "config/timer_ev.h"
#include "mygpiod/config/config.h"
#include "mygpiod/lib/timer.h"
#include "mygpiod/timer_ev/action.h"
#include "mygpiod/timer_ev/timer_ev.h"


/**
 * Reads the event data from a timer event
 * @param config Pointer to config
 * @param fd Pointer to file descriptor with data to read
 * @returns true on success, else false
 */
bool timer_ev_handle_event(struct t_config *config, int *fd) {
    struct t_timer_definition *timer_definition = timer_ev_get_by_fd(&config->timer_definitions, fd);
    if (timerfd_read_value(fd) == false) {
        return false;
    }
    timer_ev_action_handle(config, timer_definition);
    timer_log_next_expire(timer_definition->name, timer_definition->fd);
    return true;
}
