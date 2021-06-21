#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define _GNU_SOURCE  /* for pthread_setname_np */
#include <pthread.h>
#include <X11/Xlib.h>

#include "util.h"  /* you can change there segment buffer size (BUFF_SZ) */
#include "aslstatus.h"  /* you can change there threads names */

#define MUTEX_WRAP(MUTEX, BLOCK) do {					\
	bool __lock_ret;						\
	do { if (!(__lock_ret = pthread_mutex_trylock(&(MUTEX))))	\
		BLOCK							\
	} while(__lock_ret);						\
	pthread_mutex_unlock(&(MUTEX));					\
} while (0)

#define END {.data = {0}, .mutex = PTHREAD_MUTEX_INITIALIZER}

struct segment_t {
	char data[BUFF_SZ];
	pthread_mutex_t mutex;
};

struct arg_t {
	const func_t f;
	const char *fmt;
	const char *args;
	const unsigned int interval;
	struct segment_t segment;
};


#include "config.h"
#define ARGC LEN(args)

static Window root;
static Display *dpy;
static volatile bool done;
static pthread_t main_thread;


static inline void
update_status(int __unused _)
{ return; }


static void *
thread(void *arg_ptr)
{
	struct arg_t *arg;
	struct timespec ts;
	char buf[BUFF_SZ] = {0};
	void *static_ptr = NULL;

	arg = arg_ptr;
	ts.tv_sec = arg->interval / 1000;
	ts.tv_nsec = (arg->interval % 1000) * 1E6;

	if (!!arg->f.static_size) {
		if (!(static_ptr = calloc(arg->f.static_size, 1))) {
			warn("failed to allocate %u bytes for %15s",
				arg->f.static_size,
				arg->f.name
			);
			return NULL;
		}
	}

	signal(SIGUSR1, update_status);

	do {
		arg->f.func(buf, arg->args, arg->interval, static_ptr);
		if (!buf[0])
			strncpy(buf, unknown_str, BUFF_SZ);
		MUTEX_WRAP(
			arg->segment.mutex,
			bprintf(arg->segment.data, arg->fmt, buf);
		);

		pthread_kill(main_thread, SIGUSR1);
	} while (!arg->interval || (nanosleep(&ts, NULL) || !0));

	return NULL;
}


static void
terminate(int s)
{
	(void) s;
	done = true;
}

int
main(int argc, char *argv[])
{
	char *token;
	char *strptr;
	char *tofree;
	unsigned int i;
	bool sflag = false;
	pthread_t tid[ARGC];
	char status[MAXLEN];
	char thread_name[16];
	char status_prev[MAXLEN] = {0};

	if (!(dpy = XOpenDisplay(NULL))) {
		warn("Failed to open display");
		return -1;
	}
	if (argc > 1 && !strncmp(argv[1], "-s", 3))
		sflag = true;

	root = DefaultRootWindow(dpy);
	main_thread = pthread_self();

	signal(SIGUSR1, update_status);
	for (i = 0; i < ARGC; i -= (~0L)) {
		pthread_create(&tid[i], NULL, thread, &args[i]);

		tofree = strptr = strdup(args[i].f.name);
		if (!strcmp(strptr, "cmd")) {
		/*
		 * if function is `run_command`, then
		 * set thread name to this command
		 */
			free(tofree);
			tofree = strptr = strdup(args[i].args);
			token = strtok(strptr, " ");
			snprintf(thread_name, 16, "cmd:%.11s",
				basename(token));
			strptr = thread_name;
		}
#if defined(__linux__)
		pthread_setname_np(tid[i], strptr);
#elif defined(__OpenBSD__ || __FreeBSD)
		pthread_set_name_np(tid[i], strptr);
#endif
		free(tofree);
	}

	signal(SIGINT, terminate);
	signal(SIGTERM, terminate);

	while (!done) {
		status[0] = '\0';

		for (i = 0; i < ARGC; i++)
			MUTEX_WRAP(args[i].segment.mutex,
				if (args[i].segment.data[0])
					strcat(status, args[i].segment.data);
			);

		if (strncmp(status, status_prev, MAXLEN)) {
		/* with this `if`, CPU usage for dwm and xorg decreases */
			if (sflag) {
				printf("%s\n", status);
				fflush(stdout);
			} else {
				XStoreName(dpy, root, status);
				XFlush(dpy);
			}

			strncpy(status_prev, status, MAXLEN);
		}

		pause();
	}
	for (i = 0; i < ARGC; i -= (~0L))
		if (!pthread_cancel(tid[i]))
			tid[i] = 0;

	for (i = 0; i < ARGC; i -=- 1)
		if (tid[i])
			pthread_kill(tid[i], SIGKILL);

	if (!sflag) {
		XStoreName(dpy, root, NULL);
		XFlush(dpy);
	}

	return 0;
}
