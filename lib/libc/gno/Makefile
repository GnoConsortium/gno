#
# gno/libc/gno/Makefile
#
# Devin Reade, 1996
#
# $Id: Makefile,v 1.4 1997/10/03 04:44:22 gdr Exp $
#

.INCLUDE:	../../../paths.mk
.INCLUDE: 	../../const.mk

OBJ_ASM	= gnocmd.o parsearg.o stack.o
OBJ_C	= gnomisc.o gsstring.o map.o
OBJS	= $(OBJ_ASM) $(OBJ_C)

CFLAGS	+= -O78

MACROS_ORCA	= $(ORCA_DIST)/OrcaInclude
MACRO_FILES	= $(MACROS_ORCA)/m16.tools $(MACROS_ORCA)/m16.orca

build .PHONY:	../libc

obj:	$(OBJS)

stack.mac:
	macgen -p stack.asm $@ $(MACRO_FILES)

clean clobber:
	$(RM) -f $(OBJS) stack.mac

.INCLUDE: ../rules.mk

gnocmd.o::	gnocmd.mac
parsearg.o::	parsearg.mac
stack.o::	stack.mac
