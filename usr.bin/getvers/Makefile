#
# This makefile is intended for use with dmake(1) on Apple IIGS
# $Id: Makefile,v 1.1 1998/04/29 00:41:45 tribby Exp $
#
#	Created by Dave Tribby, April 1998

# Program name
PROG=	getvers

# Because this is an assembly program, need to avoid .c defaults
SRCS	*= $(PROG).asm
MAIN	*= $(PROG)
MAINSRC	*= $(MAIN).asm

# Delivery directory
BINDIR	= /usr/bin

.INCLUDE : /src/gno/prog.mk
