#
# Default 'release' and 'install' rules.  These are intended for
# typical user programs.  They are likely not suitable for system
# daemons, etc.
#
# $Id: binrelease.mk,v 1.1 1997/10/30 04:26:58 gdr Exp $
#

# Place files where they will subsequently be archived in a binary
# distribution.
release: $(PROG) $(PROG).1 $(DESC)
	$(INSTALL) -d $(RELBIN) $(RELMAN)/man1 $(DESC_DIR)
	$(INSTALL) $(PROG) $(RELBIN)
	$(INSTALL) $(PROG).1 $(RELMAN)/man1
	$(DESCU) -o $(DESC_SRC) $(DESC_SRC) $(DESC)

# Install files into a live system.  This doesn't update the describe
# database.
install: $(PROG) $(PROG).1 $(DESC)
	$(INSTALL) -d $(BINDIR) $(MANDIR)/man1 $(DESC_DIR)
	$(INSTALL) $(PROG) $(BINDIR)
	$(INSTALL) $(PROG).1 $(MANDIR)/man1
