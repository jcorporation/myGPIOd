/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief myGPIOd client library
 *
 * Do not include this header directly. Use libmygpio/libmygpio.h instead.
 */

#ifndef LIBMYGPIO_HOOK_H
#define LIBMYGPIO_HOOK_H

#include "libmygpio_gpio_struct.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct t_mygpio_connection;

/**
 * @defgroup libmygpio_hook_functions Hook functions
 *
 * @brief This module provides functions to trigger hooks.
 *
 * @{
 */

/**
 * Triggers a hook
 * @param connection connection struct
 * @param name Hook name
 * @return true on success, else false
 */
bool mygpio_hook(struct t_mygpio_connection *connection, const char *name);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
