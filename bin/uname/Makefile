#	@(#)Makefile	8.1 (Berkeley) 6/6/93
#
# Modified for GNO (Apple IIgs) and dmake(1) by Steve Reeves, January 1998
#
# $Id: Makefile,v 1.3 1998/03/08 17:24:25 gdr-ftp Exp $

PROG=	uname

DEFINES+=	-DNOID

# Default stack size and optimization (can be overridden by cmd line)
#   These values have been tested and certified as working
STACK		*= 1024
OPTIMIZE	*= 127

.INCLUDE:	/src/gno/prog.mk
