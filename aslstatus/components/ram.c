/* See LICENSE file for copyright and license details. */
#include <stdio.h>

#include "../util.h"

#if defined(__linux__)
	#include <stdint.h>

	void
	ram_free(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		uintmax_t free;

		if (pscanf("/proc/meminfo",
		           "MemTotal: %ju kB\n"
		           "MemFree: %ju kB\n"
		           "MemAvailable: %ju kB\n",
		           &free, &free, &free) != 3) {
			ERRRET(out);
		}

		fmt_human(out, free * 1024, 1024);
	}

	void
	ram_perc(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		uintmax_t
			free,
			total,
			cached,
			buffers;

		if (pscanf("/proc/meminfo",
		           "MemTotal: %ju kB\n"
		           "MemFree: %ju kB\n"
		           "MemAvailable: %ju kB\n"
		           "Buffers: %ju kB\n"
		           "Cached: %ju kB\n",
		           &total, &free, &buffers, &buffers, &cached) != 5)
			ERRRET(out);

		if (!total)
			ERRRET(out);

		bprintf(out, "%d", 100 * ((total - free) - (buffers + cached))
				/ total);
	}

	void
	ram_total(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		uintmax_t total;

		if (pscanf("/proc/meminfo", "MemTotal: %ju kB\n", &total) != 1)
			ERRRET(out);

		fmt_human(out, total * 1024, 1024);
	}

	void
	ram_used(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		uintmax_t
			free,
			total,
			cached,
			buffers;

		if (pscanf("/proc/meminfo",
		           "MemTotal: %ju kB\n"
		           "MemFree: %ju kB\n"
		           "MemAvailable: %ju kB\n"
		           "Buffers: %ju kB\n"
		           "Cached: %ju kB\n",
		           &total, &free, &buffers, &buffers, &cached) != 5)
			ERRRET(out);

		fmt_human(out, (total - free - buffers - cached) * 1024, 1024);
	}
#elif defined(__OpenBSD__)
	#include <stdlib.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/sysctl.h>

	#define LOG1024 10
	#define pagetok(size, pageshift) (size_t)(size << (pageshift - LOG1024))

	static inline int
	load_uvmexp(struct uvmexp *uvmexp)
	{
		size_t size;
		int uvmexp_mib[] = {CTL_VM, VM_UVMEXP};

		size = sizeof(*uvmexp);

		if (sysctl(uvmexp_mib, 2, uvmexp, &size, NULL, 0) >= 0)
			return 1;

		return 0;
	}

	void
	ram_free(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		int free_pages;
		struct uvmexp uvmexp;

		if (load_uvmexp(&uvmexp)) {
			free_pages = uvmexp.npages - uvmexp.active;
			fmt_human(out, pagetok(free_pages, uvmexp.pageshift) *
					1024, 1024);
		}

		ERRRET(out);
	}

	void
	ram_perc(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		struct uvmexp uvmexp;
		int percent;

		if (load_uvmexp(&uvmexp)) {
			percent = uvmexp.active * 100 / uvmexp.npages;
			bprintf(out, "%d", percent);
		}

		ERRRET(out);
	}

	void
	ram_total(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		struct uvmexp uvmexp;

		if (load_uvmexp(&uvmexp))
			fmt_human(out, pagetok(uvmexp.npages,
					uvmexp.pageshift) * 1024, 1024);

		ERRRET(out);
	}

	void
	ram_used(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		struct uvmexp uvmexp;

		if (load_uvmexp(&uvmexp))
			fmt_human(out, pagetok(uvmexp.active,
					uvmexp.pageshift) * 1024, 1024);

		ERRRET(out);
	}
#elif defined(__FreeBSD__)
	#include <unistd.h>
	#include <sys/sysctl.h>
	#include <sys/vmmeter.h>
	#include <vm/vm_param.h>

	void
	ram_free(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		size_t len;
		struct vmtotal vm_stats;
		int mib[] = {CTL_VM, VM_TOTAL};

		len = sizeof(struct vmtotal);
		if (sysctl(mib, 2, &vm_stats, &len, NULL, 0) == -1 || !len)
			ERRRET(out);

		fmt_human(out, vm_stats.t_free * getpagesize(), 1024);
	}

	void
	ram_total(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		size_t len;
		long int npages;

		len = sizeof(npages);
		if (sysctlbyname("vm.stats.vm.v_page_count",
				&npages, &len, NULL, 0) == -1 || !len)
			ERRRET(out);

		fmt_human(out, npages * getpagesize(), 1024);
	}

	void
	ram_perc(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		size_t len;
		long int npages;
		long int active;

		len = sizeof(npages);
		if (sysctlbyname("vm.stats.vm.v_page_count",
				&npages, &len, NULL, 0) == -1 || !len)
			ERRRET(out);

		if (sysctlbyname("vm.stats.vm.v_active_count",
				&active, &len, NULL, 0) == -1 || !len)
			ERRRET(out);

		bprintf(out, "%d", active * 100 / npages);
	}

	void
	ram_used(char *out, const char __unused *_a,
		unsigned int __unused _i, void __unused *_p)
	{
		size_t len;
		long int active;

		len = sizeof(active);
		if (sysctlbyname("vm.stats.vm.v_active_count",
				&active, &len, NULL, 0) == -1 || !len)
			ERRRET(out);

		fmt_human(out, active * getpagesize(), 1024);
	}
#endif
