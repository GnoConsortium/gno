#
# gno/libc/gno/Makefile
#
# $Id: Makefile,v 1.6 1998/02/09 08:44:20 taubert Exp $
#

LIB=	c
LIBPFX=	$(OBJ_DIR)../
OBJ_ASM=gnocmd.o parsearg.o stack.o
OBJ_C=	gnomisc.o gsstring.o map.o stack2.o
OBJS=	$(OBJ_ASM) $(OBJ_C)

MACROS_ORCA	= $(ORCA_DIST)/OrcaInclude
MACRO_FILES	= $(MACROS_ORCA)/m16.tools $(MACROS_ORCA)/m16.orca

.INCLUDE: /src/gno/lib/lib.mk

stack.mac:
	macgen -p stack.asm $@ $(MACRO_FILES)

clean clobber:
	$(RM) -f $(OBJS) stack.mac

gnocmd.o::	gnocmd.mac
parsearg.o::	parsearg.mac
stack.o::	stack.mac
