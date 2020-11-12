/*
 SPDX-License-Identifier: GPL-2.0-or-later
 myGPIOd (c) 2020 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "log.h"

int loglevel;
int log_on_tty;

static const char *loglevel_names[] = {
  "ERROR", "WARN", "INFO", "VERBOSE", "DEBUG"
};

static const char *loglevel_colors[] = {
  "\033[0;31m", "\033[0;33m", "", "", "\033[0;34m"
};

void set_loglevel(int level) {
    if (level > 4) {
        level = 4;
    }
    else if (level < 0) {
        level = 0;
    }
    LOG_INFO("Setting loglevel to %s", loglevel_names[level]);
    loglevel = level;
}

void log_log(int level, const char *file, int line, const char *fmt, ...) {
    if (level > loglevel) {
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
    if (loglevel == 4) {
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
