#
# $Id: Makefile,v 1.2 1998/03/08 17:40:53 gdr-ftp Exp $
#

PROG	= help

.INCLUDE:	/src/gno/paths.mk
.INCLUDE:	/src/gno/binconst.mk

BINDIR	= /usr/bin
MANDIR	= /usr/man
RELBIN	= $(RELEASE_DIR)$(BINDIR)
RELMAN	= $(RELEASE_DIR)$(MANDIR)

build:	$(OBJ_DIR) $(OBJ_DIR)$(PROG)

$(OBJ_DIR)$(PROG): help.gsh $(PROG).r
	$(INSTALL) -m755 help.gsh $@
	$(CATREZ) -d $@ $(OBJ_DIR)$(PROG).r

$(OBJ_DIR):;	$(INSTALL) -d $(OBJ_DIR)

.INCLUDE:	/src/gno/binrelease.mk
