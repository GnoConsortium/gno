#
# This makefile is intended for use with dmake(1) on Apple IIGS
#
# $Id: Makefile,v 1.3 1997/09/30 05:14:47 gdr Exp $
#

# Program name
PROG	= catrez

# Delivery directory
BINDIR	= /usr/bin

# If optimization wasn't set on the command line use 95, since
# catrez has been tested to work at that level.
.IF $(OPTIMIZE) == $(NULL)
	OPTIMIZE=95
.END

# If stack size wasn't set on the command line use 1024, since
# catrez has been tested to work at that size.
.IF $(STACK) == $(NULL)
	STACK	= 1024
.END


.INCLUDE:	/src/gno/prog.mk
