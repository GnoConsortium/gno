#
# This makefile is intended for use with dmake(1) on Apple IIGS
# $Id: Makefile,v 1.2 1998/03/08 17:46:30 gdr-ftp Exp $
#
#	Created by Dave Tribby, October 1997

# Default stack size (can be overridden by cmd line)
#   This value have been tested and certified as working
STACK	*= 1024

# Program name
PROG=	sed

# Source files
SRCS=	main.c compile.c misc.c process.c

.INCLUDE : /src/gno/prog.mk

#
# Additional dependancies
#
sed.o:		extern.h defs.h
compile.o:	extern.h defs.h
misc.o:		extern.h defs.h
process.o:	extern.h defs.h

  
