/*
 SPDX-License-Identifier: GPL-2.0-or-later
 myGPIOd (c) 2020-2021 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int loglevel;
bool log_on_tty;
bool log_to_syslog;

static const char *loglevel_names[] = {
  "EMERG", "ALERT", "CRITICAL", "ERROR", "WARN", "NOTICE", "INFO", "DEBUG"
};

static const char *loglevel_colors[] = {
  "\033[0;31m", "\033[0;31m", "\033[0;31m", "\033[0;31m", "\033[0;33m", "", "", "\033[0;34m"
};

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

void log_log(int level, const char *file, int line, const char *fmt, ...) {
    if (level > loglevel) {
        return;
    }
    
    if (log_to_syslog == true) {
        va_list args;
        va_start(args, fmt);
        vsyslog(level, fmt, args);
        va_end(args);
        return;
    }
    
    if (log_on_tty == 1) {
        printf("%s", loglevel_colors[level]);
    }
    if (log_on_tty == 1) {
        time_t now = time(NULL);
        struct tm timeinfo;
        if (localtime_r(&now, &timeinfo) != NULL) {
            printf("%02d:%02d:%02d ", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        }
    }
    printf("%-8s", loglevel_names[level]);
    if (loglevel == 7) {
        printf("%s:%d: ", file, line);
    }
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    if (log_on_tty == 1) {
        printf("\033[0m");
    }
}
