#
# Makefile for libc/gen.
#
# $Id: Makefile,v 1.11 1998/06/24 04:19:57 gdr-ftp Exp $
#

LIB	= c
LIBPFX	= $(OBJ_DIR)../
NO_REZ	= true
SRC_ASM	= setjmp.asm
SRC_C	= basename.c bmem.c compat.c dirent.c err.c fnmatch.c fts.c \
	  getcwd.c getgrent.c getlogin.c getpass.c getpwent.c \
	  getttyent.c hostname.c \
	  oldlog.c popen.c psignal.c pwcache.c scandir.c siglist.c sleep.c \
	  syslog.c tty.c uname.c utime.c

SRCS	= $(SRC_ASM) $(SRC_C)

.INCLUDE: /src/gno/lib/lib.mk
