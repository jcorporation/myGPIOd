/*
 SPDX-License-Identifier: GPL-3.0-or-later
 libmygpio (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief myGPIOd client library
 *
 * Do not include this header directly. Use libmygpio/libmygpio.h instead.
 */

#ifndef LIBMYGPIO_GPIO_H
#define LIBMYGPIO_GPIO_H

#include <stdbool.h>

struct t_mygpio_connection;

/**
 * \struct t_mygpio_gpio
 *
 * Opaque struct holding the settings of a GPIO.
 */
struct t_mygpio_gpio;

/**
 * The mode of a GPIO.
 */
enum mygpio_gpio_mode {
    MYGPIO_GPIO_MODE_UNKNOWN = -1,  //!< Unknown mode.
    MYGPIO_GPIO_MODE_IN,            //!< Input mode, myGPIOd can read events from this GPIO.
    MYGPIO_GPIO_MODE_OUT            //!< Output mode, myGPIOd can set the value to: MYGPIO_GPIO_VALUE_HIGH or MYGPIO_GPIO_VALUE_LOW.
};

/**
 * Lookups the name for the gpio mode.
 * @param mode the gpio mode.
 * @return gpio mode name
 */
const char *mygpio_gpio_lookup_mode(enum mygpio_gpio_mode mode);

/**
 * Parses a string to the gpio mode.
 * @param str string to parse
 * @return mode of the gpio
 */
enum mygpio_gpio_mode mygpio_gpio_parse_mode(const char *str);

/**
 * The value of an output GPIO.
 */
enum mygpio_gpio_value {
    MYGPIO_GPIO_VALUE_UNKNOWN = -1,  //!< Unknown state
    MYGPIO_GPIO_VALUE_LOW,           //!< GPIO state is low
    MYGPIO_GPIO_VALUE_HIGH           //!< GPIO state is high
};

/**
 * Lookups the name for the gpio value.
 * @param value the gpio value.
 * @return gpio value name
 */
const char *mygpio_gpio_lookup_value(enum mygpio_gpio_value value);

/**
 * Parses a string to a gpio value.
 * @param str string to parse
 * @return gpio value or GPIO_VALUE_UNKNOWN on error
 */
enum mygpio_gpio_value mygpio_gpio_parse_value(const char *str);

/**
 * Bias setting for an input gpio
 */
enum mygpio_gpio_bias {
    MYGPIO_BIAS_UNKNOWN = -1,
    MYGPIO_BIAS_AS_IS,
    MYGPIO_BIAS_DISABLED,
    MYGPIO_BIAS_PULL_DOWN,
    MYGPIO_BIAS_PULL_UP
};

/**
 * Lookups the name for the gpio bias.
 * @param bias the gpio bias.
 * @return gpio bias name
 */
const char *mygpio_gpio_lookup_bias(enum mygpio_gpio_bias bias);

/**
 * Parses a string to a gpio bias.
 * @param str string to parse
 * @return gpio bias or GPIO_BIAS_UNKNOWN on error
 */
enum mygpio_gpio_bias mygpio_gpio_parse_bias(const char *str);

/**
 * Events requested for an input gpio
 */
enum mygpio_event_request {
    MYGPIO_EVENT_REQUEST_UNKNOWN = -1,
    MYGPIO_EVENT_REQUEST_FALLING,
    MYGPIO_EVENT_REQUEST_RISING,
    MYGPIO_EVENT_REQUEST_BOTH
};

/**
 * Lookups the name for an event request.
 * @param value the gpio event request.
 * @return gpio value name
 */
const char *mygpio_gpio_lookup_event_request(enum mygpio_event_request event_request);

/**
 * Parses a string to an event request.
 * @param str string to parse
 * @return gpio event request or GPIO_EVENT_REQUEST_UNKNOWN on error
 */
enum mygpio_event_request mygpio_gpio_parse_event_request(const char *str);

/**
 * Clock setting for an input gpio
 */
enum mygpio_event_clock {
    MYGPIO_EVENT_CLOCK_UNKNOWN = -1,
    MYGPIO_EVENT_CLOCK_MONOTONIC,
    MYGPIO_EVENT_CLOCK_REALTIME,
    MYGPIO_EVENT_CLOCK_HTE
};

/**
 * Lookups the name for the gpio event clock.
 * @param clock the gpio clock.
 * @return gpio clock name
 */
const char *mygpio_gpio_lookup_event_clock(enum mygpio_event_clock clock);

/**
 * Parses a string to a gpio event clock.
 * @param str string to parse
 * @return gpio event clock or MYGPIO_EVENT_CLOCK_UNKNOWN on error
 */
enum mygpio_event_clock mygpio_gpio_parse_event_clock(const char *str);

/**
 * Drive setting for an output gpio
 */
enum mygpio_drive {
    MYGPIO_DRIVE_UNKNOWN = -1,
    MYGPIO_DRIVE_PUSH_PULL,
    MYGPIO_DRIVE_OPEN_DRAIN,
    MYGPIO_DRIVE_OPEN_SOURCE
};

/**
 * Lookups the name for the gpio drive setting.
 * @param drive the gpio drive.
 * @return gpio drive name
 */
const char *mygpio_gpio_lookup_drive(enum mygpio_drive drive);

/**
 * Parses a string to a gpio drive.
 * @param str string to parse
 * @return gpio bias or MYGPIO_DRIVE_UNKNOWN on error
 */
enum mygpio_drive mygpio_gpio_parse_drive(const char *str);

/**
 * Returns the current value of a configured input or output GPIO.
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

/**
 * Toggles the value of a configured output GPIO.
 * @param connection Pointer to the connection struct returned by mygpio_connection_new.
 * @param gpio GPIO number
 * @return true on success, else false.
 */
bool mygpio_gpiotoggle(struct t_mygpio_connection *connection, unsigned gpio);

/**
 * Returns the GPIO number from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO number. 
 */
unsigned mygpio_gpio_get_gpio(struct t_mygpio_gpio *gpio);

/**
 * Returns the GPIO mode from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO mode, one of enum mygpio_gpio_mode.
 */
enum mygpio_gpio_mode mygpio_gpio_get_mode(struct t_mygpio_gpio *gpio);

/**
 * Returns the GPIO value from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO mode, one of enum mygpio_gpio_mode.
 */
enum mygpio_gpio_value mygpio_gpio_get_value(struct t_mygpio_gpio *gpio);

/**
 * Returns the GPIO active_low from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO is set to active_low?
 */
bool mygpio_gpio_in_get_active_low(struct t_mygpio_gpio *gpio);

/**
 * Returns the GPIO bias from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO bias, one of enum mygpio_gpio_bias.
 */
enum mygpio_gpio_bias mygpio_gpio_in_get_bias(struct t_mygpio_gpio *gpio);

/**
 * Returns the requested events from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return requested GPIO events, one of enum event_request.
 */
enum mygpio_event_request mygpio_gpio_in_get_event_request(struct t_mygpio_gpio *gpio);

/**
 * Returns if the GPIO is debounced.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO debounced?
 */
bool mygpio_gpio_in_get_is_debounced(struct t_mygpio_gpio *gpio);

/**
 * Returns the GPIO debounce period from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO debounce period in nanoseconds.
 */
int mygpio_gpio_in_get_debounce_period(struct t_mygpio_gpio *gpio);

/**
 * Returns the GPIO event clock from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO event clock, one of enum mygpio_event_clock.
 */
enum mygpio_event_clock mygpio_gpio_in_get_event_clock(struct t_mygpio_gpio *gpio);

/**
 * Returns the GPIO drive setting from struct t_mygpio_gpio.
 * @param gpio Pointer to struct t_mygpio_gpio.
 * @return GPIO drive setting, one of enum mygpio_drive.
 */
enum mygpio_drive mygpio_gpio_out_get_drive(struct t_mygpio_gpio *gpio);

/**
 * Frees the struct received by mygpio_recv_gpio.
 * @param gpio Pointer to struct mygpio_recv_gpio.
 */
void mygpio_free_gpio(struct t_mygpio_gpio *gpio);

#endif
