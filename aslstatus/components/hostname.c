/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <unistd.h>

#include "../util.h"

void
hostname(char *out, const char __unused *_a,
	unsigned int __unused _i, void __unused *_p)
{
	if (gethostname(out, BUFF_SZ) < 0) {
		warn("gethostbyname:");
		ERRRET(out);
	}
}
