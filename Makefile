.PHONY: all install uninstall clean
.POSIX:
CC = cc
CFLAGS = -Wall
LDLIBS = -lm

all: muc
install: muc
	cp -f muc $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/muc

muc: muc.c muc.h

clean:
	rm -f muc
