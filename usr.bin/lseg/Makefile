#
# This makefile is intended for use with dmake(1) on Apple IIGS
#
# $Id: Makefile,v 1.2 1997/09/21 22:05:58 gdr Exp $
#

# Program name
PROG	= lseg

# Default stack size and optimization (can be overridden by cmd line)
.IF $(STACK) == $(NULL)
	STACK	= 1024
.END
##.IF $(STACK) == $(NULL)
##	OPTIMIZE	= -1
##.END
  
# Delivery directory
BINDIR	= /usr/bin

.INCLUDE : /src/gno/gno.prog.mk
