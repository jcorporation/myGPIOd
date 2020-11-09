/*
 SPDX-License-Identifier: GPL-2.0-or-later
 myGPIOd (c)2020 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <signal.h>

#include "config.h"

sig_atomic_t s_signal_received;

static void signal_handler(int sig_num) {
    signal(sig_num, signal_handler);  // Reinstantiate signal handler
    s_signal_received = sig_num;
    printf("Signal %d received, exiting", sig_num);
}

void consume_value(int fd) {
    char buf[8];
    lseek(fd, 0, SEEK_SET);
    ssize_t n = read(fd, buf, sizeof buf);
    if (n < 0) {
        fprintf(stderr, "Error reading fd\n");
    }    
}

bool export_gpio(unsigned gpio) {
    printf("\tExporting GPIO %u\n", gpio);
    FILE *fp = fopen("/sys/class/gpio/export", "w");
    if (fp == NULL) {
        return false;
    }
    int n = fprintf(fp, "%u", gpio);
    fclose(fp);
    if (n != 1) {
        return false;
    }
    return true;
}

bool check_gpio_export(unsigned gpio) {
    char gpio_file[80];
    snprintf(gpio_file, 80, "/sys/class/gpio/gpio%u/value", gpio);
    FILE *fp = fopen(gpio_file, "r");
    if (fp == NULL) {
        return false;
    }
    fclose(fp);
    return true;
}

bool set_gpio_mode(unsigned gpio, const char *key, const char *value) {
    printf("\tSetting %s to %s\n", key, value);
    char gpio_file[80];
    snprintf(gpio_file, 80, "/sys/class/gpio/gpio%u/%s", gpio, key);
    FILE *fp = fopen(gpio_file, "w");
    if (fputs(value, fp) > 0) {
        return true;
    }
    printf("\tError setting %s\n", key);
    return false;
}

int main(int argc, char **argv) {
    //set signal handler
    s_signal_received = 0;
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);


    printf("Starting myGPIOd %s\n", MYGPIOD_VERSION);
    //todo: handle commandline parameters
    (void) argc;
    (void) argv;
    
    //read configuration
    printf("Reading /etc/mygpiod.conf\n");
    struct t_config *config = (struct t_config *) malloc(sizeof(struct t_config));
    config->head = NULL;
    config->tail = NULL;
    config->length = 0;
    if (read_config(config) == false) {
        fprintf(stderr, "Error reading /etc/mymgpiod.conf\n");
        config_free(config);
        return 1;
    }
    
    //Set GPIOs
    struct pollfd ufds[40] = {{0}};
    memset(ufds, 0, sizeof(struct pollfd) * 40);
    unsigned fd_num = 0;
    
    struct t_config_line *current = config->head;
    while (current != NULL) {
        printf("Setting GPIO %u:\n", current->gpio);
        if (check_gpio_export(current->gpio) == false) {
            if (export_gpio(current->gpio) == false) {
                fprintf(stderr, "\tError exporting GPIO %u\n", current->gpio);        
            }
        }
        if (check_gpio_export(current->gpio) == true) {
            set_gpio_mode(current->gpio, "direction", current->direction);
            if (strcmp(current->direction, "in") == 0) {
                set_gpio_mode(current->gpio, "edge", current->edge);
                set_gpio_mode(current->gpio, "active_low", current->active_low);
                //open file descriptor
                if (fd_num <= 40) {
                    char gpio_file[80];
                    snprintf(gpio_file, 80, "/sys/class/gpio/gpio%u/value", current->gpio);
                    ufds[fd_num].fd = open(gpio_file, O_RDONLY, S_IREAD);
                    ufds[fd_num].events = POLLPRI | POLLERR;
                    if (ufds[fd_num].fd > 0) {
                        current->fd = ufds[fd_num].fd;
                        fd_num++;
                    }
                    else {
                        fprintf(stderr, "\tError opening fd for GPIO %u\n", current->gpio);
                    }
                }
                else {
                    fprintf(stderr, "\tSkipping GPIO - to many open fds (maximum of 40)\n");
                }
            }
        }
        current = current->next;
    }
    
    if (fd_num == 0) {
        goto cleanup;
    }

    //Read old values
    for (unsigned i = 0; i < fd_num; i++) {
        consume_value(ufds[i].fd);    
    }
    //Main loop    
    printf("Listening on GPIO events\n");
    while (s_signal_received == 0) {
        int read_fds = poll(ufds, 1, -1);
        if (read_fds < 0) {
            printf("Error polling fds\n");
            break;
        }
        for (unsigned i = 0; i < fd_num; i++) {
            if (ufds[0].revents & POLLPRI) {
                current = get_config_from_fd(config, ufds[0].fd);
                printf("Event for GPIO %u detected\n", current->gpio);
                consume_value(ufds[0].fd);        
            }
        }
    }
    //Cleanup
    cleanup:
    config_free(config);
    for (unsigned i = 0; i < fd_num; i++) {
        close(ufds[i].fd);    
    }
    printf("Exiting gracefully, thank you for using myGPIOd\n");
    return 0;
}
