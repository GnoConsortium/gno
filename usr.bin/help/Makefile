#
# $Id: Makefile,v 1.3 1998/12/31 21:34:56 gdr-ftp Exp $
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

# Implicit rule to handle Rez source on case sensitive Appleshare servers
.IF $(APPLESHARE_CASE_SENSITIVE) != $(NULL)
%.r : %.rez
	$(INSTALL) $< $(TMPDIR)/$<
	$(REZ) -o $@ -c $(REZFLAGS) $(TMPDIR)/$<
	$(RM) -f $(TMPDIR)/$<
.END

.INCLUDE:	/src/gno/binrelease.mk
