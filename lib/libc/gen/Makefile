#
# Makefile for libc/gen.
#
# $Id: Makefile,v 1.7 1998/02/09 08:25:39 taubert Exp $
#

.INCLUDE:	../../../paths.mk
.INCLUDE: 	../../const.mk

OBJ_ASM	= setjmp.o syslog.o
OBJ_C	= basename.o bmem.o compat.o dirent.o err.o fnmatch.o fts.o \
	  getcwd.o getgrent.o getpass.o getpwent.o getttyent.o hostname.o \
	  oldlog.o popen.o psignal.o pwcache.o scandir.o siglist.o sleep.o \
	  tty.o utime.o

OBJS	= $(OBJ_ASM) $(OBJ_C)

CFLAGS	+= -O78

build .PHONY:	../libc
obj:		$(OBJS)
asm:		$(OBJ_ASM)
c:		$(OBJ_C)

.INCLUDE:	../rules.mk
