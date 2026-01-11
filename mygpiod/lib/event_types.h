/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_EVENT_TYPES_H
#define MYGPIOD_EVENT_TYPES_H

/**
 * myGPIOD event types
 */
enum mygpiod_event_types {
    MYGPIOD_EVENT_FALLING,
    MYGPIOD_EVENT_RISING,
    MYGPIOD_EVENT_LONG_PRESS,
    MYGPIOD_EVENT_LONG_PRESS_RELEASE
};

#endif
