/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#include "../util.h"

/*
 * fmt consists of uppercase or lowercase 'c' for caps lock and/or 'n' for num
 * lock, each optionally followed by '?', in the order of indicators desired.
 * If followed by '?', the letter with case preserved is included in the output
 * if the corresponding indicator is on.  Otherwise, the letter is always
 * included, lowercase when off and uppercase when on.
 */
void
keyboard_indicators(char *out, const char *fmt,
	unsigned int __unused _i, void *static_ptr)
{
	size_t
		i, n,
		fmtlen;
	int
		isset,
		togglecase;

	XEvent e;
	char key;
	XKeyboardState state;
	Display **dpy = static_ptr;

	if (!*dpy) {
		if (!(*dpy = XOpenDisplay(NULL))) {
			warn("XOpenDisplay: Failed to open display");
			ERRRET(out);
		}
		XkbSelectEventDetails(*dpy, XkbUseCoreKbd,
				XkbIndicatorStateNotify, XkbAllEventsMask,
				XkbAllEventsMask);
	} else {
		XNextEvent(*dpy, &e);
	}

	XGetKeyboardControl(*dpy, &state);

	fmtlen = strnlen(fmt, 4);
	for (i = n = 0; i < fmtlen; i++) {
		key = tolower(fmt[i]);
		if (key != 'c' && key != 'n') {
			continue;
		}
		togglecase = (i + 1 >= fmtlen || fmt[i + 1] != '?');
		isset = (state.led_mask & (1 << (key == 'n')));
		if (togglecase) {
			out[n++] = isset ? toupper(key) : key;
		} else if (isset) {
			out[n++] = fmt[i];
		}
	}
	out[n] = '\0';
}
