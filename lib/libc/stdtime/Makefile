#
# Makefile for libc/stdtime.
#
# $Id: Makefile,v 1.3 1998/02/09 08:45:04 taubert Exp $
#

LIB=	c
LIBPFX=	$(OBJ_DIR)../
OBJS=	asctime.o difftime.o localtime.o strftime.o

CFLAGS	+= -v

default:
	@echo do not use this directory yet

.INCLUDE: /src/gno/lib/lib.mk

asctime.o::	private.h
difftime.o::	private.h
localtime.o::	private.h
strftime.o::	private.h
