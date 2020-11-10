/*
 SPDX-License-Identifier: GPL-2.0-or-later
 myMPD (c) 2018-2020 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef LOG_H
#define LOG_H

enum { LOGLEVEL_ERROR, LOGLEVEL_WARN, LOGLEVEL_INFO, LOGLEVEL_VERBOSE, LOGLEVEL_DEBUG };

#define LOG_ERROR(...) log_log(LOGLEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) log_log(LOGLEVEL_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) log_log(LOGLEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_VERBOSE(...) log_log(LOGLEVEL_VERBOSE, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) log_log(LOGLEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)

extern int loglevel;
extern int log_on_tty;

void log_log(int level, const char *file, int line, const char *fmt, ...);
void set_loglevel(int level);

#endif
