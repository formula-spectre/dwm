/* See LICENSE file for copyright and license details. */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "volume.h"
#include "../util.h"


#if defined(USE_ALSA)
	#include <math.h>
	#include <stdbool.h>

	#include <alsa/asoundlib.h>
	#include <alsa/control.h>
	/* header file inclusion order is important */


	static const size_t CTL_NAME_MAX = 3 + 10 + 1;
	/*
		3  - "hw:"
		10 - len(str(UINT_MAX))
		1  - zero byte
	*/
	static const char CARD[] = "default";
	static const char MIXER_NAME[] = "Master";

	typedef struct volume_static_data static_data;

	static inline snd_mixer_t *
	get_mixer_elem(snd_mixer_elem_t **ret, snd_mixer_selem_id_t **sid)
	/*
		after using `mixer_elem`
		to free memory returned `mixer` must be closed with:
		`snd_mixer_close`

		(see `is_muted` function)
	*/
	{
		int err;
		snd_mixer_t *handle;

		if (!*sid) {
			if ((err = snd_mixer_selem_id_malloc(sid)) < 0) {
				warn("failed to allocate memory for: %s",
						snd_strerror(err));
				return NULL;
			}
			snd_mixer_selem_id_set_name(*sid, MIXER_NAME);
		}

		if ((err = snd_mixer_open(&handle, 0)) < 0) {
			warn("cannot open mixer: %s", snd_strerror(err));
			return NULL;
		}
		if ((err = snd_mixer_attach(handle, CARD)) < 0) {
			warn("cannot attach mixer: %s", snd_strerror(err));
			snd_mixer_close(handle);
			return NULL;
		}
		if ((err = snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
			warn("cannot register mixer: %s", snd_strerror(err));
			snd_mixer_close(handle);
			return NULL;
		}
		if ((err = snd_mixer_load(handle)) < 0) {
			warn("failed to load mixer: %s", snd_strerror(err));
			snd_mixer_close(handle);
			return NULL;
		}

		*ret = snd_mixer_find_selem(handle, *sid);

		return handle;
	}


	static inline bool
	is_muted(snd_mixer_selem_id_t **sid)
	{
		int psw;
		snd_mixer_t *handle;
		snd_mixer_elem_t *elem;

		if (!(handle = get_mixer_elem(&elem, sid)))
			return 0;

		snd_mixer_selem_get_playback_switch(elem,
				SND_MIXER_SCHN_MONO, &psw);
		snd_mixer_close(handle);

		return !psw;
	}


	static inline unsigned short int
	get_percentage(__typeof__(((static_data*)0)->volume) *v,
			snd_mixer_selem_id_t **sid)
	{
		int err;
		long int vol;
		snd_mixer_t *handle;
		snd_mixer_elem_t *elem;

		if (!(handle = get_mixer_elem(&elem, sid)))
			return 0;

		if (!v->max)
			snd_mixer_selem_get_playback_volume_range(
				elem,
				&v->min,
				&v->max
			);

		err = snd_mixer_selem_get_playback_volume(elem, 0, &vol);
		snd_mixer_close(handle);
		if (err < 0) {
			warn("cannot get playback volume: %s",
					snd_strerror(err));
			return 0;
		}

		return (unsigned short int)
			((vol - v->min) * 100 / (v->max - v->min));
	}


	static inline char *
	get_ctl_name(snd_mixer_selem_id_t **sid)
	/* after using return must be freed */
	{
		char *ctl_name;
		unsigned int index;
		snd_mixer_t *handle;
		snd_mixer_elem_t *elem;

		if (!(handle = get_mixer_elem(&elem, sid))) {
			index = 0;
		} else {
			index = snd_mixer_selem_get_index(elem);
			snd_mixer_close(handle);
		}
		if (!(ctl_name = calloc(CTL_NAME_MAX, 1))) {
			warn("failed to allocate memory for ctl_name");
			return NULL;
		}
		snprintf(ctl_name, CTL_NAME_MAX, "hw:%u", index);
		return ctl_name;
	}


	void
	vol_perc(char *volume, const char __unused *_a,
		unsigned int __unused _i, void *static_ptr)
	{
		int err;
		char *ctl_name;
		static_data *data = static_ptr;

		if (!data->ctl) {
			if (!(ctl_name = get_ctl_name(&data->sid)))
				ERRRET(volume);

			snd_ctl_open(&data->ctl, ctl_name, SND_CTL_READONLY);
			free(ctl_name);

			err = snd_ctl_subscribe_events(data->ctl, 1);
			if (err < 0) {
				snd_ctl_close(data->ctl);
				data->ctl = NULL;
				warn("cannot subscribe to alsa events: %s",
						snd_strerror(err));
				ERRRET(volume);
			}
			snd_ctl_event_malloc(&data->e);
		} else {
			snd_ctl_read(data->ctl, data->e);
		}

		if (is_muted(&data->sid))
			bprintf(volume, "%s", MUTED);
		else
			bprintf(
				volume, "%s%3hu%s", SYM,
				get_percentage(&data->volume, &data->sid),
				PERCENT
			);
	}

#elif defined(USE_PULSE)
/* !!! in progress !!! */
/*
	#include <signal.h>
	#include <assert.h>
	#include <stdlib.h>
	#include <stdbool.h>

	#include <pthread.h>
	#include <pulse/pulseaudio.h>


	static char *server = NULL;
	static pa_context *context = NULL;
	static pa_proplist *proplist = NULL;
	static pa_mainloop_api *mainloop_api = NULL;

	static pthread_t updater = NULL;
	static volatile bool done = false;
	static char volume[LEN(MUTED) > 3 ? LEN(MUTED) :3] = {0};

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

*/

#elif defined(__OpenBSD__)
	#include <sys/audioio.h>

	void
	vol_perc(char *out, const char *card)
	{
		static int cls = -1;
		mixer_devinfo_t mdi;
		mixer_ctrl_t mc;
		int afd = -1, m = -1, v = -1;

		if ((afd = open(card, O_RDONLY)) < 0) {
			warn("open '%s':", card);
			ERRRET(out);
		}

		for (mdi.index = 0; cls == -1; mdi.index++) {
			if (ioctl(afd, AUDIO_MIXER_DEVINFO, &mdi) < 0) {
				warn("ioctl 'AUDIO_MIXER_DEVINFO':");
				close(afd);
				ERRRET(out);
			}
			if (mdi.type == AUDIO_MIXER_CLASS &&
			    !strncmp(mdi.label.name,
				     AudioCoutputs,
				     MAX_AUDIO_DEV_LEN))
				cls = mdi.index;
			}
		for (mdi.index = 0; v == -1 || m == -1; mdi.index++) {
			if (ioctl(afd, AUDIO_MIXER_DEVINFO, &mdi) < 0) {
				warn("ioctl 'AUDIO_MIXER_DEVINFO':");
				close(afd);
				ERRRET(out);
			}
			if (mdi.mixer_class == cls &&
			    ((mdi.type == AUDIO_MIXER_VALUE &&
			      !strncmp(mdi.label.name,
				       AudioNmaster,
				       MAX_AUDIO_DEV_LEN)) ||
			     (mdi.type == AUDIO_MIXER_ENUM &&
			      !strncmp(mdi.label.name,
				      AudioNmute,
				      MAX_AUDIO_DEV_LEN)))) {
				mc.dev = mdi.index, mc.type = mdi.type;
				if (ioctl(afd, AUDIO_MIXER_READ, &mc) < 0) {
					warn("ioctl 'AUDIO_MIXER_READ':");
					close(afd);
					ERRRET(out);
				}
				if (mc.type == AUDIO_MIXER_VALUE)
					v = mc.un.value.num_channels == 1 ?
					    mc.un.value.level[AUDIO_MIXER_LEVEL_MONO] :
					    (mc.un.value.level[AUDIO_MIXER_LEVEL_LEFT] >
					     mc.un.value.level[AUDIO_MIXER_LEVEL_RIGHT] ?
					     mc.un.value.level[AUDIO_MIXER_LEVEL_LEFT] :
					     mc.un.value.level[AUDIO_MIXER_LEVEL_RIGHT]);
				else if (mc.type == AUDIO_MIXER_ENUM)
					m = mc.un.ord;
			}
		}

		close(afd);

		bprintf(out, "%3d", m ? 0 : v * 100 / 255);
	}
#else
	#include <sys/soundcard.h>

	void
	vol_perc(char * out, const char *card)
	{
		size_t i;
		int v, afd, devmask;
		char *vnames[] = SOUND_DEVICE_NAMES;

		if ((afd = open(card, O_RDONLY | O_NONBLOCK)) < 0) {
			warn("open '%s':", card);
			ERRRET(out);
		}

		if (ioctl(afd, (int)SOUND_MIXER_READ_DEVMASK, &devmask) < 0) {
			warn("ioctl 'SOUND_MIXER_READ_DEVMASK':");
			close(afd);
			ERRRET(out);
		}
		for (i = 0; i < LEN(vnames); i++) {
			if (devmask & (1 << i) && !strcmp("vol", vnames[i])) {
				if (ioctl(afd, MIXER_READ(i), &v) < 0) {
					warn("ioctl 'MIXER_READ(%ld)':", i);
					close(afd);
					ERRRET(out);
				}
			}
		}

		close(afd);

		bprintf(out, "%3d", v & 0xff);
	}
#endif
