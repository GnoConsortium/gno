#
# $Id: Makefile,v 1.1 1997/02/28 05:12:57 gdr Exp $
#

.INCLUDE:	../../../const.mk

LDLIBS	= -l ../../libc

CFLAGS	:= -v -w -G25 $(STACK) $(DEFINES) $(INCLUDES)

default: creat exectest stat.o trap1

.PRECIOUS: creat.o exectest.o stat.o trap1.o

.INCLUDE:	../../rules.mk
