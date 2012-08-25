#
# $Id: GNUmakefile,v 1.1 2012/08/25 07:19:00 gdr Exp $
#


SRCROOT 	= ../../..
WEB_HOME_BASE	= refs/gsh
SUBPROJECTS	= 

include $(SRCROOT)/doc/etc/constpriv.mk
include $(SRCROOT)/doc/etc/const.mk

WEB_HOME	= $(DOCROOT_INSECURE)/$(WEB_HOME_BASE)
HTML_DIR	= $(WEB_HOME)/html
TARGET_DIR	= $(GNO_PUBLIC_HTML)/$(WEB_HOME_BASE)
HEAD_PAGE	= 
TAIL_PAGE	= 
TARGETS		= buildDocbookHtml buildDocbookPdf
DOCBOOK_TOP	= gsh.docbook

default: build

clean::
	/bin/rm -rf $(HTML_DIR)

include $(SRCROOT)/doc/etc/rules.mk
include $(SRCROOT)/doc/etc/tailcat.mk

install:
	@/bin/rm -rf $(TARGET_DIR)
	install -d -m755 $(TARGET_DIR)
	@echo "copying files to $(TARGET_DIR)"; \
	cd $(WEB_HOME); tar -cf - . | \
	  (cd $(TARGET_DIR); tar -xpBf -);
	@echo "setting permissions on $(TARGET_DIR)"; \
	find $(TARGET_DIR) -type d \! -perm 0755 -exec chmod 0755 {} \; ; \
	find $(TARGET_DIR) -type f \! -perm 0644 -exec chmod 644 {} \;
