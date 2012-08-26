#
# $Id: GNUmakefile,v 1.1 2012/08/26 02:27:36 gdr Exp $
#

SRCROOT		= ../..
WEB_HOME_BASE	= refs.aug96
SUBPROJECTS	= 

include $(SRCROOT)/doc/etc/constpriv.mk
include $(SRCROOT)/doc/etc/const.mk

WEB_HOME	= $(DOCROOT_INSECURE)/$(WEB_HOME_BASE)
HEAD_PAGE	= 
TAIL_PAGE	= 
TARGETS		= literalCopy

#INSTALL_REAL	= $(GNO_PUBLIC_HTML)/refs.aug96
#INSTALL_FAKE	= $(INSTALL_REAL)-test

MAIN_PAGES	= gsh.12.html \
	          gsh.3.html \
	          gsh.4.html \
	          gsh.5.html \
		  install.html \
		  kern.html \
		  man1.html \
		  man2.html \
		  man3.html \
		  man4.html

default: build

literalCopy:
	@for f in $(MAIN_PAGES); do \
		echo install -m644 $$f $(WEB_HOME); \
		install -m644 $$f $(WEB_HOME); \
	done

include $(SRCROOT)/doc/etc/rules.mk
include $(SRCROOT)/doc/etc/tailcat.mk
