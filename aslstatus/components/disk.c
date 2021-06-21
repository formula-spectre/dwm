/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <sys/statvfs.h>

#include "../util.h"

void
disk_free(char *out, const char *path,
	unsigned int __unused _i, void __unused *_p)
{
	struct statvfs fs;

	if (statvfs(path, &fs) < 0) {
		warn("statvfs '%s':", path);
		ERRRET(out);
	}

	fmt_human(out, fs.f_frsize * fs.f_bavail, 1024);
}

void
disk_perc(char *out, const char *path,
	unsigned int __unused _i, void __unused *_p)
{
	struct statvfs fs;

	if (statvfs(path, &fs) < 0) {
		warn("statvfs '%s':", path);
		ERRRET(out);
	}

	bprintf(out, "%d", (int)(100 *
	               (1.0f - ((float)fs.f_bavail / (float)fs.f_blocks))));
}

void
disk_total(char *out, const char *path,
	unsigned int __unused _i, void __unused *_p)
{
	struct statvfs fs;

	if (statvfs(path, &fs) < 0) {
		warn("statvfs '%s':", path);
		ERRRET(out);
	}

	fmt_human(out, fs.f_frsize * fs.f_blocks, 1024);
}

void
disk_used(char *out, const char *path,
	unsigned int __unused _i, void __unused *_p)
{
	struct statvfs fs;

	if (statvfs(path, &fs) < 0) {
		warn("statvfs '%s':", path);
		ERRRET(out);
	}

	fmt_human(out, fs.f_frsize * (fs.f_blocks - fs.f_bfree), 1024);
}
