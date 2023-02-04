/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LOG_H
#define LOG_H

#include <stdbool.h>
#include <syslog.h>

/**
 * Macros for logging
 */
#ifdef MYGPIOD_DEBUG
    #define MYGPIOD_LOG_EMERG(...) log_log(LOG_EMERG, __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_ALERT(...) log_log(LOG_ALERT, __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_CRIT(...) log_log(LOG_CRIT, __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_ERROR(...) log_log(LOG_ERR, __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_WARN(...) log_log(LOG_WARNING,  __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_NOTICE(...) log_log(LOG_NOTICE, __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_INFO(...) log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_DEBUG(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_ERRNO(ERRNUM) log_log_errno(__FILE__, __LINE__, ERRNUM)
#else
    /**
     * release build should have no references to build dir
     */
    #define MYGPIOD_LOG_EMERG(...) log_log(LOG_EMERG, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_ALERT(...) log_log(LOG_ALERT, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_CRIT(...) log_log(LOG_CRIT, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_ERROR(...) log_log(LOG_ERR, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_WARN(...) log_log(LOG_WARNING, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_NOTICE(...) log_log(LOG_NOTICE, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_INFO(...) log_log(LOG_INFO, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_DEBUG(...) log_log(LOG_DEBUG, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_ERRNO(ERRNUM) log_log_errno("", __LINE__, ERRNUM)
#endif

extern int loglevel;
extern bool log_on_tty;
extern bool log_to_syslog;

void log_log(int level, const char *file, int line, const char *fmt, ...)
    __attribute__ ((format (printf, 4, 5))); /* Flawfinder: ignore */
void set_loglevel(int level);

#endif
