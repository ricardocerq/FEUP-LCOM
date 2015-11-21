# Makefile for lab5

COMPILER_TYPE= gnu

CC=gcc

PROG=	lab5
SRCS=	lab5.c kbd.c test5.c timer.c kbdhandler.S mouse.c vector.c vbe.c video_gr.c read_xpm.c

CCFLAGS= -Wall

DPADD+=	${LIBDRIVER} ${LIBSYS}
LDADD+= -llm -ldriver -lsys

LDFLAGS+= -L .

MAN=

.include <bsd.prog.mk>
.include <bsd.gcc.mk>

install: 
	cp `pwd`/conf/lab5 /etc/system.conf.d