/* See LICENSE file for copyright and license details. */
#include <time.h>
#include <stdio.h>
#include <stdint.h>

#include "../util.h"

#if defined(CLOCK_BOOTTIME)
	#define UPTIME_FLAG CLOCK_BOOTTIME
#elif defined(CLOCK_UPTIME)
	#define UPTIME_FLAG CLOCK_UPTIME
#else
	#define UPTIME_FLAG CLOCK_MONOTONIC
#endif

void
uptime(char *out, const char __unused *_a,
	unsigned int __unused _i, void __unused *_p)
{
	uintmax_t h, m;
	char warn_buf[256];
	struct timespec uptime;

	if (clock_gettime(UPTIME_FLAG, &uptime) < 0) {
		snprintf(warn_buf, 256, "clock_gettime %d", UPTIME_FLAG);
		warn(warn_buf);
		ERRRET(out);
	}

	h = uptime.tv_sec / 3600;
	m = uptime.tv_sec % 3600 / 60;

	bprintf(out, "%juh %jum", h, m);
}
