#
# $Id: Makefile,v 1.2 1997/12/21 22:11:07 gdr Exp $
#

PROG	= cpp
SRCS	= cpp.c eval.c getopt.c hideset.c include.c lex.c macro.c nlist.c \
	  tokens.c unix.c 
                   
STACK		= 8192
OPTIMIZE	= 0
DEBUG		= 25
CFLAGS		+= -DDO_SEGMENTS -D__STACK_CHECK__
LDLIBS		+= /src/gno/lib/libc/gno/stack.o /src/gno/lib/libc/gno/stack2.o

# Delivery directory
BINDIR	= /usr/bin

.INCLUDE : /src/gno/prog.mk
