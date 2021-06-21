/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "../util.h"

#if defined(__linux__)
	void
	cpu_freq(char *out, const char __unused *_a, unsigned int __unused _i,
		void __unused *_p)
	{
		uintmax_t freq;

		/* in kHz */
		if (pscanf("/sys/devices/system/cpu/cpu0/cpufreq/"
		           "scaling_cur_freq", "%ju", &freq) != 1) {
			ERRRET(out);
		}

		fmt_human(out, freq * 1000, 1000);
	}

	void
	cpu_perc(char *out, const char __unused *_a, unsigned int __unused _i,
		void *static_ptr)
	{
		long double b[7], sum;
		long double *a =  static_ptr;

		memcpy(b, a, sizeof(b));
		/* cpu user nice system idle iowait irq softirq */
		if (pscanf("/proc/stat", "%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf",
		           &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6])
		    != 7) {
			ERRRET(out);
		}
		if (!b[0]) {
			ERRRET(out);
		}

		sum = (b[0] + b[1] + b[2] + b[3] + b[4] + b[5] + b[6]) -
		      (a[0] + a[1] + a[2] + a[3] + a[4] + a[5] + a[6]);

		if (!sum)
			ERRRET(out);

		bprintf(out, "%3d", (int)(100 *
		               ((b[0] + b[1] + b[2] + b[5] + b[6]) -
		                (a[0] + a[1] + a[2] + a[5] + a[6])) / sum));
	}
#elif defined(__OpenBSD__)
	#include <sys/param.h>
	#include <sys/sched.h>
	#include <sys/sysctl.h>

	void
	cpu_freq(char *out, const char __unused *_a, unsigned int __unused _i,
		void __unused *_p)
	{
		int freq, mib[2];
		size_t size;

		mib[0] = CTL_HW;
		mib[1] = HW_CPUSPEED;

		size = sizeof(freq);

		/* in MHz */
		if (sysctl(mib, 2, &freq, &size, NULL, 0) < 0) {
			warn("sysctl 'HW_CPUSPEED':");
			ERRRET(out);
		}

		fmt_human(out, freq * 1E6, 1000);
	}

	void
	cpu_perc(char *out, const char __unused *_a, unsigned int __unused _i,
		void *static_ptr)
	{
		int mib[2];
		size_t size;
		uintmax_t
			sum,
			b[CPUSTATES];
		uintmax_t *a = static_ptr;

		mib[0] = CTL_KERN;
		mib[1] = KERN_CPTIME;

		size = sizeof(b);

		memcpy(b, a, size);
		if (sysctl(mib, 2, &a, &size, NULL, 0) < 0) {
			warn("sysctl 'KERN_CPTIME':");
			ERRRET(out);
		}
		if (!b[0])
			ERRRET(out);

		sum = (a[CP_USER] + a[CP_NICE] + a[CP_SYS] + a[CP_INTR] + a[CP_IDLE]) -
		      (b[CP_USER] + b[CP_NICE] + b[CP_SYS] + b[CP_INTR] + b[CP_IDLE]);

		if (!sum)
			ERRRET(out);

		bprintf(out, "%3d", 100 *
		               ((a[CP_USER] + a[CP_NICE] + a[CP_SYS] +
		                 a[CP_INTR]) -
		                (b[CP_USER] + b[CP_NICE] + b[CP_SYS] +
		                 b[CP_INTR])) / sum);
	}
#elif defined(__FreeBSD__)
	#include <sys/param.h>
	#include <sys/sysctl.h>
	#include <devstat.h>

	void
	cpu_freq(char *out, const char __unused *_a, unsigned int __unused _i,
		void __unused *_p)
	{
		int freq;
		size_t size;

		size = sizeof(freq);
		/* in MHz */
		if (sysctlbyname("hw.clockrate", &freq, &size, NULL, 0) == -1
				|| !size) {
			warn("sysctlbyname 'hw.clockrate':");
			ERRRET(out);
		}

		fmt_human(out, freq * 1E6, 1000);
	}

	void
	cpu_perc(char *out, const char __unused *_a, unsigned int __unused _i,
		void *static_ptr)
	{
		size_t size;
		long
			sum,
			b[CPUSTATES];
		long *a = static_ptr;

		size = sizeof(b);
		memcpy(b, a, size);
		if (sysctlbyname("kern.cp_time", &a, &size, NULL, 0) == -1
				|| !size) {
			warn("sysctlbyname 'kern.cp_time':");
			ERRRET(out);
		}
		if (!b[0]) {
			ERRRET(out);
		}

		sum = (a[CP_USER] + a[CP_NICE] + a[CP_SYS] + a[CP_INTR] + a[CP_IDLE]) -
		      (b[CP_USER] + b[CP_NICE] + b[CP_SYS] + b[CP_INTR] + b[CP_IDLE]);

		if (!sum) {
			ERRRET(out);
		}

		bprintf(out, "%3d", 100 *
		               ((a[CP_USER] + a[CP_NICE] + a[CP_SYS] +
		                 a[CP_INTR]) -
		                (b[CP_USER] + b[CP_NICE] + b[CP_SYS] +
		                 b[CP_INTR])) / sum);
	}
#endif
