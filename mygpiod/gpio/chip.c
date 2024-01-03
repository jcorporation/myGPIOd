/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/gpio/chip.h"

#include "mygpiod/lib/config.h"
#include "mygpiod/lib/log.h"

#include <gpiod.h>

/**
 * Opens the gpio chip.
 * @param config pointer to config
 * @return true on success, else false
 */
bool gpio_open_chip(struct t_config *config) {
    MYGPIOD_LOG_INFO("Opening chip \"%s\"", config->chip_path);
    config->chip = gpiod_chip_open(config->chip_path);
    if (config->chip == NULL) {
        MYGPIOD_LOG_ERROR("Error opening chip");
        return false;
    }
    return true;
}
