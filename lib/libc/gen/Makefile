#
# Makefile for libc/gen.
#
# $Id: Makefile,v 1.2 1997/07/27 23:06:00 gdr Exp $
#

.INCLUDE: 	../../const.mk

OBJ_ASM	= crypta.o setjmp.o syslog.o
OBJ_C	= basename.o bmem.o compat.o crypt.o dirent.o err.o fnmatch.o fts.o \
	  getcwd.o getgrent.o getpass.o getpwent.o getttyent.o hostname.o \
	  oldlog.o popen.o pwcache.o scandir.o sleep.o tty.o utime.o
NOT_YET	= fts.o

OBJS	= $(OBJ_ASM) $(OBJ_C)

default:	$(OBJS)
asm:		$(OBJ_ASM)
c:		$(OBJ_C)

.INCLUDE:	../rules.mk
