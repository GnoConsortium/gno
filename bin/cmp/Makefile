#
# This makefile is intended for use with dmake(1) on Apple IIGS
#
#	Created by Dave Tribby, August 1997
#	Modified by Evan Day, September 1997
#
# $Id: Makefile,v 1.3 1998/02/19 02:05:06 gdr-ftp Exp $

PROG=	cmp
SRCS=	cmp.c misc.c rcmp.c special.c

# If optimization wasn't set on the command line use 79, since
# cmp has been tested to work at that level.
OPTIMIZE *= 79

# Current implementation is using ~1050 bytes give or take
STACK	= 1280

.INCLUDE : /src/gno/prog.mk

#
# Additional dependancies
#
cmp.o:		extern.h
misc.o:		extern.h
rcmp.o:		extern.h
special.o:	extern.h
