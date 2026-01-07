/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "compile_time.h"
#include "mygpiod/lib/log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/**
 * Global variables
 */

sds logline;  //!< Thread specific log buffer

_Atomic int loglevel;     //!< Loglevel
enum log_types log_type;  //!< Type of logging system

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
static const char *loglevel_colors[8] = {
    [LOG_EMERG] = "\033[0;31m",
    [LOG_ALERT] = "\033[0;31m",
    [LOG_CRIT] = "\033[0;31m",
    [LOG_ERR] = "\033[0;31m",
    [LOG_WARNING] = "\033[0;33m",
    [LOG_NOTICE] = "",
    [LOG_INFO] = "",
    [LOG_DEBUG] = "\033[0;32m"
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
 * Initializes the logging sub-system
 */
void log_init(void) {
    if (isatty(fileno(stdout)) == true) {
        log_type = LOG_TO_TTY;
    }
    else if (getenv("INVOCATION_ID") != NULL) {
        log_type = LOG_TO_SYSTEMD;
    }
    else if (getenv("MYGPIOD_LOG_TS") != NULL) {
        log_type = LOG_WITH_TS;
    }
    else {
        log_type = LOG_TO_STDOUT;
    }
    #ifdef MYGPIOD_DEBUG
        set_loglevel(LOG_DEBUG);
    #endif
}

/**
 * Logs the errno string
 * This function should be called by the suitable macro
 * @param file filename for debug logging
 * @param line linenumber for debug logging
 * @param errnum errno
 */
void mygpiod_log_errno(const char *file, int line, int errnum) {
    if (errnum == 0) {
        //do not log success
        return;
    }
    char err_text[256];
    int rc = strerror_r(errnum, err_text, 256);
    const char *err_str = rc == 0
        ? err_text
        : "Unknown error";
    mygpiod_log(LOG_ERR, file, line, "%s", err_str);
}

/**
 * Logging function
 * This function should be called by the suitable macro
 * @param level loglevel of the message
 * @param file filename for debug logging
 * @param line linenumber for debug logging
 * @param fmt format string to print
 * @param ... arguments for the format string
 */
void mygpiod_log(int level, const char *file, int line, const char *fmt, ...) {
    if (level > loglevel) {
        return;
    }

    if (log_type == LOG_TO_SYSLOG) {
        va_list args;
        va_start(args, fmt);
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wformat-nonliteral"
            vsyslog(level, fmt, args); // NOLINT(clang-diagnostic-format-nonliteral)
        #pragma GCC diagnostic pop
        va_end(args);
        return;
    }

    if (log_type == LOG_TO_TTY) {
        logline = sdscat(logline, loglevel_colors[level]);
        time_t now = time(NULL);
        struct tm timeinfo;
        if (localtime_r(&now, &timeinfo) != NULL) {
            logline = sdscatprintf(logline, "%02d:%02d:%02d ", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        }
    }
    else if (log_type == LOG_WITH_TS) {
        time_t now = time(NULL);
        struct tm timeinfo;
        if (localtime_r(&now, &timeinfo) != NULL) {
            logline = sdscatprintf(logline, "%02d:%02d:%02d ", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        }
    }
    else if (log_type == LOG_TO_SYSTEMD) {
        logline = sdscatfmt(logline, "<%i>", level);
    }
    logline = sdscatprintf(logline, "%-8s", loglevel_names[level]);
    #ifdef MYMPD_DEBUG
        logline = sdscatfmt(logline, "%s:%i: ", file, line);
    #else
        (void)file;
        (void)line;
    #endif

    va_list args;
    va_start(args, fmt);
    logline = sdscatvprintf(logline, fmt, args);
    va_end(args);

    if (sdslen(logline) > 1023) {
        sdsrange(logline, 0, 1020);
        logline = sdscatlen(logline, "...", 3);
    }

    sdstrim(logline, "\n");
    if (log_type == LOG_TO_TTY) {
        logline = sdscat(logline, "\033[0m\n");
    }
    else {
        logline = sdscatlen(logline, "\n", 1);
    }

    (void) fputs(logline, stdout);
    sdsclear(logline);
}
