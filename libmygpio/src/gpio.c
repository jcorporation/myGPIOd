/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "libmygpio/include/libmygpio/gpio.h"

#include "libmygpio/include/libmygpio/pair.h"
#include "libmygpio/src/protocol.h"
#include "libmygpio/src/util.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// private definitions

static enum mygpio_gpio_mode parse_gpio_mode(const char *str);

// public functions

/**
 * Send the gpiolist command and receives the status
 * @param connection connection struct
 * @return true on success, else false
 */
bool mygpio_gpiolist(struct t_mygpio_connection *connection) {
    return send_line(connection, "gpiolist") &&
        recv_response_status(connection);
}

struct t_mygpio_gpio_conf *mygpio_recv_gpio(struct t_mygpio_connection *connection) {
    unsigned gpio;
    enum mygpio_gpio_mode mode;

    struct t_mygpio_pair *pair = mygpio_recv_pair(connection);
    if (pair == NULL ||
        strcmp(pair->name, "gpio") != 0 ||
        parse_uint(pair->value, &gpio, NULL, 0, 99) == false)
    {
        mygpio_free_pair(pair);
        return NULL;
    }
    mygpio_free_pair(pair);

    pair = mygpio_recv_pair(connection);
    if (pair == NULL ||
        strcmp(pair->name, "mode") != 0 ||
        (mode = parse_gpio_mode(pair->value)) == MYGPIO_GPIO_MODE_UNKNOWN)
    {
        mygpio_free_pair(pair);
        return NULL;
    }
    mygpio_free_pair(pair);

    struct t_mygpio_gpio_conf *gpio_conf = malloc(sizeof(struct t_mygpio_gpio_conf));
    assert(gpio_conf);
    gpio_conf->gpio = gpio;
    gpio_conf->mode = mode;
    return gpio_conf;
}

void mygpio_free_gpio(struct t_mygpio_gpio_conf *gpio_conf) {
    free(gpio_conf);
}

// private functions;

static enum mygpio_gpio_mode parse_gpio_mode(const char *str) {
    if (strcmp(str, "in") == 0) {
        return MYGPIO_GPIO_MODE_IN;
    }
    if (strcmp(str, "in") == 0) {
        return MYGPIO_GPIO_MODE_OUT;
    }
    return MYGPIO_GPIO_MODE_UNKNOWN;
}
