#
# This makefile is intended for use with dmake(1) on Apple IIGS
#
#	Created by Dave Tribby, August 1997
#	Modified by Evan Day, September 1997
#
# $Id: Makefile,v 1.2 1997/10/05 16:41:59 gdr Exp $

# Program name
PROG=	cmp
# Source files
SRCS=	cmp.c misc.c rcmp.c special.c

# If optimization wasn't set on the command line use 79, since
# cmp has been tested to work at that level.
.IF $(OPTIMIZE) == $(NULL)
	OPTIMIZE=79
.END

# Current implementation is using ~1050 bytes give or take
STACK	= 1280

# Installation point
BINDIR	= /bin

.INCLUDE : /src/gno/prog.mk

#
# Additional dependancies
#
cmp.o::	extern.h
misc.o::	extern.h
rcmp.o::	extern.h
special.o::	extern.h
