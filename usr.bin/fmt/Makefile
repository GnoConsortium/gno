#
# $Id: Makefile,v 1.3 1998/03/08 17:38:21 gdr-ftp Exp $
#

# Program name
PROG	= fmt
SRCS	= fmt.c head.c

# Add -D__STACK_CHECK__	to print, on exit, the amount of stack used
CFLAGS	+= -DBUILD_FMT -DNO_LOCALE
STACK	= 1536
BINDIR	= /usr/bin

.INCLUDE:	/src/gno/prog.mk

head.o:	def.h extern.h
