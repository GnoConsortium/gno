#	@(#)Makefile	8.1 (Berkeley) 6/6/93
#
# Modified for GNO (Apple IIgs) and dmake(1) by Steve Reeves, January 1998
#
# $Id: Makefile,v 1.2 1998/03/08 17:36:27 gdr-ftp Exp $

PROG=	colcrt

BINDIR=	/usr/bin

DEFINES+=	-DNOID

# Default stack size and optimization (can be overridden by cmd line)
#   These values have been tested and certified as working
STACK		*= 1024
OPTIMIZE	*= 127

.INCLUDE:	/src/gno/prog.mk
