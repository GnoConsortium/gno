#
# This makefile is intended for use with dmake(1) on Apple IIGS
#
# $Id: Makefile,v 1.3 1999/01/11 01:25:43 gdr-ftp Exp $
#
# Devin Reade, September 1997
#

PROG	= whereis

BINDIR	= /usr/bin
ETCDIR	= /etc
OPTIMIZE= 78
STACK	= 1024
LDLIBS	= -l/usr/lib/libcontrib

.INCLUDE:	/src/gno/prog.mk

RELETC = $(RELEASE_DIR)$(ETCDIR)

release:: $(RELETC) whereis.conf
	$(INSTALL) whereis.conf $(RELETC)

$(RELETC):
	$(INSTALL) -d $(RELETC)

# Don't install whereis.conf into a running system; we don't want to
# overwrite the one that's been customized for this site.
