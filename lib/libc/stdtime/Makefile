#
# Makefile for libc/stdtime.
#
# $Id: Makefile,v 1.4 2012/08/26 02:54:59 gdr Exp $
#

LIB	= c
LIBPFX	= $(OBJ_DIR)../
NO_REZ	= true
# SRCS	= asctime.c difftime.c localtime.c strftime.c
SRCS	= localtime.c strftime.c

DEBUG	= 25
OPTIMIZE= 0
CFLAGS	+= -v

#default:
#	@echo do not use this directory yet

.INCLUDE: /src/gno/lib/lib.mk

asctime.o:	private.h
difftime.o:	private.h
localtime.o:	private.h
strftime.o:	private.h
