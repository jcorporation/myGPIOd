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

#ifndef LIBMYGPIO_GPIO_STRUCT_H
#define LIBMYGPIO_GPIO_STRUCT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct t_mygpio_connection;

/**
 * @struct t_mygpio_gpio
 * @{
 * The opaque GPIO obect. You can not access it directly.
 * Refer to @ref gpio_settings for function that operate on this struct.
 * @}
 */
struct t_mygpio_gpio;

/**
 * @defgroup gpio_settings GPIO
 *
 * @brief This module provides functions to access the t_mygpio_gpio struct,
 * received by gpioinfo or gpiolist.
 *
 * @{
 */

/**
 * The mode of a GPIO.
 */
enum mygpio_gpio_mode {
    MYGPIO_GPIO_MODE_UNKNOWN = -1,  //!< Unknown GPIO mode.
    MYGPIO_GPIO_MODE_IN,            //!< Input mode, myGPIOd can read events from this GPIO.
    MYGPIO_GPIO_MODE_OUT            //!< Output mode, myGPIOd can set the value to: MYGPIO_GPIO_VALUE_HIGH or MYGPIO_GPIO_VALUE_LOW.
};

/**
 * The value of an output or input GPIO.
 */
enum mygpio_gpio_value {
    MYGPIO_GPIO_VALUE_UNKNOWN = -1,  //!< Unknown GPIO value
    MYGPIO_GPIO_VALUE_LOW,           //!< GPIO state is low
    MYGPIO_GPIO_VALUE_HIGH           //!< GPIO state is high
};

/**
 * Bias setting for an input GPIO.
 */
enum mygpio_gpio_bias {
    MYGPIO_BIAS_UNKNOWN = -1,  //!< Unknown bias setting
    MYGPIO_BIAS_AS_IS,         //!< Do not touch the bias state
    MYGPIO_BIAS_DISABLED,      //!< Disable the bias
    MYGPIO_BIAS_PULL_DOWN,     //!< Pull-down the GPIO
    MYGPIO_BIAS_PULL_UP        //!< Pull-up the GPIO
};

/**
 * Events requested for an input GPIO.
 */
enum mygpio_event_request {
    MYGPIO_EVENT_REQUEST_UNKNOWN = -1,  //!< Unknown event request setting
    MYGPIO_EVENT_REQUEST_FALLING,       //!< Request falling events
    MYGPIO_EVENT_REQUEST_RISING,        //!< Request rising events
    MYGPIO_EVENT_REQUEST_BOTH           //!< Request falling and rising events
};

/**
 * Clock setting for an input GPIO.
 */
enum mygpio_event_clock {
    MYGPIO_EVENT_CLOCK_UNKNOWN = -1,  //!< Unknown event clock setting
    MYGPIO_EVENT_CLOCK_MONOTONIC,     //!< Monotonic clock
    MYGPIO_EVENT_CLOCK_REALTIME,      //!< Realtime clock
    MYGPIO_EVENT_CLOCK_HTE            //!< Hardware timestamp engine
};

/**
 * Drive setting for an output GPIO.
 */
enum mygpio_drive {
    MYGPIO_DRIVE_UNKNOWN = -1,  //!< Unknown drive setting
    MYGPIO_DRIVE_PUSH_PULL,     //!< Drive setting is push-pull
    MYGPIO_DRIVE_OPEN_DRAIN,    //!< Drive setting is open-drain
    MYGPIO_DRIVE_OPEN_SOURCE    //!< Drive setting is open-source
};

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
 * Returns true if the GPIO is debounced.
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

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
