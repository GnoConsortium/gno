#
# Default 'release' and 'install' rules.  These are intended for
# typical user programs.  They are likely not suitable for system
# daemons, etc.
#
# $Id: binrelease.mk,v 1.3 1998/04/22 05:07:20 gdr-ftp Exp $
#

.IF $(CHAPTER) == $(NULL)
OLDSFX	= $(MAN1SFX)
NEWSFX	= 1
.ELSE
OLDSFX	= $(MAN8SFX)
NEWSFX	= 8
.END

# Place files where they will subsequently be archived in a binary
# distribution.
release: $(OBJ_DIR)$(PROG) $(PROG).$(OLDSFX) $(DESC)
	$(INSTALL) -d $(RELBIN) $(RELMAN)/man$(NEWSFX) $(DESC_DIR)
	$(INSTALL) $(OBJ_DIR)$(PROG) $(RELBIN)
	$(INSTALL) $(PROG).$(OLDSFX) $(RELMAN)/man$(NEWSFX)/$(PROG).$(NEWSFX)
	$(DESCU) -o $(DESC_SRC) $(DESC_SRC) $(DESC)

# Install files into a live system.  This doesn't update the describe
# database.
install: $(OBJ_DIR)$(PROG) $(PROG).$(OLDSFX)
	$(INSTALL) -d $(BINDIR) $(MANDIR)/man$(NEWSFX)
	$(INSTALL) $(OBJ_DIR)$(PROG) $(BINDIR)
	$(INSTALL) $(PROG).$(OLDSFX) $(MANDIR)/man$(NEWSFX)/$(PROG).$(NEWSFX)

.IF $(HAS_MKLINK_DATA) != $(NULL)
release::
	$(INSTALL) -d $(MKSO_DIR)
	$(INSTALL) mklink.data $(MKSO_DIR)/mklink.$(PROG)
.END

.IF $(HAS_MKSO_DATA) != $(NULL)
release::
	$(INSTALL) -d $(MKSO_DIR)
	$(INSTALL) mkso.data $(MKSO_DIR)/mkso.$(PROG)
.END
