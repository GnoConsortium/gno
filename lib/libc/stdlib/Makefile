#
# gno/lib/libc/stdlib/Makefile
#
# Devin Reade, 1997
#
# $Id: Makefile,v 1.3 1997/09/21 06:21:35 gdr Exp $
#

.INCLUDE:	../../../paths.mk
.INCLUDE: 	../../const.mk

OBJS	= cvt.o environ.o fpspecnum.o getopt.o getsubopt.o

CFLAGS	+= -O78

build:	../libc

obj:	$(OBJS)

.INCLUDE:	../rules.mk

fpspecnum.o::	fpspecnum.mac
