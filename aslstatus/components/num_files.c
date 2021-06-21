/* See LICENSE file for copyright and license details. */
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include "../util.h"

void
num_files(char *out, const char *path,
	unsigned int __unused _i, void __unused *_p)
{
	DIR *fd;
	unsigned int num;
	struct dirent *dp;

	if (!(fd = opendir(path))) {
		warn("opendir '%s':", path);
		ERRRET(out);
	}

	num = 0;
	while ((dp = readdir(fd))) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue; /* skip self and parent */
		num++;
	}

	closedir(fd);
	bprintf(out, "%u", num);
}
