/* See LICENSE file for copyright and license details. */
#include <sys/utsname.h>
#include <stdio.h>

#include "../util.h"

void
kernel_release(char *out, const char __unused *_a,
	unsigned int __unused _i, void __unused *_p)
{
	struct utsname udata;

	if (uname(&udata) < 0) {
		warn("uname:");
		ERRRET(out);
	}

	bprintf(out, "%s", udata.release);
}
