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
# $Id: Makefile,v 1.7 1998/02/08 03:47:22 gdr-ftp Exp $
#

LIB	= c

.INCLUDE: ../../paths.mk
.INCLUDE: ../const.mk

USE_SLASH_LIB	= true			# This library goes into /lib.

ORCALIB_OBJ	= \
	$(ORCA_SRC)/orcalib/obj/assert.a

libc:	$(ORCALIB_OBJ)
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $^

$(ORCALIB_OBJ):;	true

.INCLUDE:		../librelease.mk

$(LIBTARGET):: lib$(LIB).r
	$(CATREZ) -d $@ lib$(LIB).r

