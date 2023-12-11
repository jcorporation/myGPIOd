/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "compile_time.h"
#include "mygpiod/actions/system.h"

#include "mygpiod/lib/log.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Runs an executable or script in a new process
 * @param cmd command to execute
 */
bool action_system(const char *cmd) {
    errno = 0;
    int rc = fork();
    if (rc == 0) {
        // this is the child process
        errno = 0;
        execl(cmd, cmd, (char *)NULL);
        // successful execl call does not return
        MYGPIOD_LOG_ERROR("Error executing action \"%s\": %s", cmd, strerror(errno));
        exit(EXIT_FAILURE);
    }
    else {
        if (rc == -1) {
            MYGPIOD_LOG_ERROR("Could not fork: %s", strerror(errno));
            return false;
        }
        MYGPIOD_LOG_DEBUG("Forked process with id %d", rc);
        return true;
    }
}
