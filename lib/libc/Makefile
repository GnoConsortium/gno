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
# Devin Reade, 1997.
#
# $Id: Makefile,v 1.5 1997/09/21 16:28:30 gdr Exp $
#

.INCLUDE: ../../paths.mk
.INCLUDE: ../const.mk

ORCALIB_OBJ	= \
	$(ORCA_SRC)/orcalib/obj/assert.a

libc:
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $(ORCALIB_OBJ)
