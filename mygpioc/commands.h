/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOC_COMMANDS_H
#define MYGPIOC_COMMANDS_H

enum cmds {
    CMD_INVALID,
    CMD_COUNT
};

enum cmds check_command(const char *command);

#endif
