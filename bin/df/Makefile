#
# This makefile is intended for use with dmake(1) on Apple IIGS
#
#	Created by Dave Tribby, August 1997
#	Modified by Evan Day, September 1997
#
# $Id: Makefile,v 1.1 1997/10/03 05:42:18 gdr Exp $

# Program name
PROG=	df
# Source files
SRCS=	df.c gnodf.c

# If optimization wasn't set on the command line use 79, since
# df has been tested to work at that level.
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
tr.o::	df.h
str.o::	df.h
