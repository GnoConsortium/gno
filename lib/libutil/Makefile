#
# This is the makefile for libutil, GNO v2.0.6
#
# $Id: Makefile,v 1.4 1998/02/08 03:47:34 gdr-ftp Exp $
#

LIB		= util
SRCS		= login.o logintty.o logwtmp.o
OPTIMIZE	= 78

.INCLUDE: 	/src/gno/lib/lib.mk

$(LIBTARGET):: lib$(LIB).r
	$(CATREZ) -d $@ lib$(LIB).r
