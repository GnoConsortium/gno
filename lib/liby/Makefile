#
# This is the makefile for liby, GNO v2.0.6
#
# $Id: Makefile,v 1.3 1998/02/08 03:47:40 gdr-ftp Exp $
#

LIB		= y
SRCS		= main.c yyerror.c
OPTIMIZE	= 78

.INCLUDE:	/src/gno/lib/lib.mk

$(LIBTARGET):: lib$(LIB).r
	$(CATREZ) -d $@ lib$(LIB).r

