#
# This is the makefile for libcontrib, GNO v2.0.6
#
# $Id: Makefile,v 1.4 1998/03/08 18:11:49 gdr-ftp Exp $
#

LIB		= contrib
SRCS		= copyfile.c expandpath.c strarray.c xalloc.c
OPTIMIZE	= 78
CFLAGS		+= -I. -Slibcontrib

.INCLUDE:	/src/gno/lib/lib.mk

HDIR		= $(RELEASE_DIR)/usr/include/gno

release::
	$(INSTALL) -d $(HDIR)
	$(INSTALL) contrib.h $(HDIR)

$(OBJS):: contrib.h
