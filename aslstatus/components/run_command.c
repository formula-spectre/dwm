/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>

#include "../util.h"

void
run_command(char *buf, const char *cmd,
	unsigned int __unused _i, void __unused *_p)
{
	char *p;
	FILE *fp;

	if (!(fp = popen(cmd, "r"))) {
		warn("popen '%s':", cmd);
		ERRRET(buf);
	}
	p = fgets(buf, BUFF_SZ -1, fp);
	if (pclose(fp) < 0) {
		warn("pclose '%s':", cmd);
		ERRRET(buf);
	}
	if (!p)
		ERRRET(buf);

	if ((p = strrchr(buf, '\n')))
		p[0] = '\0';
}
