#
# This file is intended for use with dmake.  There are constructs in 
# here that (as yet) make it unsuitable to be used on the GS.
#
# $Id: Makefile,v 1.6 2012/08/26 02:54:57 gdr Exp $
#

.INCLUDE:	/src/gno/paths.mk

INSTALL	*=	/usr/bin/install
CHTYP	*=	/bin/chtyp

headerInstall .USESHELL:
	$(CHTYP) -lasm $(SRC_DIR)/gno/ainclude/m*
	$(CHTYP) -lrez $(SRC_DIR)/gno/rinclude/*.rez
	$(CHTYP) -lcc $(SRC_DIR)/gno/include/*.h
	$(CHTYP) -lcc $(SRC_DIR)/gno/include/arpa/*.h
	$(CHTYP) -lcc $(SRC_DIR)/gno/include/gno/*.h
	$(CHTYP) -lcc $(SRC_DIR)/gno/include/machine/*.h
	$(CHTYP) -lcc $(SRC_DIR)/gno/include/net/*.h
	$(CHTYP) -lcc $(SRC_DIR)/gno/include/netinet/*.h
	$(CHTYP) -lcc $(SRC_DIR)/gno/include/protocols/*.h
	$(CHTYP) -lcc $(SRC_DIR)/gno/include/rpc/*.h
	$(CHTYP) -lcc $(SRC_DIR)/gno/include/sys/*.h
	$(CHTYP) -lcc $(SRC_DIR)/gno/orcacdefs/*.h
	$(INSTALL) $(SRC_DIR)/gno/ainclude/m* /usr/ainclude
	$(INSTALL) $(SRC_DIR)/gno/rinclude/*.rez /usr/rinclude
	$(INSTALL) $(SRC_DIR)/gno/include/*.h /usr/include
	$(INSTALL) $(SRC_DIR)/gno/include/arpa/*.h /usr/include/arpa
	$(INSTALL) $(SRC_DIR)/gno/include/gno/*.h /usr/include/gno
	$(INSTALL) $(SRC_DIR)/gno/include/machine/*.h /usr/include/machine
	$(INSTALL) $(SRC_DIR)/gno/include/net/*.h /usr/include/net
	$(INSTALL) $(SRC_DIR)/gno/include/netinet/*.h /usr/include/netinet
	$(INSTALL) $(SRC_DIR)/gno/include/protocols/*.h /usr/include/protocols
	$(INSTALL) $(SRC_DIR)/gno/include/rpc/*.h /usr/include/rpc
	$(INSTALL) $(SRC_DIR)/gno/include/sys/*.h /usr/include/sys
	$(INSTALL) $(SRC_DIR)/gno/orcacdefs/*.h /lib/orcacdefs

#
# The remaining macros, targets, and recipies were used before trenco
# was fully serving the GNO stuff.
#
XFER	= xfer
TMP	= /tmp/gnobuild
UDL	= /usr/local/bin/udl
NULIB	= /usr/local/bin/nulib

XFER_LIST= \
	$(XFER)/HFSinclude.shk \
	$(XFER)/NOTES.shk \
	$(XFER)/bin.shk \
	$(XFER)/build.tools.shk \
	$(XFER)/include.shk \
	$(XFER)/lib.shk \
	$(XFER)/orcacdefs.shk \
	$(XFER)/root.shk \
	$(XFER)/sbin.shk \
	$(XFER)/usr.bin.shk \
	$(XFER)/usr.orcabin.shk \
	$(XFER)/usr.man.shk \
	$(XFER)/usr.sbin.shk \
	$(XFER)/verbatim.shk

NOT_YET= \
	$(XFER)/libexec.shk \
	$(XFER)/sys.shk \
	$(XFER)/usr.sbin.shk

srcxfer: $(XFER_LIST)

ROOTFILES = Makefile binconst.mk binrelease.mk binrules.mk paths.mk prog.mk

$(XFER)/root.shk: $(ROOTFILES)
	@echo "making $@"; \
	$(RM) -f $@; \
	[ -d $(XFER) ] || mkdir -p $(XFER); \
	if [ -d $(TMP) ]; then \
		echo "$(TMP) exists.  Aborted"; \
		exit 1; \
	fi; \
	cwd=`pwd`; \
	mkdir -p $(TMP); \
	cp -r -p $(ROOTFILES) $(TMP); \
	$(RM) -rf `find $(TMP) -name CVS -print `; \
	$(UDL) -gR $(TMP); \
	(cd $(TMP); $(NULIB) -cf $$cwd/$@ $(ROOTFILES) ); \
	$(RM) -rf $(TMP)

# make sure the filename fits
$(XFER)/usr.orcabin.shk: $(XFER)/usr.orca.bin.shk
	mv $< $@

# .PHONY: $(XFER_LIST)

$(XFER)/%.shk:
	@echo "making $@"; \
	$(RM) -f $@; \
	[ -d $(XFER) ] || mkdir -p $(XFER); \
	if [ -d $(TMP) ]; then \
		echo "$(TMP) exists.  Aborted"; \
		exit 1; \
	fi; \
	mkdir -p $(TMP); \
	cp -r -p $* $(TMP); \
	$(RM) -rf `find $(TMP) -name CVS -print `; \
	$(UDL) -gR $(TMP); \
	(cd $(TMP); $(NULIB) -cf $*.shk $*); \
	mv $(TMP)/$*.shk $(XFER); \
	$(RM) -rf $(TMP)


#	/bin/true $*; 
