#- aslstatus version -#
VERSION := 0.1

CC ?= cc

#- paths -#
PREFIX    := /usr
MANPREFIX := ${PREFIX}/share/man

X11INC := /usr/X11R6/include
X11LIB := /usr/X11R6/lib

#- flags -#
CPPFLAGS += -I${X11INC} -D_DEFAULT_SOURCE
CFLAGS   += -std=c99 -pedantic -Wall -Wextra

#- linker -#
LDFLAGS  += -L${X11LIB} -s
LDLIBS   := -lX11 -lpthread #-lxcb #-Wl,-O3 -Wl,--as-needed
LDALSA   := -lasound
