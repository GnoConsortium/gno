#
# $Id: Makefile,v 1.2 1997/10/30 03:49:46 gdr Exp $
#

# Program name
PROG	= fmt
SRCS	= fmt.c head.c

# Add -D__STACK_CHECK__	to print, on exit, the amount of stack used
CFLAGS	+= -DBUILD_FMT -DNO_LOCALE
STACK	= 1536
BINDIR	= /usr/bin

.INCLUDE:	/src/gno/prog.mk

head.o::	def.h extern.h
