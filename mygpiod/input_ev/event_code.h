/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

/*! \file
 * \brief Input device event code parsing
 */

#ifndef MYGPIOD_INPUT_EVENT_CODE_H
#define MYGPIOD_INPUT_EVENT_CODE_H

const char *input_event_code_name(unsigned short event_type, unsigned short event_code);
unsigned short input_event_code_parse(const char *name);

#endif
