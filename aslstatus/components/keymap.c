/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#include "../util.h"

static inline bool
valid_layout_or_variant(char *sym)
{
	size_t i;
	/* invalid symbols from xkb rules config */
	static const char *invalid[] = { "evdev", "inet", "pc", "base" };

	for (i = 0; i < LEN(invalid); i++)
		if (!strcmp(sym, invalid[i]))
			return false;

	return true;
}

static inline char *
get_layout(char *syms, int grp_num)
{
	char *tok, *layout;
	int grp;

	layout = NULL;
	tok = strtok(syms, "+:");
	for (grp = 0; tok && grp <= grp_num; tok = strtok(NULL, "+:")) {
		if (!valid_layout_or_variant(tok)) {
			continue;
		} else if (strlen(tok) == 1 && isdigit(tok[0])) {
			/* ignore :2, :3, :4 (additional layout groups) */
			continue;
		}
		layout = tok;
		grp++;
	}

	return layout;
}

void
keymap(char *layout, const char __unused *_a, unsigned int __unused _i,
	void *static_ptr)
{
	XEvent e;
	char *symbols;
	XkbDescRec *desc;
	XkbStateRec state;
	Display **dpy = static_ptr;

	if (!*dpy) {
		if (!(*dpy = XOpenDisplay(NULL))) {
			warn("XOpenDisplay: Failed to open display");
			ERRRET(layout);
		}
		XkbSelectEventDetails(*dpy, XkbUseCoreKbd, XkbStateNotify,
				XkbGroupStateMask, XkbGroupStateMask);
	} else {
		XNextEvent(*dpy, &e);
	}

	if (!(desc = XkbAllocKeyboard())) {
		warn("XkbAllocKeyboard: Failed to allocate keyboard");
		goto end;
	}
	if (XkbGetNames(*dpy, XkbSymbolsNameMask, desc)) {
		warn("XkbGetNames: Failed to retrieve key symbols");
		goto end;
	}
	if (XkbGetState(*dpy, XkbUseCoreKbd, &state)) {
		warn("XkbGetState: Failed to retrieve keyboard state");
		goto end;
	}
	if (!(symbols = XGetAtomName(*dpy, desc->names->symbols))) {
		warn("XGetAtomName: Failed to get atom name");
		goto end;
	}

	bprintf(layout, "%s", get_layout(symbols, state.group));

	XFree(symbols);
end:
	XkbFreeKeyboard(desc, XkbSymbolsNameMask, 1);
}
