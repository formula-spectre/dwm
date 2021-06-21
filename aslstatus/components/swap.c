/* See LICENSE file for copyright and license details. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../util.h"

#if defined(__linux__)
	#define STRSIZE(STR) (STR), (sizeof(STR) -1)

	static inline int
	get_swap_info(long int *s_total, long int *s_free, long int *s_cached)
	{
		struct {
			const char *name;
			const size_t len;
			long int *var;
		} ent[] = {
			{ STRSIZE("SwapTotal") , s_total  },
			{ STRSIZE("SwapFree")  , s_free   },
			{ STRSIZE("SwapCached"), s_cached },
		};
		FILE *fp;
		char *line = NULL;
		size_t
			i,
			left,
			line_len = 0;

		/* get number of fields we want to extract */
		for (i = 0, left = 0; i < LEN(ent); i++)
			if (ent[i].var)
				left++;

		if (!(fp = fopen("/proc/meminfo", "r"))) {
			warn("fopen '/proc/meminfo':");
			return 1;
		}

		/* read file line by line and extract field information */
		while (left > 0 && getline(&line, &line_len, fp) >= 0) {
			for (i = 0; i < LEN(ent); i++) {
				if (ent[i].var &&
				    !strncmp(line, ent[i].name, ent[i].len)) {
					sscanf(line + ent[i].len + 1,
					       "%ld kB\n", ent[i].var);
					left--;
					break;
				}
			}
		}
		free(line);
		if (ferror(fp)) {
			warn("getline '/proc/meminfo':");
			return 1;
		}

		fclose(fp);
		return 0;
	}

	void
	swap_free(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		long int free;

		if (get_swap_info(NULL, &free, NULL))
			ERRRET(out);

		fmt_human(out, free * 1024, 1024);
	}

	void
	swap_perc(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		long int
			free,
			total,
			cached;

		if (get_swap_info(&total, &free, &cached) || total == 0)
			ERRRET(out);

		bprintf(out, "%d", 100 * (total - free - cached) / total);
	}

	void
	swap_total(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		long int total;

		if (get_swap_info(&total, NULL, NULL))
			ERRRET(out);

		fmt_human(out, total * 1024, 1024);
	}

	void
	swap_used(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		long int
			free,
			total,
			cached;

		if (get_swap_info(&total, &free, &cached))
			ERRRET(out);

		fmt_human(out, (total - free - cached) * 1024, 1024);
	}
#elif defined(__OpenBSD__)
	#include <stdlib.h>
	#include <unistd.h>
	#include <sys/swap.h>
	#include <sys/types.h>

	static inline int
	getstats(int *total, int *used)
	{
		int
			i,
			nswap,
			rnswap;
		struct swapent
			*sep,
			*fsep;

		if ((nswap = swapctl(SWAP_NSWAP, 0, 0)) < 1) {
			warn("swaptctl 'SWAP_NSWAP':");
			return 1;
		}
		if (!(fsep = sep = calloc(nswap, sizeof(*sep)))) {
			warn("calloc 'nswap':");
			return 1;
		}
		if ((rnswap = swapctl(SWAP_STATS, (void *)sep, nswap)) < 0) {
			warn("swapctl 'SWAP_STATA':");
			return 1;
		}
		if (nswap != rnswap) {
			warn("getstats: SWAP_STATS != SWAP_NSWAP");
			return 1;
		}

		*total = 0;
		*used = 0;

		for (i = 0; i < rnswap; i++) {
			*total += sep->se_nblks >> 1;
			*used += sep->se_inuse >> 1;
		}

		free(fsep);

		return 0;
	}

	void
	swap_free(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		int
			used,
			total;

		if (getstats(&total, &used))
			ERRRET(out);

		fmt_human(out, (total - used) * 1024, 1024);
	}

	void
	swap_perc(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		int
			used,
			total;

		if (getstats(&total, &used))
			ERRRET(out);

		if (total == 0) {
			return NULL;
		}

		bprintf(out, "%d", 100 * used / total);
	}

	void
	swap_total(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		int
			used,
			total;

		if (getstats(&total, &used))
			ERRRET(out);

		fmt_human(out, total * 1024, 1024);
	}

	void
	swap_used(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		int
			used,
			total;

		if (getstats(&total, &used))
			ERRRET(out);

		fmt_human(out, used * 1024, 1024);
	}
#elif defined(__FreeBSD__)
	#include <stdlib.h>
	#include <sys/types.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <kvm.h>

	static inline int
	getswapinfo(struct kvm_swap *swap_info, size_t size)
	{
		kvm_t *kd;

		kd = kvm_openfiles(NULL, "/dev/null", NULL, 0, NULL);
		if(kd == NULL) {
			warn("kvm_openfiles '/dev/null':");
			return 0;
		}

		if(kvm_getswapinfo(kd, swap_info, size, 0) == -1) {
			warn("kvm_getswapinfo:");
			kvm_close(kd);
			return 0;
		}

		kvm_close(kd);
		return 1;
	}

	void
	swap_free(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		long int
			used,
			total;
		struct kvm_swap swap_info[1];

		if(!getswapinfo(swap_info, 1))
			ERRRET(out);

		total = swap_info[0].ksw_total;
		used = swap_info[0].ksw_used;

		fmt_human(out, (total - used) * getpagesize(), 1024);
	}

	void
	swap_perc(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		long int
			used,
			total;
		struct kvm_swap swap_info[1];

		if(!getswapinfo(swap_info, 1))
			ERRRET(out);

		total = swap_info[0].ksw_total;
		used = swap_info[0].ksw_used;

		bprintf(out, "%d", used * 100 / total);
	}

	void
	swap_total(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		long int total;
		struct kvm_swap swap_info[1];

		if(!getswapinfo(swap_info, 1))
			ERRRET(out);

		total = swap_info[0].ksw_total;

		fmt_human(out, total * getpagesize(), 1024);
	}

	void
	swap_used(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		long int used;
		struct kvm_swap swap_info[1];

		if(!getswapinfo(swap_info, 1))
			ERRRET(out);

		used = swap_info[0].ksw_used;

		fmt_human(out, used * getpagesize(), 1024);
	}
#endif
