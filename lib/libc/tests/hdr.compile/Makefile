#
# Makefile for libc/tests/hdr.compile
#
# $Id: Makefile,v 1.1 1997/02/28 05:12:55 gdr Exp $
#

.INCLUDE:	../../const.mk

#
# These should be tested with and without the following macros #defined:
#	_ANSI_SOURCE
#	_POSIX_SOURCE
#	TEST_SGTTY
#	KERNEL
# Currently, defining KERNEL makes no difference for the arpa, protocols,
# or rpc tests.
#
# We don't currently test:
#	net.c		-- non-BSD and unprototyped; insufficient info
#	protocols.c	-- unchanged from Derek's original, which
#			   matches the BSD version.
#

CFLAGS	+= -DKERNEL

OBJS	= netinet.o machine.o rpc.o arpa.o gno.o sys.o base.o

default:	$(OBJS)

clobber:
	$(RM) -f $(OBJS)
