/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Input device event type definition
 */

#ifndef MYGPIOD_INPUT_INPUT_EVENT_H
#define MYGPIOD_INPUT_INPUT_EVENT_H

#include "mygpiod/config/input_ev.h"

#include <sys/time.h>

/**
 * Input struct
 * https://www.kernel.org/doc/Documentation/input/input.txt
 * https://www.kernel.org/doc/html/latest/input/event-codes.html
 */
struct t_input_event {
    struct timeval time;  //!< timestamp, it returns the time at which the event happened.
    unsigned short type;  //!< is for example EV_REL for relative moment, EV_KEY for a keypress or release.
    unsigned short code;  //!< event code, for example REL_X or KEY_BACKSPACE
    unsigned int value;   //!< the value the event carries. Either a relative change for EV_REL, absolute new 
                          //!< value for EV_ABS (joysticks ...), or 0 for EV_KEY for release, 1 for keypress and 2 for autorepeat.
};

/**
 * Container for t_input_event and device
 */
struct t_mygpiod_input_event {
    struct t_input_device *device;  //!< Input device
    struct t_input_event data;      //!< Input event data
};

#endif
