#
# This makefile is intended for use with dmake(1) on Apple IIGS
# $Id: Makefile,v 1.1 1998/01/16 04:16:10 gdr Exp $
#
#	Created by Dave Tribby, December 1997

PROG=	cksum
SRCS=	cksum.c crc.c print.c sum1.c sum2.c crc32.c

CFLAGS	= -v
.INCLUDE : /src/gno/prog.mk

# Under UNIX, chsum whould be linked to sum (and cksum.1 to sum.1)
# For GNO, just copy the program. (Need to invoke "dmake sum")
sum: cksum
	cp cksum sum

#
# Additional dependancies
#

cksum.o::	extern.h
print.o::	extern.h
