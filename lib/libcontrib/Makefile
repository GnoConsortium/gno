#
# This is the top-level makefile for libcontrib, GNO v2.0.6
#
# $Id: Makefile,v 1.2 1997/10/30 04:57:24 gdr Exp $
#

.INCLUDE: ../../paths.mk
.INCLUDE: ../const.mk                          

LIB	= libcontrib
OBJS	= copyfile.o expandpath.o strarray.o xalloc.o
HEADER	= contrib.h
LIBDIR	= $(RELEASE_DIR)/usr/lib
INCDIR	= $(RELEASE_DIR)/usr/include/gno

CFLAGS	+= -I. -O78 -Slibcontrib

$(LIB): $(OBJS) $(LIB).r
	$(RM) -f $@
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $(OBJS)
	$(CATREZ) -d $(LIB) $(LIB).r

release: $(LIB) $(HEADER)
	$(INSTALL) -d $(LIBDIR) $(INCDIR)
	$(INSTALL) $(LIB) $(LIBDIR)
	$(INSTALL) $(HEADER) $(INCDIR)

$(OBJS):: $(HEADER)
