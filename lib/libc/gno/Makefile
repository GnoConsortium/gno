#
# gno/libc/gno/Makefile
#
# $Id: Makefile,v 1.7 1998/02/19 01:05:16 gdr-ftp Exp $
#

LIB	= c
LIBPFX	= $(OBJ_DIR)../
NO_REZ	= true
SRC_ASM	= gnocmd.asm parsearg.asm stack.asm
SRC_C	= gnomisc.c gsstring.c map.c stack2.c
SRCS	= $(SRC_ASM) $(SRC_C)

.INCLUDE: /src/gno/lib/lib.mk

MACROS_ORCA	= $(ORCA_DIST)/libraries/ORCAInclude
MACRO_FILES	= $(MACROS_ORCA)/m16.tools $(MACROS_ORCA)/m16.orca
MAC		= $(OBJ_DIR)stack.mac

$(MAC):
	macgen -p stack.asm $@ $(MACRO_FILES)

gnocmd.o:	gnocmd.mac
parsearg.o:	parsearg.mac
stack.o:	$(MAC)
