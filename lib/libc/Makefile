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
# $Id: Makefile,v 1.8 1998/02/09 08:44:06 taubert Exp $
#

LIB=	c
# assert.a must be copied from $(ORCA_SRC)/ORCALib/obj into $(OBJ_DIR)
OBJS=	assert.a

# This library goes into /lib.
USE_SLASH_LIB= true

.INCLUDE: /src/gno/lib/lib.mk

$(LIBTARGET):: lib$(LIB).r
	$(CATREZ) -d $@ $<
