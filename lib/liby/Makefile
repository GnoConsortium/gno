#
# This is the top-level makefile for libutil, GNO v2.0.6
#
# $Id: Makefile,v 1.2 1997/10/03 04:46:02 gdr Exp $
#

.INCLUDE: ../../paths.mk
.INCLUDE: ../const.mk                          

LIB	= liby
OBJS	= main.o yyerror.o
LIBDIR	= /usr/lib

CFLAGS	+= -O78

build:	$(LIB)

$(LIB): $(OBJS) liby.r
	$(RM) -f $@
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $(OBJS)
	$(CATREZ) -d $@ liby.r

install: $(LIB)
	$(INSTALL) -d $(LIBDIR)
	$(INSTALL) $(LIB) $(LIBDIR)

release: $(LIB)
	$(INSTALL) -d $(RELEASE_DIR)$(LIBDIR)
	$(INSTALL) $(LIB) $(RELEASE_DIR)$(LIBDIR)
