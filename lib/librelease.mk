#
# Default 'release' and 'install' rules.  These are intended for
# use when building libraries.
#
# $Id: librelease.mk,v 1.1 1998/02/08 03:47:18 gdr-ftp Exp $
#
# Devin Reade, February 1998.
#

# Select the appropriate destination directory.
.IF $(USE_SLASH_LIB) != $(NULL)
	TARGET_DIR	= $(LIBDIR)
	RELTARGET_DIR	= $(RELLIB)
.ELSE
	TARGET_DIR	= $(USRLIBDIR)
	RELTARGET_DIR	= $(RELUSRLIB)
.END

# Place files where they will subsequently be archived in a binary
# distribution.
release: $(LIBTARGET)
	$(INSTALL) -d $(RELTARGET_DIR)
	$(INSTALL) $(LIBTARGET) $(RELTARGET_DIR)

# Install files into a live system.
install: $(LIBTARGET)
	$(INSTALL) -d $(TARGET_DIR)
	$(INSTALL) $(LIBTARGET) $(TARGET_DIR)
