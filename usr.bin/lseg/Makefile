#
# This makefile is intended for use with dmake(1) on Apple IIGS
#
# $Id: Makefile,v 1.4 1997/09/28 16:41:12 gdr Exp $
#
#	Created by Dave Tribby, September 1997

# Program name
PROG	= lseg

# Default stack size and optimization (can be overridden by cmd line)
#   These values have been tested and certified as working
.IF $(STACK) == $(NULL)
	STACK	= 1024
.END
.IF $(OPTIMIZE) == $(NULL)
	OPTIMIZE	= -1
.END
  
# Delivery directory
BINDIR	= /usr/bin

.INCLUDE : /src/gno/prog.mk
