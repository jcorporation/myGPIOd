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
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <gpiod.h>
#include <sys/signalfd.h>

#include "config.h"
#include "log.h"

sig_atomic_t s_signal_received;

struct mon_ctx {
	unsigned int offset;
	unsigned int events_wanted;
	unsigned int events_done;
	bool silent;
	char *fmt;
	int sigfd;
};

struct t_config *config;

static void signal_handler(int sig_num) {
    //Reinstantiate signal handler
    signal(sig_num, signal_handler);
    s_signal_received = sig_num;
    LOG_INFO("Signal %d received, exiting", sig_num);
}

void execute_action(unsigned offset, unsigned event_type) {
    //get cmd
    char *cmd = NULL;
    struct t_config_line *current = config->head;
    while (current != NULL) {
   	if (current->gpio == offset && 
	    (current->edge == GPIOD_CTXLESS_EVENT_BOTH_EDGES || current->edge == event_type))
	{
	    cmd = current->cmd;
	    break;
	}
   	current = current->next;
    }
       
    if (cmd == NULL) {
    	return;
    }

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

static void event_print_human_readable(unsigned int offset,
				       const struct timespec *ts,
				       int event_type)
{
	char *evname;

	if (event_type == GPIOD_CTXLESS_EVENT_CB_RISING_EDGE)
		evname = " RISING EDGE";
	else
		evname = "FALLING EDGE";

	LOG_INFO("event: %s offset: %u timestamp: [%8ld.%09ld]",
	       evname, offset, ts->tv_sec, ts->tv_nsec);
	
	execute_action(offset, event_type);
}

static int poll_callback(unsigned int num_lines,
			 struct gpiod_ctxless_event_poll_fd *fds,
			 const struct timespec *timeout, void *data)
{
	struct pollfd pfds[GPIOD_LINE_BULK_MAX_LINES + 1];
	struct mon_ctx *ctx = data;
	int cnt, ts, rv;
	unsigned int i;

	for (i = 0; i < num_lines; i++) {
		pfds[i].fd = fds[i].fd;
		pfds[i].events = POLLIN | POLLPRI;
	}

	pfds[i].fd = ctx->sigfd;
	pfds[i].events = POLLIN | POLLPRI;

	ts = timeout->tv_sec * 1000 + timeout->tv_nsec / 1000000;

	cnt = poll(pfds, num_lines + 1, ts);
	if (cnt < 0)
		return GPIOD_CTXLESS_EVENT_POLL_RET_ERR;
	else if (cnt == 0)
		return GPIOD_CTXLESS_EVENT_POLL_RET_TIMEOUT;

	rv = cnt;
	for (i = 0; i < num_lines; i++) {
		if (pfds[i].revents) {
			fds[i].event = true;
			if (!--cnt)
				return rv;
		}
	}

	/*
	 * If we're here, then there's a signal pending. No need to read it,
	 * we know we should quit now.
	 */
	close(ctx->sigfd);

	return GPIOD_CTXLESS_EVENT_POLL_RET_STOP;
}

static void handle_event(struct mon_ctx *ctx, int event_type,
			 unsigned int line_offset,
			 const struct timespec *timestamp)
{
        event_print_human_readable(line_offset, timestamp, event_type);
	ctx->events_done++;
}

static int event_callback(int event_type, unsigned int line_offset,
			  const struct timespec *timestamp, void *data)
{
	struct mon_ctx *ctx = data;

	switch (event_type) {
	case GPIOD_CTXLESS_EVENT_CB_RISING_EDGE:
	case GPIOD_CTXLESS_EVENT_CB_FALLING_EDGE:
		handle_event(ctx, event_type, line_offset, timestamp);
		break;
	default:
		/*
		 * REVISIT: This happening would indicate a problem in the
		 * library.
		 */
		return GPIOD_CTXLESS_EVENT_CB_RET_OK;
	}

	if (ctx->events_wanted && ctx->events_done >= ctx->events_wanted)
		return GPIOD_CTXLESS_EVENT_CB_RET_STOP;

	return GPIOD_CTXLESS_EVENT_CB_RET_OK;
}

int make_signalfd(void) {
	sigset_t sigmask;
	int sigfd, rv;

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);
	sigaddset(&sigmask, SIGINT);

	rv = sigprocmask(SIG_BLOCK, &sigmask, NULL);
	if (rv < 0) {
		LOG_ERROR("error masking signals: %s", strerror(errno));
		return -1;
	}

	sigfd = signalfd(-1, &sigmask, 0);
	if (sigfd < 0) {
		LOG_ERROR("error creating signalfd: %s", strerror(errno));
		return -1;
	}

	return sigfd;
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
    config = (struct t_config *) malloc(sizeof(struct t_config));
    config->head = NULL;
    config->tail = NULL;
    config->length = 0;
    config->chip = strdup("0");
    config->active_low = true;
    config->edge = GPIOD_CTXLESS_EVENT_FALLING_EDGE;
    if (read_config(config, config_file) == false) {
        config_free(config);
        free(config);
        free(config_file);
        return 1;
    }

    struct mon_ctx ctx;
    memset(&ctx, 0, sizeof(ctx));
    struct timespec timeout = { 10, 0 };
    unsigned int offsets[GPIOD_LINE_BULK_MAX_LINES];

    struct t_config_line *current = config->head;
    int i = 0;
    while (current != NULL) {
	offsets[i] = current->gpio;
	current=current->next;
	i++;
    }

    ctx.sigfd = make_signalfd();
    if (ctx.sigfd > 0) {
    	int rv = gpiod_ctxless_event_monitor_multiple(
    		config->chip, config->edge,
		offsets, config->length,
		config->active_low, "gpiomon",
		&timeout, poll_callback,
		event_callback, &ctx);
        if (rv) {
	    LOG_ERROR("Error waiting for events");
	}
    }    
    //Cleanup
    config_free(config);
    free(config);
    free(config_file);
    LOG_INFO("Exiting gracefully, thank you for using myGPIOd");
    return 0;
}
