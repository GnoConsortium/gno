#
# Makefile for libc/gen.
#
# $Id: Makefile,v 1.10 1998/03/24 16:10:35 gdr-ftp Exp $
#

LIB	= c
LIBPFX	= $(OBJ_DIR)../
NO_REZ	= true
SRC_ASM	= setjmp.asm syslog.asm
SRC_C	= basename.c bmem.c compat.c dirent.c err.c fnmatch.c fts.c \
	  getcwd.c getgrent.c getlogin.c getpass.c getpwent.c \
	  getttyent.c hostname.c \
	  oldlog.c popen.c psignal.c pwcache.c scandir.c siglist.c sleep.c \
	  tty.c uname.c utime.c

SRCS	= $(SRC_ASM) $(SRC_C)

.INCLUDE: /src/gno/lib/lib.mk
