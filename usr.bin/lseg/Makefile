#
# This makefile is intended for use with dmake(1) on Apple IIGS
#
# $Id: Makefile,v 1.3 1997/09/26 06:36:22 gdr Exp $
#

# Program name
PROG	= lseg

# Default stack size and optimization (can be overridden by cmd line)
.IF $(STACK) == $(NULL)
	STACK	= 1024
.END

# Delivery directory
BINDIR	= /usr/bin

.INCLUDE:	/src/gno/prog.mk
