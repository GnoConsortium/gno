# Makefile for SANE glue code
# 1997 Soenke Behrens
#
# All paths assume a GNO/ME setup as described in Devin Reade's
# GNO/ME FAQ. For installation under ORCA/Shell, see the README
# file.
#

.INCLUDE .IGNORE: ../../paths.mk

LIBDIR = 13
# Change INCDIR to 13/orcacdefs if you are using the "old-style"
# path naming convention
# Or set to /lang/orca/include if you wish to completely replace
# ORCA/C's sane.h.
INCDIR = /usr/include

# You should not have to modify anything beyond this point

LIB	= lsaneglue

all: sanetest findfpe $(LIB)

sanetest: sanetest.o saneglue.o
	$(RM) saneglue.root
	occ -o sanetest sanetest.o saneglue.o

findfpe: findfpe.o findfpcp.o
	$(RM) findfpcp.root
	occ -o findfpe findfpe.o findfpcp.o

saneglue.mac: saneglue.asm saneglue.macro
	macgen saneglue.asm saneglue.mac saneglue.mac saneglue.macro \
	13:ainclude:m16.sane 13:orcainclude:m16.orca

findfpcp.mac: findfpcp.asm
	macgen findfpcp.asm findfpcp.mac findfpcp.mac \
	13:orcainclude:m16.orca

$(LIB): saneglue.o findfpcp.o
	$(RM) $(LIB)
	makelib $(LIB) +saneglue.o +findfpcp.o
	copyfork $(LIB).r $(LIB)

install:
	cp -f sane.h $(INCDIR)
	cp -f $(LIB) $(LIBDIR)

MI_LIBDIR	= $(RELEASE_DIR)/lib
MI_INCDIR	= $(RELEASE_DIR)/usr/include
MI_MANDIR	= $(RELEASE_DIR)/usr/man/man3
MI_INSTALL	= $(RELEASE_DIR)/install

makeinstall: $(LIB) sane.h saneglue.3 mkso.data
	$(INSTALL) -d $(MI_LIBDIR)
	$(INSTALL) -d $(MI_INCDIR)
	$(INSTALL) -d $(MI_MANDIR)
	$(INSTALL) $(LIB) $(MI_LIBDIR)
	$(INSTALL) sane.h $(MI_INCDIR)
	$(INSTALL) saneglue.3 $(MI_MANDIR)
	$(INSTALL) mkso.data $(MI_INSTALL)/mkso.lsaneglue

findfpcp.o:: findfpcp.mac
saneglue.o:: saneglue.mac
