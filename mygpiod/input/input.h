/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_INPUT_H
#define MYGPIOD_INPUT_H

#include "mygpiod/lib/config.h"
#include "mygpiod/event_loop/event_loop.h"

/**
 * Input struct
 * https://www.kernel.org/doc/Documentation/input/input.txt
 */
struct t_input_event {
    struct timeval time;  //!< timestamp, it returns the time at which the event happened.
    unsigned short type;  //!< is for example EV_REL for relative moment, EV_KEY for a keypress or release.
    unsigned short code;  //!< event code, for example REL_X or KEY_BACKSPACE
    unsigned int value;   //!< the value the event carries. Either a relative change for EV_REL, absolute new 
                          //!< value for EV_ABS (joysticks ...), or 0 for EV_KEY for release, 1 for keypress and 2 for autorepeat.
};

bool inputs_open(struct t_config *config, struct t_poll_fds *poll_fds);
bool input_handle_event(struct t_config *config, int *fd);

#endif
