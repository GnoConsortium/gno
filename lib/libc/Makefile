#
# gno/lib/libc/Makefile
#
# This is the top-level makefile for libc, GNO v2.0.6
#
# dmake is too bloated right now to due recursive makes well, so before
# you run make in this directory, ensure you run make in the following
# directories:
#		gen, gno, locale, stdio, stdlib, stdtime, string, sys
#
# Before 'make' is run in this directory, you must also build ORCALib, 
# since we steal some of our object files from that hierarchy.
#
# Devin Reade, 1997.
#
# $Id: Makefile,v 1.10 1998/03/25 06:05:46 gdr-ftp Exp $
#

LIB	= c

# This actually resides in the ORCALib source tree
SRCS	= assert.asm

# This library goes into /lib.
USE_SLASH_LIB= true

.INCLUDE: /src/gno/lib/lib.mk

# Use cp(1) rather than install(1) here so that we get a newer timestamp.
assert.o: $(ORCA_SRC:s,/src,/obj,)/ORCALib/assert.o
	cp $< $@
