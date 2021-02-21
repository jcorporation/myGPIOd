/*
 SPDX-License-Identifier: GPL-2.0-or-later
 myMPD (c) 2020-2021 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LOG_H
#define LOG_H

#include <stdbool.h>
#include <syslog.h>

#define MYGPIOD_LOG_EMERG(...) log_log(LOG_EMERG, __FILE__, __LINE__, __VA_ARGS__)
#define MYGPIOD_LOG_ALERT(...) log_log(LOG_ALERT, __FILE__, __LINE__, __VA_ARGS__)
#define MYGPIOD_LOG_CRIT(...) log_log(LOG_CRIT, __FILE__, __LINE__, __VA_ARGS__)
#define MYGPIOD_LOG_ERROR(...) log_log(LOG_ERR, __FILE__, __LINE__, __VA_ARGS__)
#define MYGPIOD_LOG_WARN(...) log_log(LOG_WARNING,  __FILE__, __LINE__, __VA_ARGS__)
#define MYGPIOD_LOG_NOTICE(...) log_log(LOG_NOTICE, __FILE__, __LINE__, __VA_ARGS__)
#define MYGPIOD_LOG_INFO(...) log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define MYGPIOD_LOG_DEBUG(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)

extern int loglevel;
extern bool log_on_tty;
extern bool log_to_syslog;

void log_log(int level, const char *file, int line, const char *fmt, ...);
void set_loglevel(int level);

#endif
