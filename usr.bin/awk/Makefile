#
# This makefile is intended for use with dmake(1) on Apple IIGS
# $Id: Makefile,v 1.2 1999/01/09 18:52:56 gdr-ftp Exp $
#
#	Created by Dave Tribby, January 1998

# --------------------------------------------------------------------
# NOTE: the original Bell Labs package uses yacc (or bison) to compile
#       awkgram.y into ytab.c and ytab.h.  It compiles maketab.c into
#       the program maketab, and produces proctab.c using maketab and
#       ytab.h.  I chose to create ytab.c, ytab.h, and proctab.c on a
#       Unix system and bring them to the Apple IIGS, since I don't
#       use yacc or bison on my GS. This makefile could be modified
#       to include these additional steps. If necessary, remove the
#       "#" from the beginning of the following lines and move them
#       to the end of this Makefile.
#OBJS += maketab maketab.o
#YFLAGS += -d
#ytab.h:	awk.h proto.h awkgram.y
#	$(YACC) $(YFLAGS) awkgram.y
#	mv y.tab.c ytab.c
#	mv y.tab.h ytab.h
#
#proctab.c:	maketab
#	$(OBJ_DIR)maketab >proctab.c
#
#maketab.o:	ytab.h maketab.c
#	$(CC) -o $@ $(CFLAGS:s/ -r / /) -a0 -c maketab.c
#
#maketab: maketab.o
#	$(CC) -o $(OBJ_DIR)$@ $(LDFLAGS) $< $(LDLIBS)
# --------------------------------------------------------------------

# Default stack size (can be overridden by cmd line)
#   This value have been tested and certified as working, although
#   even more may be required for deeply recursive awk programs
STACK	*= 4096

# Program name
PROG=	awk
MAIN=	main

# Source files
#   Compile main program first, then in size order since ORCA/C
#   can run out of memory if run.c is compiled last
SRCS=	main.c run.c ytab.c b.c lib.c lex.c tran.c parse.c proctab.c

# Deliver to /usr/bin
BINDIR	= /usr/bin

.INCLUDE : /src/gno/prog.mk

# This line is here because if we don't have it, then PR#106 is observed.
# Almost certainly, this just moves the problem; it doesn't fix it.
NOTUSED = $(PWD)

#
# Additional dependancies
#
main.o:	awk.h proto.h ytab.h
b.o:		awk.h proto.h ytab.h
parse.o:	awk.h proto.h ytab.h
lib.o:	awk.h proto.h ytab.h
run.o:	awk.h proto.h ytab.h
tran.o:	awk.h proto.h ytab.h
ytab.o:	awk.h proto.h
lex.o:	awk.h proto.h
proctab.o:	awk.h proto.h
