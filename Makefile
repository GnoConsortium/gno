#
# This file is intended for use with dmake.  There are constructs in 
# here that (as yet) make it unsuitable to be used on the GS.
#
# $Id: Makefile,v 1.3 1997/10/30 04:23:37 gdr Exp $
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

ROOTFILES = Makefile binconst.mk binrules.mk paths.mk prog.mk

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
	cp -r -p $< $(TMP); \
	$(RM) -rf `find $(TMP) -name CVS -print `; \
	$(UDL) -gR $(TMP); \
	(cd $(TMP); $(NULIB) -cf $$cwd/$@ $< ); \
	$(RM) -rf $(TMP)

# make sure the filename fits
$(XFER)/usr.orcabin.shk: $(XFER)/usr.orca.bin.shk
	cp $< $@

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
