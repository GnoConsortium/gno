#
# gno/lib/libc/sys/Makefile
#
# Devin Reade, 1997.
#
# $Id: Makefile,v 1.3 1997/09/21 16:15:29 gdr Exp $
#

.INCLUDE:	../../../paths.mk
.INCLUDE: 	../../const.mk

OBJS	= exec.o syscall.o trap.o
CFLAGS	+= -O78

build:	../libc
obj:	$(OBJS)

.INCLUDE:	../rules.mk

trap.o::	trap.mac
