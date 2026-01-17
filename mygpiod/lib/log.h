/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mygpiod
*/

#ifndef MYGPIOD_LOG_H
#define MYGPIOD_LOG_H

#include "dist/sds/sds.h"

#include <stdbool.h>
#include <syslog.h>

enum log_types {
    LOG_TO_SYSLOG,
    LOG_TO_TTY,
    LOG_TO_SYSTEMD,
    LOG_TO_STDOUT,
    LOG_WITH_TS
};

/**
 * Macros for logging
 */
#ifdef MYGPIOD_DEBUG
    #define MYGPIOD_LOG_EMERG(...) mygpiod_log(LOG_EMERG, __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_ALERT(...) mygpiod_log(LOG_ALERT, __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_CRIT(...) mygpiod_log(LOG_CRIT, __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_ERROR(...) mygpiod_log(LOG_ERR, __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_WARN(...) mygpiod_log(LOG_WARNING,  __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_NOTICE(...) mygpiod_log(LOG_NOTICE, __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_INFO(...) mygpiod_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_DEBUG(...) mygpiod_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_ERRNO(ERRNUM) mygpiod_log_errno(__FILE__, __LINE__, ERRNUM)
#else
    /**
     * release build should have no references to build dir
     */
    #define MYGPIOD_LOG_EMERG(...) mygpiod_log(LOG_EMERG, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_ALERT(...) mygpiod_log(LOG_ALERT, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_CRIT(...) mygpiod_log(LOG_CRIT, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_ERROR(...) mygpiod_log(LOG_ERR, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_WARN(...) mygpiod_log(LOG_WARNING, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_NOTICE(...) mygpiod_log(LOG_NOTICE, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_INFO(...) mygpiod_log(LOG_INFO, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_DEBUG(...) mygpiod_log(LOG_DEBUG, "", __LINE__, __VA_ARGS__)
    #define MYGPIOD_LOG_ERRNO(ERRNUM) mygpiod_log_errno("", __LINE__, ERRNUM)
#endif

/**
 * Global log variables
 */
extern _Atomic int loglevel;
extern enum log_types log_type;

extern sds logline;

const char *get_loglevel_name(int level);
void set_loglevel(int level);
void log_init(void);

int parse_loglevel(const char *name);
const char *lookup_loglevel(int level);

void mygpiod_log_errno(const char *file, int line, int errnum);
void mygpiod_log(int level, const char *file, int line, const char *fmt, ...)
    __attribute__ ((format (printf, 4, 5)));

#endif
