#
# $Id: Makefile,v 1.1 1997/10/30 02:33:48 gdr Exp $
#

PROG	= rmdir

# Add -D__STACK_CHECK__ to CFLAGS to get stack usage information
CFLAGS	+= -I/src/gno/lib/libcontrib
LDFLAGS	+= -l/src/gno/lib/libcontrib/libcontrib
STACK	= 768

.INCLUDE: /src/gno/prog.mk
