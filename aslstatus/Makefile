#- aslstatus - async suckless status monitor -#

include config.mk


COMPONENTS  := $(wildcard components/*.c)
OBJ         := ${COMPONENTS:.c=.o}

NEED_VOLUME := components/volume.o aslstatus.o

ALSA        := 1  # alsa used by default, comment out to use `sys/soundcard.h`


${OBJ}: util.h
aslstatus.o: aslstatus.c aslstatus.h config.h util.h os.h

ifdef ALSA
LDFLAGS += ${LDALSA}
${NEED_VOLUME}: CPPFLAGS += -DUSE_ALSA
endif
${NEED_VOLUME}: components/volume.h


.PHONY: all
all: aslstatus

${OBJ} util.o: %.o: %.c
	$(CC) -o $@ -c $< ${CFLAGS} ${CPPFLAGS}

aslstatus: aslstatus.o util.o ${OBJ}
	$(CC) -o $@ ${LDFLAGS} ${CFLAGS} ${CPPFLAGS} ${LDLIBS} $^

.PHONY: install
install: all
	mkdir -p "${DESTDIR}${PREFIX}/bin"
	cp -f aslstatus "${DESTDIR}${PREFIX}/bin"
	chmod 755 "${DESTDIR}${PREFIX}/bin/aslstatus"
	mkdir -p "${DESTDIR}${MANPREFIX}/man1"
	cp -f aslstatus.1 "${DESTDIR}${MANPREFIX}/man1"
	chmod 644 "${DESTDIR}${MANPREFIX}/man1/aslstatus.1"

.PHONY: clean
clean:
	rm -f aslstatus aslstatus.o util.o ${OBJ}

.PHONY: uninstall
uninstall:
	rm -f "${DESTDIR}${PREFIX}/bin/aslstatus"
	rm -f "${DESTDIR}${MANPREFIX}/man1/aslstatus.1"
