/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

// Do not include this file directly, use libmygpio.h

#ifndef LIBMYGPIO_GPIO_H
#define LIBMYGPIO_GPIO_H

#include <stdbool.h>

struct t_mygpio_connection;

/**
 * The mode of a GPIO.
 */
enum mygpio_gpio_mode {
    MYGPIO_GPIO_MODE_UNKNOWN = -1,  //!< Unknown state.
    MYGPIO_GPIO_MODE_IN,            //!< Input mode, myGPIOd can read events from this GPIO.
    MYGPIO_GPIO_MODE_OUT            //!< Output mode, myGPIOd can set the value to: MYGPIO_GPIO_VALUE_HIGH or MYGPIO_GPIO_VALUE_LOW.
};

/**
 * The value of an output GPIO.
 */
enum mygpio_gpio_value {
    MYGPIO_GPIO_VALUE_UNKNOWN = -1,  //!< Unknown state
    MYGPIO_GPIO_VALUE_LOW,           //!< GPIO state is low
    MYGPIO_GPIO_VALUE_HIGH           //!< GPIO state is high
};

/**
 * Opaque struct holding the configuration of a GPIO.
 */
struct t_mygpio_gpio_conf;

/**
 * Lists the modes of all configured GPIOs.
 * Retrieve the list elements with mygpio_recv_gpio_conf and end the response with mygpio_response_end.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @return bool true on success, else false.
 */
bool mygpio_gpiolist(struct t_mygpio_connection *connection);

/**
 * Receives a list element of mygpio_gpiolist.
 * Free it with mygpio_free_gpio_conf.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @return Allocated struct t_mygpio_gpio_conf or NULL on list end or error.
 */
struct t_mygpio_gpio_conf *mygpio_recv_gpio_conf(struct t_mygpio_connection *connection);

/**
 * Returns the GPIO number from struct t_mygpio_gpio_conf.
 * @param gpio_conf Pointer to struct t_mygpio_gpio_conf.
 * @return GPIO number. 
 */
unsigned mygpio_gpio_conf_get_gpio(struct t_mygpio_gpio_conf *gpio_conf);

/**
 * Returns the GPIO mode from struct t_mygpio_gpio_conf.
 * @param gpio_conf Pointer to struct t_mygpio_gpio_conf.
 * @return GPIO mode, one of enum mygpio_gpio_mode.
 */
enum mygpio_gpio_mode mygpio_gpio_conf_get_mode(struct t_mygpio_gpio_conf *gpio_conf);

/**
 * Frees the struct received by mygpio_recv_gpio_conf.
 * @param gpio_conf Pointer to struct mygpio_recv_gpio_conf.
 */
void mygpio_free_gpio_conf(struct t_mygpio_gpio_conf *gpio_conf);

/**
 * Returns the current value of a configured input GPIO.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @param gpio GPIO number
 * @return Value of the GPIO or MYGPIO_GPIO_VALUE_UNKNOWN on error.
 */
enum mygpio_gpio_value mygpio_gpioget(struct t_mygpio_connection *connection, unsigned gpio);

/**
 * Sets the value of a configured output GPIO.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @param gpio GPIO number
 * @param value Value to set: MYGPIO_GPIO_VALUE_LOW or MYGPIO_GPIO_VALUE_HIGH
 * @return true on success, else false.
 */
bool mygpio_gpioset(struct t_mygpio_connection *connection, unsigned gpio, enum mygpio_gpio_value value);

#endif
