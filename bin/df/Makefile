#
# This makefile is intended for use with dmake(1) on Apple IIGS
#
#	Created by Dave Tribby, August 1997
#	Modified by Evan Day, September 1997
#
# $Id: Makefile,v 1.2 1998/02/19 02:12:18 gdr-ftp Exp $

PROG=	df
SRCS=	df.c gnodf.c

# If optimization wasn't set on the command line use 79, since
# df has been tested to work at that level.
OPTIMIZE *= 79

# Current implementation is using 960 bytes give or take
STACK	= 1024

.INCLUDE : /src/gno/prog.mk

#
# Additional dependancies
#
df.o:		df.h
gnodf.o:	df.h
