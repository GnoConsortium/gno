#
# $Id: Makefile,v 1.3 1998/02/09 08:44:37 taubert Exp $
#

LIB=	c
LIBPFX=	$(OBJ_DIR)../
OBJS=	regcomp.o regerror.o regexec.o regfree.o

# I've not bothered to determine what POSIX_MISTAKE means, but 4.4BSD
# uses it.
CFLAGS	+= -v -r -DPOSIX_MISTAKE

.INCLUDE: /src/gno/lib/lib.mk

regcomp.o::	cclass.h cname.h regex2.h utils.h
regerror.o::	utils.h
regexec.o::	engine.c regex2.h utils.h
regfree.o::	regex2.h utils.h
