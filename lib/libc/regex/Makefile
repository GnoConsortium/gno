#
# $Id: Makefile,v 1.1 1997/10/08 07:07:49 gdr Exp $
#
# Devin Reade, October 1997.
#

.INCLUDE:	../../../paths.mk
.INCLUDE: 	../../const.mk

OBJS	= regcomp.o regerror.o regexec.o regfree.o

# I've not bothered to determine what POSIX_MISTAKE means, but 4.4BSD
# uses it.
CFLAGS	+= -DPOSIX_MISTAKE

build .PHONY:	../libc
obj:	$(OBJS)

.INCLUDE:	../rules.mk

regcomp.o::	cclass.h cname.h regex2.h utils.h
regerror.o::	utils.h
regexec.o::	engine.c regex2.h utils.h
regfree.o::	regex2.h utils.h
