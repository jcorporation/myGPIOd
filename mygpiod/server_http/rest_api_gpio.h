/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_SERVER_HTTPD_REST_API_GPIO_H
#define MYGPIOD_SERVER_HTTPD_REST_API_GPIO_H

#include "lib/action.h"
#include "mygpiod/lib/config.h"

#include <microhttpd.h>

sds rest_api_gpio_get(struct t_config *config,
                      sds buffer,
                      bool *rc);
sds rest_api_gpio_gpio_get(struct t_config *config,
                           sds buffer,
                           unsigned gpio_nr,
                           bool *rc);
sds rest_api_gpio_gpio_options(struct t_config *config,
                               sds buffer,
                               unsigned gpio_nr,
                               bool *rc);
sds rest_api_gpio_gpio_blink(struct t_config *config,
                             sds buffer,
                             unsigned gpio_nr,
                             struct MHD_Connection *connection,
                             bool *rc);
sds rest_api_gpio_gpio_set(struct t_config *config,
                            sds buffer,
                            unsigned gpio_nr,
                            struct MHD_Connection *connection,
                            bool *rc);
sds rest_api_gpio_gpio_toggle(struct t_config *config,
                            sds buffer,
                            unsigned gpio_nr,
                            bool *rc);

#endif
