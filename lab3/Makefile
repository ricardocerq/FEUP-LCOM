# Makefile for lab3

COMPILER_TYPE= gnu

CC=gcc

PROG=	lab3
SRCS=	lab3.c kbd.c test3.c timer.c kbdhandler.S

CCFLAGS= -Wall

DPADD+=	${LIBDRIVER} ${LIBSYS}
LDADD+= -ldriver -lsys

LDFLAGS+= -L .

MAN=

.include <bsd.prog.mk>
.include <bsd.gcc.mk>

install: 
	cp `pwd`/conf/lab3 /etc/system.conf.d