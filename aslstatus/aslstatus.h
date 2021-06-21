/* See LICENSE file for copyright and license details. */

#include <limits.h>  /* PATH_MAX */

#include <X11/Xlib.h>

#include "os.h"

#define FUNC_ARGS (char *, const char *, unsigned int, void *)


typedef struct _func_t {
	void (*func) FUNC_ARGS;
	const char name[16];
	unsigned int static_size;
} func_t;


/* battery */
void battery_perc FUNC_ARGS;
#define battery_perc {battery_perc, "batt_percentage", 0}

void battery_state FUNC_ARGS;
#define battery_state {battery_state, "batt_state", 0}

void battery_remaining FUNC_ARGS;
#define battery_remaining {battery_remaining, "batt_remaining", 0}


/* cpu */
void cpu_freq FUNC_ARGS;
#define cpu_freq {cpu_freq, "cpu_freq", 0}

void cpu_perc FUNC_ARGS;
#if ISBSD
# include <sys/param.h>  /* CPUSTATES */
#else
# define CPUSTATES 1
#endif
#define cpu_perc {cpu_perc, "cpu_percentage", 0		\
	+ (LINUX * sizeof(long double [7]))		\
	+ (OBSD  * sizeof(uintmax_t   [CPUSTATES]))	\
	+ (FBSD  * sizeof(long        [CPUSTATES]))	\
}

/* datetime */
void datetime FUNC_ARGS;
#define datetime {datetime, "datetime", 0}


/* disk */
void disk_free FUNC_ARGS;
#define disk_free {disk_free, "disk_free", 0}

void disk_perc FUNC_ARGS;
#define disk_perc {disk_perc, "disk_percentage", 0}

void disk_total FUNC_ARGS;
#define disk_total {disk_total, "disk_total", 0}

void disk_used FUNC_ARGS;
#define disk_used {disk_used, "disk_used", 0}


/* entropy */
void entropy FUNC_ARGS;
#define entropy {entropy, "entropy", 0}


/* hostname */
void hostname FUNC_ARGS;
#define hostname {hostname, "hostname", 0}


/* ip */
void ipv4 FUNC_ARGS;
#define ipv4 {ipv4, "ipv4", 0}

void ipv6 FUNC_ARGS;
#define ipv6 {ipv6, "ipv6", 0}


/* kernel_release */
void kernel_release FUNC_ARGS;
#define kernel_release {kernel_release, "kernel_release", 0}


/* keyboard_indicators */
void keyboard_indicators FUNC_ARGS;
#define keyboard_indicators {keyboard_indicators, "kbd_indicators", \
	sizeof(Display *)}


/* keymap */
void keymap FUNC_ARGS;
#define keymap {keymap, "keymap", sizeof(Display *)}


/* load_avg */
void load_avg FUNC_ARGS;
#define load_avg {load_avg, "load_avg", 0}


/* netspeeds */
void netspeed_rx FUNC_ARGS;
#define netspeed_rx {netspeed_rx, "netspeed_rx", sizeof(uintmax_t)}

void netspeed_tx FUNC_ARGS;
#define netspeed_tx {netspeed_tx, "netspeed_tx", sizeof(uintmax_t)}


/* num_files */
void num_files FUNC_ARGS;
#define num_files {num_files, "num_files", 0}


/* ram */
void ram_free FUNC_ARGS;
#define ram_free {ram_free, "ram_free", 0}

void ram_perc FUNC_ARGS;
#define ram_perc {ram_perc, "ram_percentage", 0}

void ram_total FUNC_ARGS;
#define ram_total {ram_total, "ram_total", 0}

void ram_used FUNC_ARGS;
#define ram_used {ram_used, "ram_used", 0}


/* run_command */
void run_command FUNC_ARGS;
#define run_command {run_command, "cmd", 0}
/*
 * `cmd` thread name hardcodet to be used to run shell commands
 */


/* swap */
void swap_free FUNC_ARGS;
#define swap_free {swap_free, "swap_free", 0}

void swap_perc FUNC_ARGS;
#define swap_perc {swap_perc, "swap_percentage", 0}

void swap_total FUNC_ARGS;
#define swap_total {swap_total, "swap_total", 0}

void swap_used FUNC_ARGS;
#define swap_used {swap_used, "swap_used", 0}


/* temperature */
void temp FUNC_ARGS;
#define temp {temp, "temperature", LINUX * sizeof(char) * PATH_MAX}


/* uptime */
void uptime FUNC_ARGS;
#define uptime {uptime, "uptime", 0}


/* user */
void gid FUNC_ARGS;
#define gid {gid, "gid", 0}

void uid FUNC_ARGS;
#define uid {uid, "uid", 0}

void username FUNC_ARGS;
#define username {username, "username", 0}


/* volume */
#include "components/volume.h"
void vol_perc FUNC_ARGS;
#define vol_perc {vol_perc, "volume", VOLUME_STATIC_SIZE}


/* wifi */
void wifi_perc FUNC_ARGS;
#define wifi_perc {wifi_perc, "wifi_percentage", 0}

void wifi_essid FUNC_ARGS;
#define wifi_essid {wifi_essid, "wifi_essid", 0}
