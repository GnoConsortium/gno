#
# $Id: Makefile,v 1.4 1998/02/19 01:07:27 gdr-ftp Exp $
#

LIB	= c
LIBPFX	= $(OBJ_DIR)../
NO_REZ	= true
SRCS	= regcomp.c regerror.c regexec.c regfree.c

# I've not bothered to determine what POSIX_MISTAKE means, but 4.4BSD
# uses it.
CFLAGS	+= -v -r -DPOSIX_MISTAKE

.INCLUDE: /src/gno/lib/lib.mk

regcomp.o:	cclass.h cname.h regex2.h utils.h
regerror.o:	utils.h
regexec.o:	engine.c regex2.h utils.h
regfree.o:	regex2.h utils.h
