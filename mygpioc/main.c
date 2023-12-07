/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 * https://github.com/jcorporation/myGPIOd
 *
 * myGPIOd is based on the gpiomon tool from
 * https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/about/
 */

#include "compile_time.h"

#include "libmygpio/include/libmygpio.h"

#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MYGPIOD_ENABLE_ASAN
const char *__asan_default_options(void) {
    return "abort_on_error=1:fast_unwind_on_malloc=0:detect_stack_use_after_return=1";
}
#endif

#ifdef MYGPIOD_ENABLE_TSAN
const char *__asan_default_options(void) {
    return "abort_on_error=1";
}
#endif

#ifdef MYGPIOD_ENABLE_UBSAN
const char *__asan_default_options(void) {
    return "abort_on_error=1:print_stacktrace=1";
}
#endif

int main(int argc, char **argv) {
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    struct t_mygpio_connection *conn = mygpio_connection_new(argv[1]);
    if (conn == NULL) {
        printf("Out of memory\n");
        return EXIT_FAILURE;
    }

    if (mygpio_connection_get_state(conn) != MYGPIO_STATE_OK) {
        printf("%s\n", mygpio_connection_get_error(conn));
        mygpio_connection_free(conn);
        return EXIT_FAILURE;
    }

    const unsigned *version = mygpio_connection_get_version(conn);
    printf("Connected, server version %u.%u.%u\n", version[0], version[1], version[2]);

    struct t_mygpio_pair *pair = mygpio_recv_pair(conn);
    if (pair == NULL) {
        printf("No messages pending\n");
    }

    if (mygpio_send_idle(conn) == true) {
        printf("In idle mode\n");
    }

    if (mygpio_recv_idle(conn, -1) == true) {
        printf("Event waiting\n");
    }

    mygpio_connection_free(conn);

    return EXIT_SUCCESS;
}
