/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <limits.h>

#include "../util.h"

#if defined(__linux__)
	#include <stdint.h>

	void
	netspeed_rx(char *out, const char *interface,
		unsigned int interval, void *static_ptr)
	{
		char path[PATH_MAX];
		uintmax_t oldrxbytes;
		uintmax_t *rxbytes = static_ptr;

		oldrxbytes = *rxbytes;

		if ((esnprintf(path, sizeof(path),
				"/sys/class/net/%s/statistics/rx_bytes",
				interface) < 0) ||
				(pscanf(path, "%ju", rxbytes) != 1) ||
				(oldrxbytes == 0)) {
			ERRRET(out);
		}

		fmt_human(out, (*rxbytes - oldrxbytes) * 1000 / interval, 1024);
	}

	void
	netspeed_tx(char *out, const char *interface,
		unsigned int interval, void *static_ptr)
	{
		char path[PATH_MAX];
		uintmax_t oldtxbytes;
		uintmax_t *txbytes = static_ptr;

		oldtxbytes = *txbytes;

		if (esnprintf(path, sizeof(path),
		              "/sys/class/net/%s/statistics/tx_bytes",
		              interface) < 0) {
			ERRRET(out);
		}
		if (pscanf(path, "%ju", txbytes) != 1) {
			ERRRET(out);
		}
		if (oldtxbytes == 0) {
			ERRRET(out);
		}

		fmt_human(out, (*txbytes - oldtxbytes) * 1000 / interval, 1024);
	}
#elif defined(__OpenBSD__) | defined(__FreeBSD__)
	#include <net/if.h>
	#include <string.h>
	#include <ifaddrs.h>
	#include <sys/types.h>
	#include <sys/socket.h>

	void
	netspeed_rx(char *out, const char *interface,
		unsigned int interval, void *static_ptr)
	{
		int if_ok = 0;
		struct ifaddrs *ifa;
		struct if_data *ifd;
		struct ifaddrs *ifal;
		uintmax_t oldrxbytes;
		uintmax_t *rxbytes = static_ptr;

		oldrxbytes = *rxbytes;

		if (getifaddrs(&ifal) == -1) {
			warn("getifaddrs failed");
			ERRRET(out);
		}
		*rxbytes = 0;
		for (ifa = ifal; ifa; ifa = ifa->ifa_next) {
			if (!strcmp(ifa->ifa_name, interface) &&
			   (ifd = (struct if_data *)ifa->ifa_data)) {
				*rxbytes += ifd->ifi_ibytes, if_ok = !0;
			}
		}
		freeifaddrs(ifal);
		if (!if_ok) {
			warn("reading 'if_data' failed");
			ERRRET(out);
		}
		if (oldrxbytes == 0) {
			ERRRET(out);
		}

		fmt_human(out, (*rxbytes - oldrxbytes) * 1000 / interval, 1024);
	}

	void
	netspeed_tx(char *out, const char *interface,
		unsigned int interval, void *static_ptr)
	{
		int if_ok = 0;
		struct ifaddrs *ifa;
		struct if_data *ifd;
		struct ifaddrs *ifal;
		uintmax_t oldtxbytes;
		uintmax_t *txbytes = static_ptr;

		oldtxbytes = *txbytes;

		if (getifaddrs(&ifal) == -1) {
			warn("getifaddrs failed");
			ERRRET(out);
		}
		*txbytes = 0;
		for (ifa = ifal; ifa; ifa = ifa->ifa_next) {
			if (!strcmp(ifa->ifa_name, interface) &&
			   (ifd = (struct if_data *)ifa->ifa_data)) {
				*txbytes += ifd->ifi_obytes, if_ok = 1;
			}
		}
		freeifaddrs(ifal);
		if (!if_ok) {
			warn("reading 'if_data' failed");
			ERRRET(out);
		}
		if (oldtxbytes == 0) {
			ERRRET(out);
		}

		fmt_human(out, (*txbytes - oldtxbytes) * 1000 / interval, 1024);
	}
#endif
