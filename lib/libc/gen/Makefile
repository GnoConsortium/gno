#
# Makefile for libc/gen.
#
# $Id: Makefile,v 1.8 1998/02/09 08:44:12 taubert Exp $
#

LIB=	c
LIBPFX=	$(OBJ_DIR)../
OBJ_ASM=setjmp.o syslog.o
OBJ_C=	basename.o bmem.o compat.o dirent.o err.o fnmatch.o fts.o \
	getcwd.o getgrent.o getpass.o getpwent.o getttyent.o hostname.o \
	oldlog.o popen.o psignal.o pwcache.o scandir.o siglist.o sleep.o \
	tty.o utime.o

OBJS=	$(OBJ_ASM) $(OBJ_C)

.INCLUDE: /src/gno/lib/lib.mk
