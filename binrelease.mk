#
# Default 'release' and 'install' rules.  These are intended for
# typical user programs.  They are likely not suitable for system
# daemons, etc.
#
# $Id: binrelease.mk,v 1.2 1998/02/15 19:43:58 gdr-ftp Exp $
#

# Place files where they will subsequently be archived in a binary
# distribution.
release: $(OBJ_DIR)$(PROG) $(PROG).$(MAN1SFX) $(DESC)
	$(INSTALL) -d $(RELBIN) $(RELMAN)/man1 $(DESC_DIR)
	$(INSTALL) $(OBJ_DIR)$(PROG) $(RELBIN)
	$(INSTALL) $(PROG).$(MAN1SFX) $(RELMAN)/man1/$(PROG).1
	$(DESCU) -o $(DESC_SRC) $(DESC_SRC) $(DESC)

# Install files into a live system.  This doesn't update the describe
# database.
install: $(OBJ_DIR)$(PROG) $(PROG).$(MAN1SFX)
	$(INSTALL) -d $(BINDIR) $(MANDIR)/man1
	$(INSTALL) $(OBJ_DIR)$(PROG) $(BINDIR)
	$(INSTALL) $(PROG).$(MAN1SFX) $(MANDIR)/man1/$(PROG).1
