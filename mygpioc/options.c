/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpioc/options.h"

#include "mygpio-common/util.h"
#include "mygpioc/util.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct option long_options[] = {
    {"help",    no_argument,       0, 'h'},
    {"socket",  required_argument, 0, 's'},
    {"timeout", required_argument, 0, 't'},
    {"verbose", no_argument,       0, 'v'}
};

/**
 * Prints the command line usage information
 * @param config pointer to config struct
 * @param cmd argv[0] from main function
 */
void print_usage(void) {
    fprintf(stderr, "\nUsage: mygpioc [options] <command> [<arguments>]\n\n"
                    "myGPIOc %s\n"
                    "(c) 2020-2023 Juergen Mang <mail@jcgames.de>\n"
                    "https://github.com/jcorporation/myGPIOd\n\n"
                    "Options:\n"
                    "  -h, --help                          Displays this help\n"
                    "  -s, --socket                        Path to myGPIOd socket\n"
                    "  -t, --timeout                       Connection timeout in milliseconds\n"
                    "  -v, --verbose                       Verbose output\n"
                    "\n"
                    "Commands:\n"
                    "  gpioget <number>                    Gets the current value of a gpio\n"
                    "  gpioset <number> <active|inactive>  Sets the value of an output gpio\n"
                    "  gpiotoggle <number>                 Toggles the value of an output gpio\n"
                    "  gpioinfo <number>                   Gets the settings of a gpio\n"
                    "  gpiolist                            Lists all configured gpios with its mode and value\n"
                    "  idle [<timeout>]                    Waits for idle events, timeout is in milliseconds\n"
                    "\n",
        MYGPIO_VERSION
    );
}

/**
 * Handles the command line arguments
 * @param config pointer to myMPD static configuration
 * @param argc from main function
 * @param argv from main function
 * @return optind
 */
int handle_options(int argc, char **argv, struct t_options *options) {
    int n = 0;
    while ((n = getopt_long(argc, argv, "hs:t:v", long_options, NULL)) != -1) {
        switch(n) {
            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);
                break;
            case 's':
                options->socket = optarg;
                break;
            case 't':
                if (mygpio_parse_int(optarg, &options->timeout_ms, NULL, 1, 1000000) == false) {
                    fprintf(stderr, "Invalid timeout\n");
                    print_usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'v':
                verbose = true;
                break;
            default:
                print_usage();
                exit(EXIT_FAILURE);
        }
    }
    return optind;
}

/**
 * Initializes the options struct
 * @param options options to initialize
 */
void init_options(struct t_options *options) {
    verbose = false;
    options->timeout_ms = 10000;  //milliseconds = 10 seconds
    options->socket = strdup(CFG_SOCKET_PATH);
}

/**
 * Clears the options struct
 * @param options options to initialize
 */
void clear_options(struct t_options *options) {
    free(options->socket);
}
