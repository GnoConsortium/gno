#
# This is the top-level makefile for libtermcap, GNO v2.0.6
#
# $Id: Makefile,v 1.2 1997/10/03 04:52:59 gdr Exp $
#

.INCLUDE: ../../paths.mk
.INCLUDE: ../const.mk                          

LIB	= libtermcap
OBJS	= termcap.o tgoto.o tputs.o tparm.o tospeed.o
LIBDIR	= /usr/lib
MANDIR	= /usr/man/man3

CFLAGS	+= -O78 -Slibtermcap -G25 -v -w
CFLAGS	+= -DCM_N -DCM_GT -DCM_B -DCM_D -I.

build:	$(LIB)

$(LIB): $(OBJS) $(LIB).r
	$(RM) -f $@
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $(OBJS)
	$(CATREZ) -d $@ $(LIB).r

install: beforeinstall $(LIB)
	$(INSTALL) -d $(LIBDIR) $(MANDIR)
	$(INSTALL) $(LIB) $(LIBDIR)
	$(INSTALL) termcap.3 $(MANDIR)

release: beforeinstall $(LIB)
	$(INSTALL) -d $(RELEASE_DIR)$(LIBDIR) $(RELEASE_DIR)$(MANDIR)
	$(INSTALL) $(LIB) $(RELEASE_DIR)$(LIBDIR)
	$(INSTALL) termcap.3 $(RELEASE_DIR)$(MANDIR)

beforeinstall:
	cmp ../../include/termcap.h termcap.h
