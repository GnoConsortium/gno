#
# This is the top-level makefile for libutil, GNO v2.0.6
#
# $Id: Makefile,v 1.3 1997/10/03 04:47:34 gdr Exp $
#

.INCLUDE: ../../paths.mk
.INCLUDE: ../const.mk                          

LIB	= libutil
OBJS	= login.o logintty.o logwtmp.o
LIBDIR	= /usr/lib

CFLAGS	+= -O78

build:	$(LIB)

$(LIB): $(OBJS) libutil.r
	$(RM) -f $@
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $(OBJS)
	$(CATREZ) -d $@ libutil.r

install: $(LIB)
	$(INSTALL) -d $(LIBDIR)
	$(INSTALL) $(LIB) $(LIBDIR)

release: $(LIB)
	$(INSTALL) -d $(RELEASE_DIR)$(LIBDIR)
	$(INSTALL) $(LIB) $(RELEASE_DIR)$(LIBDIR)
