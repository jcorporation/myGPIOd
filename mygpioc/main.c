/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "libmygpio/include/libmygpio/libmygpio.h"

#include "mygpioc/gpio.h"
#include "mygpioc/idle.h"
#include "mygpioc/options.h"
#include "mygpioc/util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool verbose;

typedef int (*command_handler)(int argc, char **argv, int option_index, struct t_mygpio_connection *conn);

struct t_commands {
    const char *command;
    command_handler handler;
    int min_options;
    int max_options;
};

static struct t_commands commands[] = {
    { "idle", handle_idle, 0, 1 },
    { "gpioinfo", handle_gpioinfo, 1, 1 },
    { "gpiolist", handle_gpiolist, 0, 0 },
    { "gpioget", handle_gpioget, 1, 1 },
    { "gpioset", handle_gpioset, 2, 2 },
    { "gpiotoggle", handle_gpiotoggle, 1, 1 },
    { "gpioblink", handle_gpioblink, 3, 3 },
    { NULL, NULL, 0, 0}
};

const struct t_commands *get_command(const char *command_string, int argc, int option_index) {
    const struct t_commands *p = NULL;
    for (p = commands; p->command != NULL; p++) {
        if (strcasecmp(command_string, p->command) == 0) {
            break;
        }
    }
    if (p->command != NULL) {
        if (argc < option_index + p->min_options ||
            argc > option_index + p->max_options)
        {
            fprintf(stderr, "Invalid number of arguments\n");
            return NULL;
        }
    }
    return p;
}

int main(int argc, char **argv) {
    struct t_options options;
    init_options(&options);
    int option_index = handle_options(argc, argv, &options);
    if (argc <= option_index) {
        print_usage();
        clear_options(&options);
        return EXIT_FAILURE;
    }

    const char *command_string = argv[option_index];
    option_index++;
    const struct t_commands *command = get_command(command_string, argc, option_index);
    if (command == NULL) {
        print_usage();
        clear_options(&options);
        return EXIT_FAILURE;
    }

    verbose_printf("Connecting to myGPIOd: %s", options.socket);
    struct t_mygpio_connection *conn = mygpio_connection_new(options.socket, options.timeout_ms);
    if (conn == NULL) {
        fprintf(stderr, "Out of memory\n");
        clear_options(&options);
        return EXIT_FAILURE;
    }
    if (mygpio_connection_get_state(conn) != MYGPIO_STATE_OK) {
        fprintf(stderr, "Error: %s\n", mygpio_connection_get_error(conn));
        mygpio_connection_free(conn);
        clear_options(&options);
        return EXIT_FAILURE;
    }

    if (verbose == true) {
        const unsigned *version = mygpio_connection_get_version(conn);
        printf("Connected, server version %u.%u.%u\n", version[0], version[1], version[2]);
    }

    int rc = command->handler(argc, argv, option_index, conn);

    verbose_printf("Closing connection");
    mygpio_connection_free(conn);
    clear_options(&options);
    return rc;
}
