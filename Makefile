.POSIX:
.PHONY: all install uninstall clean

VERSION = 0.4

PREFIX = /usr/local
CC = cc
CFLAGS = -Wall -Wextra -DVERSION=\"$(VERSION)\"
LDLIBS = -lm

all: muth
install: muth
	cp -f muth $(DESTDIR)/$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)/$(PREFIX)/bin/muth

muth: muth.c

clean:
	rm -f muth muth.core muth.o a.out
