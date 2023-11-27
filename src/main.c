/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 * https://github.com/jcorporation/myGPIOd
 *
 * myGPIOd is based on the gpiomon tool from
 * https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/about/
 */

#include "compile_time.h"
#include "action.h"
#include "config.h"
#include "event.h"
#include "log.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    int rc = EXIT_SUCCESS;
    log_on_tty = isatty(fileno(stdout)) ? true: false;
    log_to_syslog = false;
    
    #ifdef MYGPIOD_DEBUG
        set_loglevel(LOG_DEBUG);
    #else
        set_loglevel(CFG_LOGLEVEL);
    #endif

    MYGPIOD_LOG_NOTICE("Starting myGPIOd %s", MYGPIOD_VERSION);
    MYGPIOD_LOG_NOTICE("https://github.com/jcorporation/myGPIOd");
    MYGPIOD_LOG_NOTICE("libgpiod %s", gpiod_version_string());

    // Handle commandline parameter
    char *config_file = NULL;
    if (argc == 2 && strncmp(argv[1], "/", 1) == 0) {
        config_file = strdup(argv[1]);
    }
    else {
        config_file = strdup("/etc/mygpiod.conf");
    }

    // Read config
    MYGPIOD_LOG_INFO("Reading \"%s\"", config_file);
    struct t_config *config = config_new();
    if (config == NULL) {
        MYGPIOD_LOG_EMERG("Out of memory");
        free(config_file);
        return EXIT_FAILURE;
    }
    if (config_read(config, config_file) == false) {
        config_clear(config);
        free(config);
        free(config_file);
        return EXIT_FAILURE;
    }
    free(config_file);
    
    // Set loglevel
    #ifdef MYGPIOD_DEBUG
        set_loglevel(LOG_DEBUG);
    #else
        set_loglevel(config->loglevel);
    #endif

    if (config->syslog == true) {
        openlog(MYGPIOD_NAME, LOG_CONS, LOG_DAEMON);
        log_to_syslog = true;
    }

    // create the array of gpios to monitor
    // set the state for output gpios
    unsigned gpios_in[GPIOD_LINE_BULK_MAX_LINES];
    unsigned gpios_in_count = 0;
    unsigned gpios_out_count = 0;
    struct t_gpio_node *current = config->gpios;
    while (current != NULL) {
        if (current->mode == GPIO_MODE_INPUT &&
            value_in_array(current->gpio, gpios_in, gpios_in_count) == false)
        {
            gpios_in[gpios_in_count] = current->gpio;
            gpios_in_count++;
        }
        if (current->mode == GPIO_MODE_OUTPUT) {
            //TODO: set gpio
            gpios_out_count++;
        }
        current = current->next;
    }

    if (gpios_in_count == 0) {
        MYGPIOD_LOG_ERROR("No gpios for monitoring configured");
        config_clear(config);
        free(config);
        return EXIT_FAILURE;
    }

    // data structure
    struct t_mon_ctx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.config = config;
    ctx.sigfd = make_signalfd();
    if (ctx.sigfd <= 0) {
        config_clear(config);
        free(config);
        return EXIT_FAILURE;
    }
    
    // set flags for bias support
    int flags = 0;
    flags |= config->bias;

    // poll timeout
    struct timespec timeout = { 10, 0 };

    // Main event handling loop
    MYGPIOD_LOG_INFO("Entering event handling loop");
    MYGPIOD_LOG_INFO("Monitoring %u gpios", gpios_in_count);
    int rv = gpiod_ctxless_event_monitor_multiple_ext(
        config->chip, config->edge, gpios_in, gpios_in_count,
        config->active_low, MYGPIOD_NAME, &timeout, poll_callback,
        event_callback, &ctx, flags);

    if (rv == -1) {
        MYGPIOD_LOG_ERROR("Error waiting for events");
        rc = EXIT_FAILURE;
    }

    // Cleanup
    action_delay_abort(config);
    config_clear(config);
    free(config);
    if (rc == EXIT_SUCCESS) {
        MYGPIOD_LOG_INFO("Exiting gracefully, thank you for using myGPIOd");
    }
    return rc;
}
