/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>

#include "../util.h"

void
load_avg(char *out, const char __unused *_a,
	unsigned int __unused _i, void __unused *_p)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		warn("getloadavg: Failed to obtain load average");
		ERRRET(out);
	}

	bprintf(out, "%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}
