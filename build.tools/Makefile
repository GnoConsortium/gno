#
# $Id: Makefile,v 1.2 1999/03/19 05:53:32 gdr-ftp Exp $
#

.INCLUDE:	/src/gno/paths.mk

#
# In this directory, we're currently only copying some files verbatim,
# which makes our targets very simple.
#

build:
	@/bin/true

install:
	$(INSTALL) -d -m755 /usr/local/lib /usr/lib
	$(INSTALL) -d -m755 13/RInclude
	$(INSTALL) -m644 dmake.startup /usr/local/lib/startup.mk
	$(INSTALL) -m644 dmake.startup /usr/lib/startup.mk
	$(INSTALL) -m644 builddate.rez 13/RInclude

release:
	$(INSTALL) -d -m755 $(RELEASE_DIR)/usr/local/lib $(RELEASE_DIR)/usr/lib
	$(INSTALL) -d -m755 $(RELEASE_DIR)/lib/RInclude
	$(INSTALL) -m644 dmake.startup $(RELEASE_DIR)/usr/local/lib/startup.mk
	$(INSTALL) -m644 dmake.startup $(RELEASE_DIR)/usr/lib/startup.mk
	$(INSTALL) -m644 builddate.rez $(RELEASE_DIR)/lib/RInclude

clean clobber:
	@/bin/true
