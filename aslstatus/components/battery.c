/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>

#include "../util.h"

static const char FULL[] =        "!";  /* "f" */
static const char UNCNOWN[] =     "?";  /* "?" */
static const char CHARGING[] =    "+"; /* "+" */
static const char DISCHARGING[] = "-"; /* "-" */

#if defined(__linux__)
	#include <limits.h>
	#include <stdint.h>
	#include <unistd.h>

	#define MAX_STATE 13
	#define STATE_PATTERN "%" STR(MAX_STATE) "[^\n]s"

	static inline const char *
	pick(const char *bat, const char *f1, const char *f2, char *path,
		size_t length)
	{
		if (esnprintf(path, length, f1, bat) > 0 &&
		    access(path, R_OK) == 0) {
			return f1;
		}

		if (esnprintf(path, length, f2, bat) > 0 &&
		    access(path, R_OK) == 0) {
			return f2;
		}

		return NULL;
	}

	void
	battery_perc(char *out, const char *bat,
		unsigned int __unused _i, void __unused *_p)
	{
		int perc;
		char path[PATH_MAX];
		
		perc = esnprintf(path, sizeof(path),
				"/sys/class/power_supply/%s/capacity", bat);
		if (perc < 0 || pscanf(path, "%d", &perc) != 1) {
			ERRRET(out);
		}

		bprintf(out, "%3d", perc);
	}

	void
	battery_state(char *out, const char *bat,
		unsigned int __unused _i, void __unused *_p)
	{
		size_t i;
		char
			path[PATH_MAX],
			state[MAX_STATE];
		static const struct {
			const char *state;
			const char *symbol;
		} map[] = {
			{ "Charging",     CHARGING },
			{ "Discharging",  DISCHARGING },
			{ "Not charging", FULL },
		};

		if (esnprintf(path, sizeof(path),
		              "/sys/class/power_supply/%s/status", bat) < 0) {
			ERRRET(out);
		}
		if (pscanf(path, STATE_PATTERN, state) != 1)
			ERRRET(out);

		for (i = 0; i < LEN(map); i++)
			if (!strcmp(map[i].state, state))
				break;
		bprintf(out, "%s", (i == LEN(map)) ? UNCNOWN : map[i].symbol);
	}

	void
	battery_remaining(char *out, const char *bat,
		unsigned int __unused _i, void __unused *_p)
	{
		uintmax_t
			m, h,
			charge_now,
			current_now;
		char
			path[PATH_MAX],
			state[MAX_STATE];

		double timeleft;

		if (esnprintf(path, sizeof(path),
		              "/sys/class/power_supply/%s/status", bat) < 0) {
			ERRRET(out);
		}
		if (pscanf(path, STATE_PATTERN, state) != 1) {
			ERRRET(out);
		}

		if (!pick(bat, "/sys/class/power_supply/%s/charge_now",
		          "/sys/class/power_supply/%s/energy_now", path,
		          sizeof(path)) ||
		    pscanf(path, "%ju", &charge_now) < 0) {
			ERRRET(out);
		}

		if (!strcmp(state, "Discharging")) {
			if (!pick(bat, "/sys/class/power_supply/%s/current_now",
			          "/sys/class/power_supply/%s/power_now", path,
			          sizeof(path)) ||
			    pscanf(path, "%ju", &current_now) < 0) {
				ERRRET(out);
			}

			if (current_now == 0) {
				ERRRET(out);
			}

			timeleft = (double)charge_now / (double)current_now;
			h = timeleft;
			m = (timeleft - (double)h) * 60;

			bprintf(out, "%juh %jum", h, m);
			return;
		}

		ERRRET(out);
	}
#elif defined(__OpenBSD__)
	#include <fcntl.h>
	#include <machine/apmvar.h>
	#include <sys/ioctl.h>
	#include <unistd.h>

	static inline int
	load_apm_power_info(struct apm_power_info *apm_info)
	{
		int fd;

		fd = open("/dev/apm", O_RDONLY);
		if (fd < 0) {
			warn("open '/dev/apm':");
			return 0;
		}

		memset(apm_info, 0, sizeof(struct apm_power_info));
		if (ioctl(fd, APM_IOC_GETPOWER, apm_info) < 0) {
			warn("ioctl 'APM_IOC_GETPOWER':");
			close(fd);
			return 0;
		}
		return close(fd), 1;
	}

	void
	battery_perc(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		struct apm_power_info apm_info;

		if (load_apm_power_info(&apm_info)) {
			bptintf(out, "%3d", apm_info.battery_life);
		}

		ERRRET(out);
	}

	void
	battery_state(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		struct {
			unsigned int state;
			char *symbol;
		} map[] = {
			{ APM_AC_ON,      CHARGING },
			{ APM_AC_OFF,     DISCHARGING },
		};
		struct apm_power_info apm_info;
		size_t i;

		if (!load_apm_power_info(&apm_info))
			ERRRET(out);
		for (i = 0; i < LEN(map); i++) {
			if (map[i].state == apm_info.ac_state) {
				break;
			}
		}
		strncpy(out, (i == LEN(map)) ? UNCNOWN : map[i].symbol, BUFF_SZ);
	}

	void
	battery_remaining(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		struct apm_power_info apm_info;

		if (load_apm_power_info(&apm_info))
			if (apm_info.ac_state != APM_AC_ON) {
				bptintf(out, "%uh %02um",
			                       apm_info.minutes_left / 60,
				               apm_info.minutes_left % 60);
				return;
			}
		ERRRET(out);
	}
#elif defined(__FreeBSD__)
	#include <sys/sysctl.h>

	void
	battery_perc(char *out, const char *unused)
	{
		int cap;
		size_t len;

		len = sizeof(cap);
		if (sysctlbyname("hw.acpi.battery.life", &cap, &len, NULL, 0) == -1
				|| !len)
			ERRRET(out);

		bptintf(out, "%3d", cap);
	}

	void
	battery_state(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		int state;
		size_t len;

		len = sizeof(state);
		if (sysctlbyname("hw.acpi.battery.state", &state, &len, NULL, 0) == -1
				|| !len)
			ERRRET(out);

		switch(state) {
			case 0:
			case 2:
				strncpy(out, CHARGING, BUFF_SZ);
			case 1:
				strncpy(out, DISCHARGING, BUFF_SZ);
			default:
				strncpy(out, UNCNOWN, BUFF_SZ);
		}
	}

	void
	battery_remaining(char *out, const char __unused *f,
		unsigned int __unused i, void __unused *p)
	{
		int rem;
		size_t len;

		len = sizeof(rem);
		if (sysctlbyname("hw.acpi.battery.time", &rem, &len, NULL, 0) == -1
				|| !len
				|| rem == -1)
			ERRRET(out);

		bptintf(out, "%uh %02um", rem / 60, rem % 60);
	}
#endif
