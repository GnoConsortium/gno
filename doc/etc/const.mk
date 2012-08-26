#
# $Id: const.mk,v 1.1 2012/08/26 02:27:35 gdr Exp $
#
# Before this file is included, it is assumed that you have included
# the constpriv.mk file, which needs to define these variables:
#
#   GNO_PUBLIC_HTML
#      The directory where the GNO web pages are anchored.  This
#      top level directory is not managed by these files, but
#      the files there need to reference these files.  If you are
#      not building this for the official GNO web site, you can
#      just point this at a scratch directory somewhere.
#   

# This is the name and address that will be given as contact info
# at the bottom of each of the HTML pages.  Do NOT use '<' or '>' in the
# address.
NAME	= Devin Reade
ADDRESS	= gdr@gno.org

OUTPUT_DIR		= $(SRCROOT)/doc/gen
DOCROOT			= $(OUTPUT_DIR)
DOCROOT_INSECURE	= $(OUTPUT_DIR)/insecure/gno
DOCROOT_SECURE		= $(OUTPUT_DIR)/secure/gno

HTTP_SERVER		= www.gno.org
HTTP_PORT		= # :81
HTTPS_PORT		= # :8443
FTP_SERVER		= ftp.gno.org

DATE			= $(SRCROOT)/etc/getdate -date

BUILD_FILES		= GNUmakefile $(HEAD_PAGE) $(TAIL_PAGE) \
			  $(SRCROOT)/doc/etc/const.mk \
			  $(SRCROOT)/doc/etc/rules.mk \
			  $(SRCROOT)/doc/etc/tailcat.mk

# This is where the files will eventually wind up.
TARGET_DIR		= $(GNO_PUBLIC_HTML)/$(WEB_HOME_BASE)
