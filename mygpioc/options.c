/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpioc/options.h"
#include "mygpioc/util.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

static struct option long_options[] = {
    {"socket",  required_argument, 0, 's'},
    {"timeout", required_argument, 0, 't'},
    {"help",    no_argument,       0, 'h'},
    {"version", no_argument,       0, 'v'}
};

/**
 * Prints the command line usage information
 * @param config pointer to config struct
 * @param cmd argv[0] from main function
 */
void print_usage(void) {
    fprintf(stderr, "\nUsage: mygpioc [OPTIONS] <COMMAND> [<ARGUMENTS>]\n\n"
                    "mygpioc %s\n"
                    "(c) 2020-2023 Juergen Mang <mail@jcgames.de>\n"
                    "https://github.com/jcorporation/myGPIOd\n\n"
                    "Options:\n"
                    "  -s, --socket           path to myGPIOd socket\n"
                    "  -t, --timeout          connection timeout in milliseconds\n"
                    "  -h, --help             displays this help\n"
                    "  -v, --version          displays this help\n",
        MYGPIO_VERSION);
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
    while ((n = getopt_long(argc, argv, "vhs:", long_options, NULL)) != -1) {
        switch(n) {
            case 'v':
            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);
            case 's':
                options->socket = optarg;
                break;
            case 't':
                if (parse_int(optarg, &options->timeout, NULL, 1, 1000000) == false) {
                    print_usage();
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                print_usage();
                exit(EXIT_FAILURE);
        }
    }
    return optind;
}
