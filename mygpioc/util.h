/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Utility functions
 */

#ifndef MYGPIOC_UTIL_H
#define MYGPIOC_UTIL_H

#include <stdbool.h>
#include <stdint.h>

extern bool verbose; //!< Global verbose flag

void verbose_printf(const char *fmt, ...);

#endif
