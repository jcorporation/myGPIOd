/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/lib/log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//global variables
int loglevel;
bool log_on_tty;
bool log_to_syslog;

/**
 * Maps loglevels to names
 */
static const char *loglevel_names[] = {
    "EMERG",
    "ALERT",
    "CRITICAL",
    "ERROR",
    "WARN",
    "NOTICE",
    "INFO",
    "DEBUG"
};

/**
 * Maps loglevels to terminal colors
 */
static const char *loglevel_colors[] = {
    "\033[0;31m",
    "\033[0;31m",
    "\033[0;31m",
    "\033[0;31m",
    "\033[0;33m",
    "",
    "",
    "\033[0;34m"
};

/**
 * Parses the loglevel name in its enum.
 * Returns the default loglevel on error.
 * @param name string to parse
 * @return loglevel
 */
int parse_loglevel(const char *name) {
    for (int i = 0; i < 8; i++) {
        if (strcasecmp(name, loglevel_names[i]) == 0) {
            return i;
        }
    }
    // default loglevel is INFO
    MYGPIOD_LOG_WARN("Could not parse loglevel, setting default");
    return CFG_SYSLOG;
}

/**
 * Lookups the loglevel name
 * @param level the loglevel
 * @return name
 */
const char *lookup_loglevel(int level) {
    if (level > 0 &&
        level < 8)
    {
        return loglevel_names[level];
    }
    MYGPIOD_LOG_WARN("Could not lookup loglevel");
    return "";
}

/**
 * Sets the loglevel
 * @param level loglevel to set
 */
void set_loglevel(int level) {
    if (level == loglevel) {
        return;
    }
    if (level > 7) {
        level = 7;
    }
    else if (level < 0) {
        level = 0;
    }
    MYGPIOD_LOG_NOTICE("Setting loglevel to %s", loglevel_names[level]);
    loglevel = level;
}

/**
 * Logs the errno string
 * This function should be called by the suitable macro
 * @param level loglevel of the message
 * @param file filename for debug logging
 * @param line linenumber for debug logging
 * @param partition mpd partition
 * @param fmt format string to print
 * @param ... arguments for the format string
 */
void log_log(int level, const char *file, int line, const char *fmt, ...) {
    if (level > loglevel) {
        return;
    }

    if (log_to_syslog == true) {
        va_list args;
        va_start(args, fmt);
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wformat-nonliteral"
        vsyslog(level, fmt, args);
        va_end(args);
        #pragma GCC diagnostic pop
        return;
    }

    if (log_on_tty == 1) {
        printf("%s", loglevel_colors[level]);
        time_t now = time(NULL);
        struct tm timeinfo;
        if (localtime_r(&now, &timeinfo) != NULL) {
            printf("%02d:%02d:%02d ", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        }
    }
    printf("%-8s", loglevel_names[level]);
    #ifdef MYGPIOD_DEBUG
        printf("%s:%d: ", file, line);
    #else
        (void)file;
        (void)line;
    #endif
    va_list args;
    va_start(args, fmt);
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
    vprintf(fmt, args);
    va_end(args);
    #pragma GCC diagnostic pop
    if (log_on_tty == 1) {
        printf("\033[0m");
    }
    printf("\n");
}
