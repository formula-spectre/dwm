/* See LICENSE file for copyright and license details. */
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#include "../util.h"

void
gid(char *out, const char __unused *_a,
	unsigned int __unused _i, void __unused *_p)
{ bprintf(out, "%d", getgid()); }

void
uid(char *out, const char __unused *_a,
	unsigned int __unused _i, void __unused *_p)
{ bprintf(out, "%d", geteuid()); }

void
username(char *out, const char __unused *_a,
	unsigned int __unused _i, void __unused *_p)
{
	struct passwd *pw;

	if (!(pw = getpwuid(geteuid()))) {
		warn("getpwuid '%d':", geteuid());
		ERRRET(out);
	}

	bprintf(out, "%s", pw->pw_name);
}
