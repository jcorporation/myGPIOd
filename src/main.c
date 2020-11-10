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
#include <errno.h>

#include "config.h"
#include "log.h"

sig_atomic_t s_signal_received;

static void signal_handler(int sig_num) {
    //Reinstantiate signal handler
    signal(sig_num, signal_handler);
    s_signal_received = sig_num;
    LOG_INFO("Signal %d received, exiting", sig_num);
}

void consume_value(int fd) {
    char buf[8];
    lseek(fd, 0, SEEK_SET);
    ssize_t n = read(fd, buf, sizeof buf);
    if (n < 0) {
        LOG_ERROR("Error reading fd");
    }
    LOG_DEBUG("Consumed value %s", buf);
}

bool export_gpio(unsigned gpio) {
    LOG_INFO("Exporting GPIO %u", gpio);
    FILE *fp = fopen(GPIO_PATH"export", "w");
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
    snprintf(gpio_file, 80, GPIO_PATH"gpio%u/value", gpio);
    FILE *fp = fopen(gpio_file, "r");
    if (fp == NULL) {
        return false;
    }
    fclose(fp);
    return true;
}

bool set_gpio_mode(unsigned gpio, const char *key, const char *value) {
    LOG_INFO("Setting gpio %u, %s to %s", gpio, key, value);
    char gpio_file[80];
    snprintf(gpio_file, 80, GPIO_PATH"gpio%u/%s", gpio, key);
    FILE *fp = fopen(gpio_file, "w");
    if (fp != NULL) {
        int rc = fputs(value, fp);
        fclose(fp);
        if (rc > 0) {
            return true;
        }
    }
    else {
        LOG_ERROR("Error opening %s: %s", gpio_file, strerror(errno));
    }
    LOG_ERROR("Error setting gpio %u, %s", gpio, key);
    return false;
}

void execute_action(const char *cmd) {
    if (fork() == 0) {
        //child process executes cmd
        int rc = system(cmd);
        if (rc == -1) {
            LOG_ERROR("Error executing cmd \"%s\": %s", cmd, strerror(errno));
        }
        exit(0);
    }
    //parent process returns to main loop
}

int main(int argc, char **argv) {
    log_on_tty = isatty(fileno(stdout)) ? 1: 0;
    #ifdef DEBUG
    set_loglevel(4);
    #else
    set_loglevel(2);
    #endif
    
    LOG_INFO("Starting myGPIOd %s", MYGPIOD_VERSION);

    //set signal handler
    s_signal_received = 0;
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    //handle commandline parameter
    char *config_file = NULL;
    if (argc == 2 && strncmp(argv[1], "/", 1) == 0) {
        config_file = strdup(argv[1]);
    }
    else {
        config_file = strdup("/etc/mygpiod.conf");
    }

    //read configuration
    LOG_INFO("Reading %s", config_file);
    struct t_config *config = (struct t_config *) malloc(sizeof(struct t_config));
    config->head = NULL;
    config->tail = NULL;
    config->length = 0;
    if (read_config(config, config_file) == false) {
        config_free(config);
        free(config);
        free(config_file);
        return 1;
    }
    
    //Set GPIOs
    struct pollfd ufds[MAX_GPIO] = {{0}};
    memset(ufds, 0, sizeof(struct pollfd) * 40);
    unsigned fd_num = 0;
    
    struct t_config_line *current = config->head;
    while (current != NULL) {
        LOG_INFO("Setting GPIO %u:", current->gpio);
        if (check_gpio_export(current->gpio) == false) {
            if (export_gpio(current->gpio) == false) {
                LOG_ERROR("Error exporting GPIO %u", current->gpio);        
            }
        }
        if (check_gpio_export(current->gpio) == true) {
            set_gpio_mode(current->gpio, "direction", current->direction);
            if (strcmp(current->direction, "in") == 0) {
                set_gpio_mode(current->gpio, "edge", current->edge);
                set_gpio_mode(current->gpio, "active_low", current->active_low);
                //open file descriptor
                if (fd_num < MAX_GPIO) {
                    char gpio_file[80];
                    snprintf(gpio_file, 80, GPIO_PATH"gpio%u/value", current->gpio);
                    ufds[fd_num].fd = open(gpio_file, O_RDONLY, S_IREAD);
                    ufds[fd_num].events = POLLPRI | POLLERR;
                    if (ufds[fd_num].fd > 0) {
                        current->fd = ufds[fd_num].fd;
                        fd_num++;
                    }
                    else {
                        LOG_ERROR("Error opening fd for GPIO %u", current->gpio);
                    }
                }
                else {
                    LOG_WARN("Skipping GPIO - to many open fds (maximum of 40)");
                }
            }
        }
        current = current->next;
    }
    
    if (fd_num == 0) {
        LOG_WARN("No GPIOs configures for direction in, exiting...");
        goto cleanup;
    }

    //Read old values
    for (unsigned i = 0; i < fd_num; i++) {
        consume_value(ufds[i].fd);    
    }
    //Main loop    
    LOG_INFO("Listening on GPIO events");
    while (s_signal_received == 0) {
        int read_fds = poll(ufds, 1, -1);
        if (read_fds < 0) {
            LOG_ERROR("Error polling fds");
            break;
        }
        for (unsigned i = 0; i < fd_num; i++) {
            if (ufds[0].revents & POLLPRI) {
                current = get_config_from_fd(config, ufds[0].fd);
                LOG_INFO("Event for GPIO %u detected", current->gpio);
                execute_action(current->cmd);
                consume_value(ufds[0].fd);        
            }
        }
    }
    //Cleanup
    cleanup:
    config_free(config);
    free(config);
    free(config_file);
    for (unsigned i = 0; i < fd_num; i++) {
        close(ufds[i].fd);    
    }
    LOG_INFO("Exiting gracefully, thank you for using myGPIOd");
    return 0;
}
