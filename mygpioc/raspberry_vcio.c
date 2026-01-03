/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "libmygpio/include/libmygpio/libmygpio_connection.h"
#include "libmygpio/include/libmygpio/libmygpio_protocol.h"
#include "libmygpio/include/libmygpio/libmygpio_raspberry_vcio.h"
#include "libmygpio/src/pair.h"
#include "mygpioc/gpio.h"
#include "mygpioc/util.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * Gets the value of an vcio command.
 * Private function, used by the handle_vcio* functions
 * @param conn connection struct
 * @param command Command to send
 * @return 0 on success, else 1
 */
static int send_vcio(struct t_mygpio_connection *conn, const char *command) {
    verbose_printf("Sending %s", command);
    struct t_mygpio_pair *pair = mygpio_vcio(conn, command);
    if (pair == NULL) {
        fprintf(stderr, "Error: %s\n", mygpio_connection_get_error(conn));
        mygpio_response_end(conn);
        return EXIT_FAILURE;
    }
    printf("Value: %s\n", pair->value);
    mygpio_free_pair(pair);
    mygpio_response_end(conn);
    return EXIT_SUCCESS;
}

/**
 * Gets the temperature from /dev/vcio
 * @param argc argument count
 * @param argv argument list
 * @param option_index parsed option index
 * @param conn connection struct
 * @return 0 on success, else 1
 */
int handle_vciotemp(int argc, char **argv, int option_index, struct t_mygpio_connection *conn) {
    (void)argc;
    (void)argv;
    (void)option_index;
    return send_vcio(conn, "vciotemp");
}

/**
 * Gets the core voltage from /dev/vcio
 * @param argc argument count
 * @param argv argument list
 * @param option_index parsed option index
 * @param conn connection struct
 * @return 0 on success, else 1
 */
int handle_vciovolts(int argc, char **argv, int option_index, struct t_mygpio_connection *conn) {
    (void)argc;
    (void)argv;
    (void)option_index;
    return send_vcio(conn, "vciovolts");
}

/**
 * Gets the clock speed from /dev/vcio
 * @param argc argument count
 * @param argv argument list
 * @param option_index parsed option index
 * @param conn connection struct
 * @return 0 on success, else 1
 */
int handle_vcioclock(int argc, char **argv, int option_index, struct t_mygpio_connection *conn) {
    (void)argc;
    (void)argv;
    (void)option_index;
    return send_vcio(conn, "vcioclock");
}

/**
 * Gets the throttled mask from /dev/vcio
 * @param argc argument count
 * @param argv argument list
 * @param option_index parsed option index
 * @param conn connection struct
 * @return 0 on success, else 1
 */
int handle_vciothrottled(int argc, char **argv, int option_index, struct t_mygpio_connection *conn) {
    (void)argc;
    (void)argv;
    (void)option_index;
    return send_vcio(conn, "vciothrottled");
}