/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/actions/gpio.h"

#include "mygpio-common/util.h"
#include "mygpiod/gpio/output.h"
#include "mygpiod/gpio/util.h"
#include "mygpiod/lib/log.h"

#include <errno.h>
#include <stdlib.h>

/**
 * Sets an output gpio
 * @param action Action struct, options must be:
 *            {gpio} {active|inactive}
 * @returns true on success, else false
 */
bool action_gpioset(struct t_config *config, struct t_action *action) {
    if (action->options_count != 2) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments: %d", action->options_count);
        return false;
    }
    unsigned gpio;
    if (mygpio_parse_uint(action->options[0], &gpio, NULL, 0, GPIOS_MAX) == false) {
        MYGPIOD_LOG_ERROR("Invalid gpio number");
        return false;
    }
    errno = 0;
    enum gpiod_line_value value = parse_gpio_value(action->options[1]);
    if (errno == EINVAL) {
        MYGPIOD_LOG_ERROR("Invalid value");
        return false;
    }
    return gpio_set_value(config, gpio, value);
}

/**
 * Toggle the value of an output gpio
 * @param action Action struct, options must be:
 *            {gpio}
 * @returns true on success, else false
 */
bool action_gpiotoggle(struct t_config *config, struct t_action *action) {
    if (action->options_count != 1) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments: %d", action->options_count);
        return false;
    }
    unsigned gpio;
    if (mygpio_parse_uint(action->options[0], &gpio, NULL, 0, GPIOS_MAX) == false) {
        MYGPIOD_LOG_ERROR("Invalid gpio number");
        return false;
    }
    return gpio_toggle_value(config, gpio);
}

/**
 * Blink the value of an output gpio
 * @param action Action struct, options must be:
 *            {gpio} {timeout} {interval}
 * @returns true on success, else false
 */
bool action_gpioblink(struct t_config *config, struct t_action *action) {
    if (action->options_count != 3) {
        MYGPIOD_LOG_ERROR("Invalid number of arguments: %d", action->options_count);
        return false;
    }
    unsigned gpio;
    if (mygpio_parse_uint(action->options[0], &gpio, NULL, 0, GPIOS_MAX) == false) {
        MYGPIOD_LOG_ERROR("Invalid gpio number");
        return false;
    }
    int timeout;
    if (mygpio_parse_int(action->options[1], &timeout, NULL, 0, TIMEOUT_MS_MAX) == false) {
        MYGPIOD_LOG_ERROR("Invalid timeout");
        return false;
    }
    int interval;
    if (mygpio_parse_int(action->options[2], &interval, NULL, 0, TIMEOUT_MS_MAX) == false) {
        MYGPIOD_LOG_ERROR("Invalid interval");
        return false;
    }
    return gpio_blink(config, gpio, timeout, interval);
}
