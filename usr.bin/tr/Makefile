#
# This makefile is intended for use with dmake(1) on Apple IIGS
#
#	Created by Dave Tribby, August 1997
#
# $Id: Makefile,v 1.2 1997/09/24 06:21:24 gdr Exp $

# Program name
PROG=	tr
# Source files
SRCS=	tr.c str.c

# If optimization wasn't set on the command line use 79, since
# tr has been tested to work at that level.
.IF $(OPTIMIZE) == $(NULL)
	OPTIMIZE=79
.END

# Current implementation is using 810 bytes
STACK	= 1024

.INCLUDE : /src/gno/prog.mk

#
# Additional dependancies
#
tr.o::	extern.h
str.o::	extern.h
