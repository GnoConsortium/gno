#
# gno/lib/libc/sys/Makefile
#
# Devin Reade, 1997.
#
# $Id: Makefile,v 1.2 1997/09/05 06:35:22 gdr Exp $
#

.INCLUDE:	../../../paths.mk
.INCLUDE: 	../../const.mk

OBJS	= trap.o syscall.o exec.o

build:		$(OBJS)

clean clobber:
	$(RM) -f $(OBJS)

.INCLUDE:	../rules.mk

trap.o::	trap.mac
