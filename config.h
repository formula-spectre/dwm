/* See LICENSE file for copyright and license details. */

/* appearance */
static unsigned int borderpx  = 2;        /* border pixel of windows */
static unsigned int snap      = 4;        /* snap pixel */
static unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static unsigned int systrayspacing = 2;   /* systray spacing */
static int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static int showsystray        = 1;     /* 0 means no systray */
static int showbar            = 1;        /* 0 means no bar */
static int topbar             = 1;        /* 0 means bottom bar */
static char *fonts[]          = { "Iosevka Nerd Font:size=10:antialias=true:autohint=true" };
static char color0[]       = "#161821";
static char color1[]       = "#e27878";
static char color2[]       = "#b4be62";
static char color3[]       = "#e2a478";
static char color4[]       = "#84a0c6";
static char color5[]       = "#a093c7";
static char color6[]       = "#89b8c2";
static char color7[]       = "#c6c8d1";

static char *colors[][4]      = {
	/*               fg      bg      border  float  */
	[SchemeNorm] = { color7, color0, color0, color5 },
	[SchemeSel]  = { color0, color7, color2, color3 },
	[SchemeHid]  = { color4, color0 }, /* title bar hidden titles */
	[SchemeLti]  = { color0, color4 }, /* layout indicator */
	[SchemeSts]  = { color0, color4 }, /* status bar */
	[SchemeTgN]  = { color7, color0 }, /* tag normal */
	[SchemeTgS]  = { color0, color4 }, /* tag selected */
};

static int focusonclick            = 0;   /* 1 means focus on click */
static unsigned int fborderpx      = 1;   /* border pixel for floating windows */
static int floatbordercol          = 1;   /* 1 means different border color for floating windows */
static int smartborder             = 1;   /* 0 means no border when monocle mode/one tiled window */
static unsigned int gappih         = 20;  /* horiz inner gap between windows */
static unsigned int gappiv         = 10;  /* vert inner gap between windows */
static unsigned int gappoh         = 10;  /* horiz outer gap between windows and screen edge */
static unsigned int gappov         = 30;  /* vert outer gap between windows and screen edge */
static int smartgaps               = 1;   /* 1 means no outer gap when there is only one window */

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class             instance    title   tags mask  switchtag  iscentered  isfloating   monitor */
	{ "mpv",             NULL,       NULL,   0,         0,         0,          1,           -1 },
	{ "arandr",             NULL,       NULL,   0,         0,         0,          1,           -1 },
};

/* layout(s) */
static float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static int nmaster     = 1;    /* number of clients in master area */
static int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[T]",      tile },    /* first entry is default */
	{ "(F)",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define MODOPT Mod1Mask
#define TAGKEYS(CHAIN,KEY,TAG) \
{ MODKEY,                       CHAIN, KEY,      view,           {.ui = 1 << TAG} }, \
{ MODKEY|ControlMask,           CHAIN, KEY,      toggleview,     {.ui = 1 << TAG} }, \
{ MODKEY|ShiftMask,             CHAIN, KEY,      tag,            {.ui = 1 << TAG} }, \
{ MODKEY|ControlMask|ShiftMask, CHAIN, KEY,      toggletag,      {.ui = 1 << TAG} }, \
{ MODKEY|MODOPT,                CHAIN, KEY,      sendtag,        {.ui = 1 << TAG} }, \
{ MODKEY|MODOPT|ShiftMask,      CHAIN, KEY,      swaptag,        {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } },

/* commands */
static char *termcmd[]    = { "xst", NULL };
static char *plumber[]    = { "sh ", "~/.config/dwm/scripts/plumb.sh", NULL };
static char *slock[]      = { "slock", NULL };
static char *dmenucmd[]   = { "rofi", "-show","run", NULL };
static char *dmusick[]   = { "dmusick", "-c","-l","6", NULL };

/*
 * Xresources preferences to load at startup
 */
ResourcePref resources[] = {
	{ "dwm.color0",		STRING,  &color0 },
	{ "dwm.color1",		STRING,  &color1 },
	{ "dwm.color2",		STRING,  &color2 },
	{ "dwm.color3",		STRING,  &color3 },
	{ "dwm.color4",		STRING,  &color4 },
	{ "dwm.color5",		STRING,  &color5 },
	{ "dwm.color6",		STRING,  &color6 },
	{ "dwm.color7",		STRING,  &color7 },
	{ "dwm.borderpx",	INTEGER, &borderpx },
	{ "dwm.snap",		INTEGER, &snap },
	{ "dwm.showbar",	INTEGER, &showbar },
	{ "dwm.topbar",		INTEGER, &topbar },
	{ "dwm.nmaster",	INTEGER, &nmaster },
	{ "dwm.resizehints",	INTEGER, &resizehints },
	{ "dwm.mfact",		FLOAT,   &mfact },
	{ "dwm.fborderpx",	INTEGER, &fborderpx },
	{ "dwm.focusonclick",	INTEGER, &focusonclick },
	{ "dwm.floatbordercol",	INTEGER, &floatbordercol },
	{ "dwm.smartborder",	INTEGER, &smartborder },
	{ "dwm.gappih",		INTEGER, &gappih },
	{ "dwm.gappiv",		INTEGER, &gappiv },
	{ "dwm.gappoh",		INTEGER, &gappoh },
	{ "dwm.gappov",		INTEGER, &gappov },
	{ "dwm.smartgaps",	INTEGER, &smartgaps },
};

/* To use XF86 keys */
#include <X11/XF86keysym.h>
static Key keys[] = {
	/* modifier			chain	key		function		argument */
        { MODKEY,                       -1,	XK_Return,	spawn,          {.v = termcmd } },
	{ MODKEY,                       -1,     XK_p,	        spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             -1,	XK_r,		quit,           {1} },
	{ MODKEY|ShiftMask,             -1,	XK_c,		killclient,     {0} },
	{ MODKEY|ShiftMask,             -1,	XK_q,		quit,           {0} },

	{ MODKEY,                       -1,	XK_Tab,		swapfocus,      {.i = -1 } },

	{ MODKEY,                       -1,	XK_b,		togglebar,      {0} },
	{ MODKEY,                       -1,	XK_f,		togglefullscr,  {0} },
	{ MODKEY|ShiftMask,             -1,	XK_g,		togglegaps,     {0} },

	{ MODKEY|ShiftMask,             -1,	XK_i,		incnmaster,     {.i = +1 } },
	{ MODKEY|ShiftMask,             -1,	XK_d,		incnmaster,     {.i = -1 } },


	{ MODKEY,                       -1,	XK_h,		focusstack,     {.i = -1 } },
	{ MODKEY,                       -1,	XK_l,		focusstack,     {.i = +1 } },
	{ MODKEY|ShiftMask,             -1,	XK_j,		rotatestack,    {.i = +1 } },
	{ MODKEY|ShiftMask,             -1,	XK_k,		rotatestack,    {.i = -1 } },

	{ MODKEY|ShiftMask,             -1,	XK_h,		setmfact,       {.f = -0.05} },
	{ MODKEY|ShiftMask,             -1,	XK_l,		setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             -1,	XK_h,		setcfact,       {.f = +0.25} },
	{ MODKEY|ShiftMask,             -1,	XK_l,		setcfact,       {.f = -0.25} },

	{ MODKEY,                       -1,	XK_t,		togglefloating, {0} },
	{ MODKEY|ShiftMask,             -1,	XK_t,		setlayout,      {0} },

	{ MODKEY,                       -1,	XK_Left,	shiftviewc,     {.i = -1 } },       // switch to the prev tag containing a client
	{ MODKEY,                       -1,	XK_Right,	shiftviewc,     {.i = +1 } },       // switch to the next tag containing a client
	{ MODKEY|ShiftMask,             -1,	XK_Left,	shifttagc,      {.i = -1 } },       // send focused client to the prev tag containing a client
	{ MODKEY|ShiftMask,             -1,	XK_Right,	shifttagc,      {.i = +1 } },       // send focused client to the next tag containing a client
	{ MODKEY|MODOPT,                -1,	XK_Left,	shiftview,      {.i = -1 } },       // switch to the prev tag
	{ MODKEY|MODOPT,                -1,	XK_Right,	shiftview,      {.i = +1 } },       // switch to the next tag
	{ MODKEY|MODOPT|ShiftMask,      -1,	XK_Left,	shifttag,       {.i = -1 } },       // send client to the prev tag
	{ MODKEY|MODOPT|ShiftMask,      -1,	XK_Right,	shifttag,       {.i = +1 } },       // send client to the next tag

	{ MODKEY,                       -1,	XK_0,		view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             -1,	XK_0,		tag,            {.ui = ~0 } },
	{ MODKEY|ShiftMask,             -1,	XK_p,		spawn,          {.v = dmusick } },
	{ MODKEY,                       -1,	XK_comma,	focusmon,       {.i = -1 } },
	{ MODKEY,                       -1,	XK_period,	focusmon,       {.i = +1 } },
	{ MODKEY|ControlMask|ShiftMask, -1,	XK_comma,	tagmon,         {.i = -1 } },
	{ MODKEY|ControlMask|ShiftMask, -1,	XK_period,	tagmon,         {.i = +1 } },
        { MODKEY,                       -1,	XK_m,    	spawn,          SHCMD("wal -i $(find ~/Pictures/Walls/* | shuf | head -n 1")},
        { Mod1Mask,                     -1,	XK_o,    	spawn,          SHCMD("sh ~/.config/dwm/scripts/search.sh &disown") },
        { Mod1Mask,                     -1,	XK_i,    	spawn,          SHCMD("greenclip-menu &") },
        { ControlMask|Mod1Mask,         -1,	XK_v,    	spawn,          SHCMD("~/.vim-anywhere/bin/run &") },
        { Mod1Mask,                     -1,	XK_l,     	spawn,          {.v = slock} },
	TAGKEYS(                        -1,	XK_1,                      0)
	TAGKEYS(                        -1,	XK_2,                      1)
	TAGKEYS(                        -1,	XK_3,                      2)
	TAGKEYS(                        -1,	XK_4,                      3)
	TAGKEYS(                        -1,	XK_5,                      4)
	TAGKEYS(                        -1,	XK_6,                      5)
	TAGKEYS(                        -1,	XK_7,                      6)
	TAGKEYS(                        -1,	XK_8,                      7)
	TAGKEYS(                        -1,	XK_9,                      8)

	// XF86Keys
        /*
	{ 0,    -1,		XF86XK_AudioMute,               spawn,      SHCMD("amixer -q sset Master toggle") },
	{ 0,    -1,		XF86XK_AudioRaiseVolume,        spawn,      SHCMD("amixer -q sset Master 5%+") },
	{ 0,    -1,		XF86XK_AudioLowerVolume,        spawn,      SHCMD("amixer -q sset Master 5%-") },
	{ 0,    -1,		XF86XK_AudioPrev,               spawn,      SHCMD("mpc -q prev") },
	{ 0,    -1,		XF86XK_AudioNext,               spawn,      SHCMD("mpc -q next") },
	{ 0,    -1,		XF86XK_AudioPlay,               spawn,      SHCMD("mpc -q toggle") },
	{ 0,    -1,		XF86XK_AudioMicMute,            spawn,      SHCMD("amixer -q sset Capture toggle") },
        */
	{ 0,    -1,		XF86XK_MonBrightnessUp,         spawn,      SHCMD("xbacklight -inc 10") },
	{ 0,    -1,		XF86XK_MonBrightnessDown,       spawn,      SHCMD("xbacklight -dec 10") },
	{ 0,    -1,		XK_Print,                       spawn,      SHCMD("flameshot gui") },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {.v = &layouts[0]} },
        { ClkClientWin,         ControlMask,   	Button2,     	spawn,          SHCMD("sh ~/.config/dwm/scripts/plumb.sh &disown") },
	{ ClkLtSymbol,          0,              Button2,        setlayout,      {.v = &layouts[1]} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button1,        showtitlewin,   {0} },
	{ ClkWinTitle,          0,              Button2,        zoomtitlewin,   {0} },
	{ ClkWinTitle,          0,              Button3,        hidetitlewin,   {0} },
	{ ClkWinTitle,          MODKEY,         Button3,        killtitlewin,   {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkClientWin,         MODKEY|ShiftMask, Button3,      dragcfact,      {0} },
	{ ClkClientWin,         MODKEY|ShiftMask, Button1,      dragmfact,      {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} }, { ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} }, };
