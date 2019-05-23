# Makefile
LDFLAGS=-lncurses -lm

all: uttt

clean:
	rm -f uttt

install: all
	mkdir -p /usr/bin
	cp -f uttt /usr/bin/
	chmod 755 /usr/bin/uttt
