#
# gno/lib/libc/sys/Makefile
#
# Devin Reade, 1997.
#
# $Id: Makefile,v 1.4 1997/10/03 04:44:26 gdr Exp $
#

.INCLUDE:	../../../paths.mk
.INCLUDE: 	../../const.mk

OBJS	= exec.o syscall.o trap.o
CFLAGS	+= -O78

build .PHONY:	../libc
obj:	$(OBJS)

.INCLUDE:	../rules.mk

trap.o::	trap.mac
