#
# This makefile is intended for use with dmake(1) on Apple IIGS
# $Id: Makefile,v 1.4 1998/04/22 05:22:35 gdr-ftp Exp $
#
#	Created by Dave Tribby, December 1997

PROG=	cksum
SRCS=	cksum.c crc.c print.c sum1.c sum2.c crc32.c
BINDIR=	/usr/bin

HAS_MKSO_DATA	= true
HAS_MKLINK_DATA	= true

.INCLUDE : /src/gno/prog.mk

# Under UNIX, cksum whould be linked to sum (and cksum.1 to sum.1)
# For GNO, just copy the program. (Need to invoke "dmake sum")
sum: cksum
	cp cksum sum

#
# Additional dependancies
#
cksum.o:	extern.h
print.o:	extern.h
