#
# This is the makefile for libcontrib, GNO v2.0.6
#
# $Id: Makefile,v 1.3 1998/02/08 03:47:29 gdr-ftp Exp $
#

LIB		= contrib
SRCS		= copyfile.c expandpath.c strarray.c xalloc.c
OPTIMIZE	= 78
CFLAGS		+= -I. -Slibcontrib

.INCLUDE:	/src/gno/lib/lib.mk

$(LIBTARGET):: lib$(LIB).r
	$(CATREZ) -d $@ lib$(LIB).r

$(OBJS):: contrib.h
