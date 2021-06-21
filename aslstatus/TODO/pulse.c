#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pthread.h>
#include <pulse/pulseaudio.h>

#define MUTED "muted"
#define LEN(S) (sizeof(S) / sizeof(*S))

static char *server = NULL;
static pa_context *context = NULL;
static pa_proplist *proplist = NULL;
static pa_mainloop_api *mainloop_api = NULL;

static pthread_t updater = NULL;
static volatile bool done = false;
static char volume[LEN(MUTED) > 3 ? LEN(MUTED) :3] = {0};

void context_state_callback(pa_context *, void *);

static inline void
update_volume(int s)
{ (void) s; }

static inline void
quit(int ret)
{
	assert(mainloop_api);
	mainloop_api->quit(mainloop_api, ret);
}


static inline void
exit_signal_callback(pa_mainloop_api *m, pa_signal_event *e, int sig,
		void *userdata)
{
	fprintf(stderr, "Got signal: %d, exiting.\n", sig);
	quit(0);
}


static inline void
context_subscribe_callback(pa_context *c, pa_subscription_event_type_t t,
		uint32_t idx, void *userdata)
{
	assert(c);
	/* printf("event %x\n", idx); */

	if (!(context = pa_context_new_with_proplist(mainloop_api, NULL, proplist))) {
		fprintf(stderr, "pa_context_new() failed.");
		return;
	}
	pa_context_set_state_callback(context, context_state_callback, NULL);
	if (pa_context_connect(context, server, 0, NULL) < 0) {
		fprintf(stderr, "pa_context_connect() failed: %s",
				pa_strerror(pa_context_errno(context)));
		return;
	}
}


static inline void
get_sink_info_callback(pa_context *c, const pa_sink_info *i, int is_last,
		void *userdata)
{
	if (is_last < 0) {
		fprintf(stderr, "Failed to get sink information: %s",
				pa_strerror(pa_context_errno(c)));
		quit(1);
	}

	if (is_last)
		return;
	if (volume[0])
		pa_context_unref(c);

	if (i->mute)
		strncpy(volume, MUTED, sizeof(MUTED));
	else
		snprintf(volume, 4, "%hhu", (unsigned short)
			(((uint64_t)i->volume.values[0] * 100 +
			  (uint64_t)PA_VOLUME_NORM / 2) /
			  (uint64_t)PA_VOLUME_NORM));
	fflush(stdout);

	pthread_kill(updater, SIGUSR1);
}


void
context_state_callback(pa_context *c, void *userdata)
{
	pa_operation *o = NULL;
	static bool is_first = true;

	assert(c);

	switch (pa_context_get_state(c)) {
	case PA_CONTEXT_CONNECTING:
	case PA_CONTEXT_AUTHORIZING:
	case PA_CONTEXT_SETTING_NAME:
		return;
	case PA_CONTEXT_READY:
		if (is_first) {
			is_first = false;
			pa_context_set_subscribe_callback(c, context_subscribe_callback, NULL);
			if ((o = pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, NULL, NULL)))
				pa_operation_unref(o);
		}
		if ((o = pa_context_get_sink_info_list(c, get_sink_info_callback, NULL)))
			pa_operation_unref(o);
		return;
	case PA_CONTEXT_TERMINATED:
		puts("term");
		quit(0);
		return;
	case PA_CONTEXT_FAILED:
		puts("failed");
	default:
		puts("end");
		fprintf(stderr, "Connection failure: %s", pa_strerror(pa_context_errno(c)));
		quit(1);
	}
}




void *
mainloop_thread(void *ptr)
{
	int ret = 1;
	pa_mainloop *m = NULL;

	if (!(m = pa_mainloop_new())) {
		fprintf(stderr, "pa_mainloop_new() failed.");
		goto quit;
	}

	mainloop_api = pa_mainloop_get_api(m);

	assert(pa_signal_init(mainloop_api) == 0);
	pa_signal_new(SIGINT, exit_signal_callback, NULL);
	pa_signal_new(SIGTERM, exit_signal_callback, NULL);

	proplist = pa_proplist_new();
	if (!(context = pa_context_new_with_proplist(mainloop_api, NULL,
							proplist))) {
		fprintf(stderr, "pa_context_new() failed.");
		goto quit;
	}


	pa_context_set_state_callback(context, context_state_callback, NULL);
	if (pa_context_connect(context, server, 0, NULL) < 0) {
		fprintf(stderr, "pa_context_connect() failed: %s",
				pa_strerror(pa_context_errno(context)));
		goto quit;
	}

	if (pa_mainloop_run(m, &ret) < 0)
		fprintf(stderr, "pa_mainloop_run() failed.");

quit:
	if (context)
		pa_context_unref(context);

	if (m) {
		pa_signal_done();
		pa_mainloop_free(m);
		pa_proplist_free(proplist);
	}
	done = true;
	pthread_kill(updater, SIGUSR1);
	return NULL;
}




int
main(void)
{
	pthread_t mainloop;

	updater = pthread_self();
	pthread_create(&mainloop, NULL, mainloop_thread, NULL);

	signal(SIGUSR1, update_volume);

	pause();
	while (!done) {
		printf("%s\n", volume);
		pause();
	}


	return 0;
}
