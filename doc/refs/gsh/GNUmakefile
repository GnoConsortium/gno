#
# $Id: GNUmakefile,v 1.2 2012/08/26 02:27:36 gdr Exp $
#


SRCROOT 	= ../../..
WEB_HOME_BASE	= refs/gsh
SUBPROJECTS	= 

include $(SRCROOT)/doc/etc/constpriv.mk
include $(SRCROOT)/doc/etc/const.mk

WEB_HOME	= $(DOCROOT_INSECURE)/$(WEB_HOME_BASE)
HEAD_PAGE	= 
TAIL_PAGE	= 
TARGETS		= buildDocbookHtml buildDocbookPdf
DOCBOOK_TOP	= gsh.docbook

default: build

clean::
	/bin/rm -rf $(WEB_HOME)/html

include $(SRCROOT)/doc/etc/rules.mk
include $(SRCROOT)/doc/etc/tailcat.mk

