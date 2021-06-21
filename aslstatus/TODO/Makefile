CC ?= clang
LDLIBS = -lpulse
WARN = -Wall -Wextra -pedantic -Wno-unused-parameter

all: pulsedemo

pulsedemo: pulse.c
	$(CC) ${WARN} -O3 -fPIC ${LDLIBS} $< -o $@

clean:
	rm -f pulsedemo
