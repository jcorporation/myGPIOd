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

#include <stdio.h>
#include <stdlib.h>

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
    int rc = EXIT_SUCCESS;
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    struct t_mygpio_connection *conn = mygpio_connection_new(argv[1]);
    mygpio_connection_free(conn);

    return rc;
}
