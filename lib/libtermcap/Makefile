#
# This is the makefile for libtermcap, GNO v2.0.6
#
# $Id: Makefile,v 1.3 1998/03/08 18:14:53 gdr-ftp Exp $
#

LIB	= termcap
SRCS	= termcap.c tgoto.c tputs.c tparm.c tospeed.c
OPTIMIZE= 78
CFLAGS	+= -I. -Slibtermcap
CFLAGS	+= -DCM_N -DCM_GT -DCM_B -DCM_D -I.

LOCAL_SETUP	= local_setup
HDIR		= $(RELEASE_DIR)/usr/include

.INCLUDE: /src/gno/lib/lib.mk

# If this target fails, do a manual update.
local_setup:
	cmp /usr/include/termcap.h termcap.h

release::
	$(INSTALL) -d $(HDIR)
	$(INSTALL) termcap.h $(HDIR)

# LIBDIR	= /usr/lib
# MANDIR	= /usr/man/man3

# build:	$(LIB)

#$(LIB): $(OBJS) $(LIB).r
#	$(RM) -f $@
#	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $(OBJS)
#	$(CATREZ) -d $@ $(LIB).r

#install: beforeinstall $(LIB)
#	$(INSTALL) -d $(LIBDIR) $(MANDIR)
#	$(INSTALL) $(LIB) $(LIBDIR)
#	$(INSTALL) termcap.3 $(MANDIR)

#release: beforeinstall $(LIB)
#	$(INSTALL) -d $(RELEASE_DIR)$(LIBDIR) $(RELEASE_DIR)$(MANDIR)
#	$(INSTALL) $(LIB) $(RELEASE_DIR)$(LIBDIR)
#	$(INSTALL) termcap.3 $(RELEASE_DIR)$(MANDIR)

