/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef MYGPIOD_INPUT_EVENT_TYPE_H
#define MYGPIOD_INPUT_EVENT_TYPE_H

const char *input_event_type_name(unsigned short event_type);
unsigned short input_event_type_parse(const char *name);

#endif
