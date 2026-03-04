/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief GPIO commands
 */

#include "compile_time.h"
#include "mygpioc/hook.h"

#include "libmygpio/include/libmygpio/libmygpio.h"

#include "mygpio-common/util.h"
#include "mygpioc/util.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * Triggers a hook
 * @param argc argument count
 * @param argv argument list
 * @param option_index parsed option index
 * @param conn connection struct
 * @return 0 on success, else 1
 */
int handle_hook(int argc, char **argv, int option_index, struct t_mygpio_connection *conn) {
    (void)argc;
    verbose_printf("Sending gpioset");
    if (mygpio_hook(conn, argv[option_index]) == false) {
        fprintf(stderr, "Error: %s\n", mygpio_connection_get_error(conn));
        mygpio_response_end(conn);
        return EXIT_FAILURE;
    }
    mygpio_response_end(conn);
    return EXIT_SUCCESS;
}
