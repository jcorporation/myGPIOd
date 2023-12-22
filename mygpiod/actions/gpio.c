/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/actions/gpio.h"

#include "dist/sds/sds.h"
#include "mygpio-common/util.h"
#include "mygpiod/gpio/output.h"
#include "mygpiod/lib/log.h"
#include "mygpiod/lib/util.h"

#include <errno.h>
#include <stdlib.h>

/**
 * Sets an output gpio
 * @param cmd command to parse, format:
 *            {gpio} {active|inactive}
 * @returns true on success, else false
 */
bool action_gpioset(struct t_config *config, const char *cmd) {
    int count = 0;
    sds *args = sdssplitargs(cmd, &count);

    if (count != 2) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments");
        sdsfreesplitres(args, count);
        return false;
    }
    unsigned gpio;
    if (mygpio_parse_uint(args[0], &gpio, NULL, 0, GPIOS_MAX) == false) {
        MYGPIOD_LOG_ERROR("Invalid gpio number");
        sdsfreesplitres(args, count);
        return false;
    }
    errno = 0;
    enum gpiod_line_value value = parse_gpio_value(args[1]);
    if (errno == EINVAL) {
        MYGPIOD_LOG_ERROR("Invalid value");
        sdsfreesplitres(args, count);
        return false;
    }
    sdsfreesplitres(args, count);
    return gpio_set_value(config, gpio, value);
}

/**
 * Toggle the value of an output gpio
 * @param cmd command to parse, format:
 *            {gpio}
 * @returns true on success, else false
 */
bool action_gpiotoggle(struct t_config *config, const char *cmd) {
    int count = 0;
    sds *args = sdssplitargs(cmd, &count);

    if (count != 1) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments");
        sdsfreesplitres(args, count);
        return false;
    }
    unsigned gpio;
    if (mygpio_parse_uint(args[0], &gpio, NULL, 0, GPIOS_MAX) == false) {
        MYGPIOD_LOG_ERROR("Invalid gpio number");
        sdsfreesplitres(args, count);
        return false;
    }
    sdsfreesplitres(args, count);
    return gpio_toggle_value(config, gpio);
}

/**
 * Blink the value of an output gpio
 * @param cmd command to parse, format:
 *            {gpio} {timeout} {interval}
 * @returns true on success, else false
 */
bool action_gpioblink(struct t_config *config, const char *cmd) {
    int count = 0;
    sds *args = sdssplitargs(cmd, &count);

    if (count != 3) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments");
        sdsfreesplitres(args, count);
        return false;
    }
    unsigned gpio;
    if (mygpio_parse_uint(args[0], &gpio, NULL, 0, GPIOS_MAX) == false) {
        MYGPIOD_LOG_ERROR("Invalid gpio number");
        sdsfreesplitres(args, count);
        return false;
    }
    int timeout;
    if (mygpio_parse_int(args[0], &timeout, NULL, 0, TIMEOUT_MS_MAX) == false) {
        MYGPIOD_LOG_ERROR("Invalid timeout");
        sdsfreesplitres(args, count);
        return false;
    }
    int interval;
    if (mygpio_parse_int(args[0], &interval, NULL, 0, TIMEOUT_MS_MAX) == false) {
        MYGPIOD_LOG_ERROR("Invalid interval");
        sdsfreesplitres(args, count);
        return false;
    }
    sdsfreesplitres(args, count);
    return gpio_blink(config, gpio, timeout, interval);
}
