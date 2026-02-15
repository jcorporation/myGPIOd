/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd

 This code unit is based on: https://github.com/raspberrypi/utils/blob/master/vcgencmd/vcgencmd.c

 Copyright (c) 2012, Broadcom Europe Ltd
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*! \file
 * \brief Raspberry videocore /dev/vcio
 */

#include "compile_time.h"
#include "mygpiod/raspberry/vcgencmd.h"

#include "dist/sds/sds.h"
#include "mygpiod/lib/log.h"

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

/**
 * Raspberry video core device path
 */
#define DEVICE_FILE_NAME "/dev/vcio"

/**
 * Raspberry video core device major number
 */
#define MAJOR_NUM 100

/**
 * Raspberry video core mbox property
 */
#define IOCTL_MBOX_PROPERTY _IOWR(MAJOR_NUM, 0, char *)

/**
 * Raspberry video core max message length
 */
#define MAX_STRING 1024

/**
 * Use ioctl to send mbox property message
 * @param file_desc 
 * @param buf 
 * @return < 0 on error
 */
static int mbox_property(int file_desc, void *buf) {
    #ifdef HAVE_IOCTL_WITH_INT_REQUEST
        int ret_val = ioctl(file_desc, (int)IOCTL_MBOX_PROPERTY, buf);
    #else
        int ret_val = ioctl(file_desc, IOCTL_MBOX_PROPERTY, buf);
    #endif
    if (ret_val < 0) {
        MYGPIOD_LOG_ERROR("ioctl_set_msg failed:%d", ret_val);
    }
    return ret_val;
}

/**
 * Open a char device file used for communicating with kernel mbox driver
 * @return File descriptor or -1 on error
 */
static int mbox_open(void) {
    int file_desc = open(DEVICE_FILE_NAME, 0);
    if (file_desc < 0) {
        MYGPIOD_LOG_ERROR("Can't open device file: %s", DEVICE_FILE_NAME);
        return -1;
    }
    return file_desc;
}

/**
 * Closes a char device
 * @param file_desc 
 */
static void mbox_close(int file_desc) {
    close(file_desc);
}

/**
 * the tag id
 */
#define GET_GENCMD_RESULT 0x00030080

/**
 * Submits a command to the mbox device
 * @param file_desc Mbox file descriptor
 * @param command Command to submit
 * @param buffer Buffer for the response
 * @return 0 on success
 */
static int gencmd(int file_desc, const char *command, sds *buffer) {
    unsigned i = 0;
    unsigned p[(MAX_STRING>>2) + 7];
    size_t len = strlen(command);
    p[i++] = 0;                 // size
    p[i++] = 0x00000000;        // process request
    p[i++] = GET_GENCMD_RESULT; // the tag id
    p[i++] = MAX_STRING;        // buffer_len
    p[i++] = 0;                 // request_len (set to response length)
    p[i++] = 0;                 // error response

    memcpy(p+i, command, len + 1);
    i += MAX_STRING >> 2;

    p[i++] = 0x00000000; // end tag
    p[0] = i*sizeof *p;  // actual size

    if (mbox_property(file_desc, p) < 0) {
        *buffer = sdscat(*buffer, "ioctl_set_msg failed");
    }
    else {
        *buffer = sdscatlen(*buffer, (const char *)(p+6), (size_t)MAX_STRING);
    }

    return (int)p[5];
}

/**
 * Returns the index of first char occurrence
 * @param buffer 
 * @param c 
 * @return ssize_t 
 */
static ssize_t index_of(sds buffer, char c) {
    char *e = strchr(buffer, c);
    if (e == NULL) {
        return -1;
    }
    return (ssize_t)(e - buffer);
}

/**
 * Reads values from /dev/vcio
 * @param command Command to submit
 * @param buffer Already allocated buffer for the result
 * @param rc Pointer to set return code: true on success, else false
 * @return Pointer to buffer
 */
sds vcgencmd(const char *command, sds buffer, bool *rc) {
    int mb = mbox_open();
    if (mb == -1) {
        *rc = false;
        return sdscatfmt(buffer, "Can't open device file: %s", DEVICE_FILE_NAME);
    }
    int ret = gencmd(mb, command, &buffer);
    if (ret == 0) {
        ssize_t start = index_of(buffer, '=');
        if (start > -1 && start + 1 < (ssize_t)sdslen(buffer)) {
            start++;
            sdsrange(buffer, start, -1);
        }
        *rc = true;
    }
    else {
        MYGPIOD_LOG_ERROR( "vc_gencmd_read_response returned %d", ret);
        *rc = false;
    }
    mbox_close(mb);
    return buffer;
}
