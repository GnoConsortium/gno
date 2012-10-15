#
# Makefile for libc/gen.
#
# $Id: Makefile,v 1.12 1999/01/04 05:10:36 gdr-ftp Exp $
#

LIB	= c
LIBPFX	= $(OBJ_DIR)../
NO_REZ	= true
SRC_ASM	= setjmp.asm
SRC_C	= basename.c compat.c dirent.c err.c errlist.c fnmatch.c fts.c \
	  getcwd.c getgrent.c getlogin.c getpass.c getpwent.c \
	  getttyent.c hostname.c \
	  popen.c psignal.c pwcache.c scandir.c siglist.c sleep.c \
	  syslog.c tty.c uname.c unvis.c utime.c vis.c

SRCS	= $(SRC_ASM) $(SRC_C)

.INCLUDE: /src/gno/lib/lib.mk
