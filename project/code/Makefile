# Makefile for the project

COMPILER_TYPE= gnu

CC=gcc

PROG=	project
SRCS=	program.c kbd.c timer.c kbdhandler.S mouse.c vbe.c video_gr.c rtc.c ser_port.c char_queue.c vector.c ppm.c game.c font.c utils.c menu.c

CCFLAGS= -Wall -O3 -pg

DPADD+=	${LIBDRIVER} ${LIBSYS}
LDADD+= -llm -ldriver -lsys

LDFLAGS+= -L . -pg

MAN=
BINDIR? = /usr/sbin

.include <bsd.prog.mk>
.include <bsd.gcc.mk>

install: 
	cp ../conf/project /etc/system.conf.d