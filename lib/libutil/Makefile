#
# This is the top-level makefile for libutil, GNO v2.0.6
#
# $Id: Makefile,v 1.2 1997/09/21 05:59:14 gdr Exp $
#

.INCLUDE: ../../paths.mk
.INCLUDE: ../const.mk                          

OBJS	= login.o logintty.o logwtmp.o

CFLAGS	+= -O78

libutil: $(OBJS)
	$(RM) -f $@
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $(OBJS)
