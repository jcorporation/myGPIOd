/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "action.h"

#include "config.h"
#include "event.h"
#include "log.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

/**
 * Executes the configured action for an event.
 * It forks and executes the script via system() call
 * @param offset the gpio number
 * @param ts timestamp of the event
 * @param event_type the event type
 * @param config pointer to myGPIOd config
 */
void action_execute(unsigned offset, const struct timespec *ts, int event_type, struct t_config *config) {
    MYGPIOD_LOG_INFO("Event: \"%s\" gpio: \"%u\" timestamp: \"[%8lld.%09ld]\"",
        (event_type == GPIOD_CTXLESS_EVENT_CB_RISING_EDGE ? " RISING EDGE" : "FALLING EDGE"), 
        offset, (long long)ts->tv_sec, ts->tv_nsec);

    //map GPIOD_CTXLESS_EVENT_CB_* to GPIOD_CTXLESS_EVENT_*
    if (event_type == GPIOD_CTXLESS_EVENT_CB_FALLING_EDGE) {
        event_type = GPIOD_CTXLESS_EVENT_FALLING_EDGE;
    }
    else if (event_type == GPIOD_CTXLESS_EVENT_CB_RISING_EDGE) {
        event_type = GPIOD_CTXLESS_EVENT_RISING_EDGE;
    }
    
    //get cmd
    char *cmd = NULL;
    long last_execution = 0;
    struct t_config_node *current = config->head;
    while (current != NULL) {
       if (current->gpio == offset && 
            (current->edge == GPIOD_CTXLESS_EVENT_BOTH_EDGES || current->edge == event_type))
        {
            cmd = current->cmd;
            last_execution = current->last_execution;
            current->last_execution = ts->tv_sec;
            break;
        }
       current = current->next;
    }

    if (current == NULL) {
        return;
    }
    //prevent multiple execution of cmds within two seconds
    if (last_execution >= ts->tv_sec - 2) {
        return;
    }

    MYGPIOD_LOG_INFO("Executing \"%s\"", cmd);
    if (fork() == 0) {
        //child process executes cmd
        errno = 0;
        int rc = system(cmd); /* Flawfinder: ignore */
        if (rc == -1) {
            MYGPIOD_LOG_ERROR("Error executing cmd \"%s\": %s", cmd, strerror(errno));
        }
        exit(0);
    }
    //parent process returns to main loop
}
