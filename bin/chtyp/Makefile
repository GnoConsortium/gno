#
# This makefile is intended for use with dmake(1) on Apple IIGS
#
#	Created by Dave Tribby, August 1997
#	Modified by Evan Day, September 1997
#
# $Id: Makefile,v 1.1 1997/10/03 05:06:50 gdr Exp $

# Program name
PROG=	chtyp
# Source files
SRCS=	chtyp.c ftypes.c

# If optimization wasn't set on the command line use 79, since
# chtyp has been tested to work at that level.
.IF $(OPTIMIZE) == $(NULL)
	OPTIMIZE=79
.END

# Current implementation is using 960 bytes give or take
STACK	= 1024

# Installation point
BINDIR	= /bin

.INCLUDE : /src/gno/prog.mk

#
# Additional dependancies
#
chtyp.o::	ftypes.h
ftypes.o::	ftypes.h
