#
# $Id: Makefile,v 1.1 1997/10/30 03:51:55 gdr Exp $
#

PROG	= help

.INCLUDE:	/src/gno/paths.mk
.INCLUDE:	/src/gno/binconst.mk

BINDIR	= /usr/bin
MANDIR	= /usr/man
RELBIN	= $(RELEASE_DIR)$(BINDIR)
RELMAN	= $(RELEASE_DIR)$(MANDIR)

build:	$(PROG)

$(PROG): help.gsh $(PROG).r
	$(INSTALL) -m755 help.gsh $@
	$(CATREZ) -d $@ $(PROG).r

.INCLUDE:	/src/gno/binrelease.mk
