# Makefile for SANE glue code
# 1997 Soenke Behrens

# All paths assume a GNO/ME setup as described in Devin Reade's
# GNO/ME FAQ. For installation under ORCA/Shell, see the README
# file

LIBDIR = 13
# Change INCDIR to 13/orcacdefs if you are using the "old-style"
# path naming convention
# Or set to /lang/orca/include if you wish to completely replace
# ORCA/C's sane.h.
INCDIR = /usr/include

# You should not have to modify anything beyond this point

all: sanetest findfpe lsaneglue

sanetest: sanetest.o saneglue.o
	$(RM) saneglue.root
	occ -o sanetest sanetest.o saneglue.o

findfpe: findfpe.o findfpcp.o
	$(RM) findfpcp.root
	occ -o findfpe findfpe.o findfpcp.o

saneglue.o:: saneglue.mac

saneglue.mac: saneglue.asm saneglue.macro
	macgen saneglue.asm saneglue.mac saneglue.mac saneglue.macro \
13:ainclude:m16.sane 13:orcainclude:m16.orca

findfpcp.o:: findfpcp.mac

findfpcp.mac: findfpcp.asm
	macgen findfpcp.asm findfpcp.mac findfpcp.mac \
13:orcainclude:m16.orca

lsaneglue: saneglue.o findfpcp.o
	$(RM) lsaneglue
	makelib lsaneglue +saneglue.o +findfpcp.o
	copyfork lsaneglue.r lsaneglue

install:
	cp -f sane.h $(INCDIR)
	cp -f lsaneglue $(LIBDIR)
