#	@(#)Makefile	8.1 (Berkeley) 6/6/93
#
# Modified for GNO (Apple IIgs) and dmake(1) by Steve Reeves, January 1998
#
# $Id: Makefile,v 1.2 1998/02/19 02:24:35 gdr-ftp Exp $

PROG=	uname

DEFINES+=	-DNOID

# Default stack size and optimization (can be overridden by cmd line)
#   These values have been tested and certified as working
.IF $(STACK) == $(NULL)
STACK=	1024
.END
.IF $(OPTIMIZE) == $(NULL)
OPTIMIZE=	127
.END

.INCLUDE:	/src/gno/prog.mk
