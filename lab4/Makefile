# Makefile for lab4

COMPILER_TYPE= gnu

CC=gcc

PROG=	lab4
SRCS=	lab4.c kbd.c test4.c timer.c kbdhandler.S mouse.c vector.c

CCFLAGS= -Wall

DPADD+=	${LIBDRIVER} ${LIBSYS}
LDADD+= -ldriver -lsys

LDFLAGS+= -L .

MAN=

.include <bsd.prog.mk>
.include <bsd.gcc.mk>

install: 
	cp `pwd`/conf/lab4 /etc/system.conf.d