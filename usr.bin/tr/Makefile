#
# This makefile is intended for use with dmake(1) on Apple IIGS
# $Id: Makefile,v 1.4 1998/03/08 17:49:40 gdr-ftp Exp $
#
#	Created by Dave Tribby, August 1997

# Program name
PROG=	tr
# Source files
SRCS=	tr.c str.c

# If optimization wasn't set on the command line use 79, since
# tr has been tested to work at that level.
OPTIMIZE	*= 79

.INCLUDE : /src/gno/prog.mk

#
# Additional dependancies
#
tr.o:	extern.h
str.o:	extern.h
