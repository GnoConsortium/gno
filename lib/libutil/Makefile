#
# This is the top-level makefile for libutil, GNO v2.0.6
#
# $Id: Makefile,v 1.1 1997/02/28 05:03:52 gdr Exp $
#

.INCLUDE: ../const.mk

OBJS	= login.o logintty.o

libutil: $(OBJS)
	$(RM) -f $@
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $(OBJS)
