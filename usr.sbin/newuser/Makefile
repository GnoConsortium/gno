#
# $Id: Makefile,v 1.1 1999/02/15 00:22:10 gdr-ftp Exp $
#

PROG	= newuser
SRCS	= newuser.c
LDADD	= -lcrypt -lcontrib
DEBUG	= 25
OPTIMIZE= 0
CFLAGS	+= -D__STACK_CHECK__
STACK	= 4096		# observed: 940
BINDIR	= /usr/sbin
CHAPTER	= 8

.INCLUDE: /src/gno/prog.mk
