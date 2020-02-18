# Makefile
LDFLAGS= -Wall -lncurses -lm

all: uttt

uttt: uttt.c AI.c cli.c board.c
	cc uttt.c AI.c cli.c board.c -o uttt ${LDFLAGS}
	

clean:
	rm -rf *.o uttt

install: all
	mkdir -p /usr/bin
	cp -f uttt /usr/bin/
	chmod 755 /usr/bin/uttt
