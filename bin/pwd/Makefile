#
# This makefile is intended for use with dmake(1) on Apple IIGS
# $Id: Makefile,v 1.1 1997/10/03 04:02:33 gdr Exp $
#
#	Created by Dave Tribby, September 1997

# Program name
PROG=	pwd

# Default stack size and optimization (can be overridden by cmd line)
#   These values have been tested and certified as working
.IF $(STACK) == $(NULL)
	STACK	= 1024
.END
.IF $(OPTIMIZE) == $(NULL)
	OPTIMIZE	= -1
.END
  
.INCLUDE : /src/gno/prog.mk
