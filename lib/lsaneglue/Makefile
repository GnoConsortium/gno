# Makefile for SANE glue code
# 1997 Soenke Behrens
#
# All paths assume a GNO/ME setup as described in Devin Reade's
# GNO/ME FAQ. For installation under ORCA/Shell, see the README
# file.
#

.INCLUDE .IGNORE: ../../paths.mk

LIBDIR = /lib
# Change INCDIR to 13/orcacdefs if you are using the "old-style"
# path naming convention.
# Or set to /lang/orca/libraries/orcacdefs if you wish to completely
# replace ORCA/C's sane.h.
INCDIR = /usr/include

# This is the directory containing the Apple and ByteWorks macro include
# file subdirectories.  It is critical that ':' be used as the path
# delimiter.
IDIR	= :lang:orca:libraries

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
	macgen -P saneglue.asm saneglue.mac saneglue.macro \
	$(IDIR):ainclude:m16.sane $(IDIR):orcainclude:m16.orca

findfpcp.mac: findfpcp.asm
	macgen -P findfpcp.asm findfpcp.mac \
	$(IDIR):orcainclude:m16.orca

$(LIB): saneglue.o findfpcp.o $(LIB).r
	$(RM) $(LIB)
	makelib -P $(LIB) +saneglue.o +findfpcp.o
	catrez -d $@ $(LIB).r

install:
	install -d $(INCDIR) $(LIBDIR)
	install sane.h $(INCDIR)
	install $(LIB) $(LIBDIR)
	chtyp -lcc $(INCDIR)/sane.h

MI_LIBDIR	= $(RELEASE_DIR)/lib
MI_INCDIR	= $(RELEASE_DIR)/usr/include
MI_MANDIR	= $(RELEASE_DIR)/usr/man/man3
MI_INSTALL	= $(RELEASE_DIR)/install

release: $(LIB) sane.h saneglue.3 mkso.data
	install -d $(MI_LIBDIR) $(MI_INCDIR) $(MI_MANDIR) $(MI_INSTALL)
	install $(LIB) $(MI_LIBDIR)
	install sane.h $(MI_INCDIR)
	install saneglue.3 $(MI_MANDIR)
	install mkso.data $(MI_INSTALL)/mkso.lsaneglue
	chtyp -lcc $(MI_INCDIR)/sane.h

# Implicit rule to handle Rez source on case sensitive Appleshare servers
.IMPORT .IGNORE: TMPDIR
.IF $(TMPDIR) == $(NULL)
TMPDIR		= 14
.END
.IF $(APPLESHARE_CASE_SENSITIVE) != $(NULL)
%.r : %.rez
	$(INSTALL) $< $(TMPDIR)/$<
	$(REZ) -o $@ -c $(REZFLAGS) $(TMPDIR)/$<
	$(RM) -f $(TMPDIR)/$<
.END

findfpcp.o:: findfpcp.mac
saneglue.o:: saneglue.mac
