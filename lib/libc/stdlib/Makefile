#
# gno/lib/libc/stdlib/Makefile
#
# Devin Reade, 1997
#
# $Id: Makefile,v 1.4 1997/10/03 04:44:25 gdr Exp $
#

.INCLUDE:	../../../paths.mk
.INCLUDE: 	../../const.mk

OBJS	= cvt.o environ.o fpspecnum.o getopt.o getsubopt.o

CFLAGS	+= -O78

build .PHONY:	../libc

obj:	$(OBJS)

.INCLUDE:	../rules.mk

fpspecnum.o::	fpspecnum.mac
