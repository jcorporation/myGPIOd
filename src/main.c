/*
 SPDX-License-Identifier: GPL-2.0-or-later
 myGPIOd (c)2020 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd

 myGPIOd is based on the gpiomon tool from https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/about/
*/

#include "config.h"
#include "log.h"

#include <errno.h>
#include <gpiod.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/signalfd.h>
#include <unistd.h>

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

static void execute_action(unsigned int offset, const struct timespec *ts, int event_type) {
    LOG_INFO("Event: \"%s\" offset: \"%u\" timestamp: \"[%8ld.%09ld]\"",
    	(event_type == GPIOD_CTXLESS_EVENT_CB_RISING_EDGE ? " RISING EDGE" : "FALLING EDGE"), 
    	offset, ts->tv_sec, ts->tv_nsec);

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
    struct t_config_line *current = config->head;
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

    LOG_INFO("Executing \"%s\"", cmd);
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

static int poll_callback(unsigned int num_lines, struct gpiod_ctxless_event_poll_fd *fds, const struct timespec *timeout, void *data) {
	struct pollfd pfds[GPIOD_LINE_BULK_MAX_LINES + 1];
	struct mon_ctx *ctx = data;
	unsigned int i;

	for (i = 0; i < num_lines; i++) {
		pfds[i].fd = fds[i].fd;
		pfds[i].events = POLLIN | POLLPRI;
	}

	pfds[i].fd = ctx->sigfd;
	pfds[i].events = POLLIN | POLLPRI;

	long ts = timeout->tv_sec * 1000 + timeout->tv_nsec / 1000000;

	int cnt = poll(pfds, num_lines + 1, (int)ts);
	if (cnt < 0) {
		return GPIOD_CTXLESS_EVENT_POLL_RET_ERR;
	}
	if (cnt == 0) {
		return GPIOD_CTXLESS_EVENT_POLL_RET_TIMEOUT;
	}

	int rv = cnt;
	for (i = 0; i < num_lines; i++) {
		if (pfds[i].revents) {
			fds[i].event = true;
			if (!--cnt) {
				return rv;
			}
		}
	}

	/*
	 * If we're here, then there's a signal pending. No need to read it,
	 * we know we should quit now.
	 */
	close(ctx->sigfd);

	return GPIOD_CTXLESS_EVENT_POLL_RET_STOP;
}

static void handle_event(struct mon_ctx *ctx, int event_type, unsigned int line_offset, const struct timespec *timestamp) {
  	execute_action(line_offset, timestamp, event_type);
	ctx->events_done++;
}

static int event_callback(int event_type, unsigned int line_offset, const struct timespec *timestamp, void *data) {
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

	if (ctx->events_wanted && ctx->events_done >= ctx->events_wanted) {
		return GPIOD_CTXLESS_EVENT_CB_RET_STOP;
	}

	return GPIOD_CTXLESS_EVENT_CB_RET_OK;
}

static int make_signalfd(void) {
	sigset_t sigmask;
	int sigfd, rv;

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGTERM);
	sigaddset(&sigmask, SIGINT);

	rv = sigprocmask(SIG_BLOCK, &sigmask, NULL);
	if (rv < 0) {
		LOG_ERROR("Error masking signals: \"%s\"", strerror(errno));
		return -1;
	}

	sigfd = signalfd(-1, &sigmask, 0);
	if (sigfd < 0) {
		LOG_ERROR("Error creating signalfd: \"%s\"", strerror(errno));
		return -1;
	}

	return sigfd;
}

static int flush_events(void) {
	struct gpiod_line_bulk input_bulk;
	gpiod_line_bulk_init(&input_bulk);
	struct timespec timeout = { 1, 0 };
	struct t_config_line *current = config->head;
    unsigned int i = 0;
	struct gpiod_chip *chip = gpiod_chip_open(config->chip);
	if (chip == NULL) {
		return 1;
	}
    while (current != NULL) {
		struct gpiod_line *line = gpiod_chip_get_line(chip, current->gpio);
		gpiod_line_bulk_add(&input_bulk, line);
		current=current->next;
		i++;
    }
	int ret = 0;
	do {
		struct gpiod_line_bulk bulk_evt;
		gpiod_line_bulk_init(&bulk_evt);
		ret = gpiod_line_event_wait_bulk(&input_bulk, &timeout, &bulk_evt);
		if (ret < 0) {
			LOG_ERROR("Could not flush pending events: %s", strerror(errno));
			return ret;
		}
		else if (ret == 1) {
			struct gpiod_line *line;
			struct gpiod_line **line_ptr;
			struct gpiod_line_event evt;
			gpiod_line_bulk_foreach_line(&bulk_evt, line, line_ptr) {
				LOG_DEBUG("Read pending event from line %u", gpiod_line_offset(line));
				gpiod_line_event_read(line, &evt);
			}
		}
	} while (ret == 1);
	return 0;
}

int main(int argc, char **argv) {
    int rc = EXIT_SUCCESS;
    log_on_tty = isatty(fileno(stdout)) ? 1: 0;
    #ifdef DEBUG
    set_loglevel(4);
    #else
    set_loglevel(2);
    #endif

    LOG_INFO("Starting myGPIOd %s", MYGPIOD_VERSION);
    LOG_INFO("https://github.com/jcorporation/myGPIOd");

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
    LOG_INFO("Reading \"%s\"", config_file);
    config = (struct t_config *) malloc(sizeof(struct t_config));
    config->head = NULL;
    config->tail = NULL;
    config->length = 0;
    config->chip = strdup("0");
    config->active_low = true;
    config->edge = GPIOD_CTXLESS_EVENT_FALLING_EDGE;
    config->loglevel = loglevel;
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
    unsigned int num_lines = 0;
    while (current != NULL) {
		offsets[num_lines] = current->gpio;
		current=current->next;
		num_lines++;
    }

    ctx.sigfd = make_signalfd();
    if (ctx.sigfd > 0) {
		//get pending events and ignore it
		LOG_INFO("Flushing gpio events");
		flush_events();
		//main event handling loop
		LOG_INFO("Entering event handling loop");
    	int rv = gpiod_ctxless_event_monitor_multiple(
    		config->chip, config->edge, offsets, num_lines,
			config->active_low, "gpiomon", &timeout, poll_callback,
			event_callback, &ctx);
        if (rv) {
	    	LOG_ERROR("Error waiting for events");
	    	rc = EXIT_FAILURE;
		}
    }    
    //Cleanup
    config_free(config);
    free(config);
    free(config_file);
    if (rc == EXIT_SUCCESS) {
	    LOG_INFO("Exiting gracefully, thank you for using myGPIOd");
	}
    return rc;
}
