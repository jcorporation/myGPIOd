/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/actions/gpioset.h"

#include "dist/sds/sds.h"
#include "mygpio-common/util.h"
#include "mygpiod/gpio/gpio.h"
#include "mygpiod/gpio/output.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/util.h"

#include <errno.h>
#include <stdlib.h>

/**
 * Sets an output gpio
 * @param cmd command to parse
 */
bool action_gpioset(struct t_config *config, const char *cmd) {
    int count = 0;
    sds *args = sdssplitargs(cmd, &count);

    if (count != 2) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments");
        return false;
    }
    unsigned gpio;
    if (mygpio_parse_uint(args[0], &gpio, NULL, 0, GPIOS_MAX) == false) {
        MYGPIOD_LOG_ERROR("Invalid gpio number");
        return false;
    }
    errno = 0;
    enum gpiod_line_value value = parse_gpio_value(args[1]);
    if (errno == EINVAL) {
        MYGPIOD_LOG_ERROR("Invalid value");
        return false;
    }
    return gpio_set_value(config, gpio, value);
}

/**
 * Toggle the value of an output gpio
 * @param cmd command to parse
 */
bool action_gpiotoggle(struct t_config *config, const char *cmd) {
    int count = 0;
    sds *args = sdssplitargs(cmd, &count);

    if (count != 1) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments");
        return false;
    }
    unsigned gpio;
    if (mygpio_parse_uint(args[0], &gpio, NULL, 0, GPIOS_MAX) == false) {
        MYGPIOD_LOG_ERROR("Invalid gpio number");
        return false;
    }
    return gpio_toggle_value(config, gpio);
}
