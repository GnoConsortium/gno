#
# Makefile for libc/stdtime.
#
# $Id: Makefile,v 1.2 1997/10/03 04:44:25 gdr Exp $
#

.INCLUDE:	../../../paths.mk
.INCLUDE: 	../../const.mk

OBJS	= asctime.o difftime.o localtime.o strftime.o

CFLAGS	+= -v

default:
	@echo do not use this directory yet

build .PHONY:	../libc
obj:	$(OBJS)

.INCLUDE:	../rules.mk

asctime.o::	private.h
difftime.o::	private.h
localtime.o::	private.h
strftime.o::	private.h
