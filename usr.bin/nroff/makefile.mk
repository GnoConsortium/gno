#
# Makefile for nroff(1).
#
# $Id: makefile.mk,v 1.2 1997/03/20 06:40:50 gdr Exp $
#

IIGS		= TRUE	# TRUE or FALSE
USE_INSIGHT	= FALSE # TRUE or FALSE

BINDIR		= /usr/bin
TMACDIR		= /usr/lib/tmac
MANDIR		= /usr/man
INSTALL		= /usr/bin/install

#
###### end of configuration
#

PROFILE		= # -pg

.IF $(IIGS) == TRUE
DEFINES		=
OPTIMIZE	= -v -w -G1
LDFLAGS		+= -v
LDLIBS		= -l/lib/ltermcap -l/trenco4/gno.src/lib/libc/libc.v211b2
.ELIF $(USE_INSIGHT) == TRUE     
CC		= insight 
DEFINES		=
OPTIMIZE	= -g
LDLIBS		= unix/int.tqs -ltermcap
.ELSE                    
CC		= gcc
DEFINES		= # -DDEBUG
OPTIMIZE	= $(PROFILE) -g
LDLIBS		= $(PROFILE) -ltermcap
.END             

.IF $(IIGS) == TRUE
UX_SRC		=
UX_OBJ		=
REZ_OBJ		= nroff.r
.ELSE
UX_SRC		= unix/err.c
UX_OBJ		= unix/err.o
REZ_OBJ		=
.END

GCC_PARANOIA	= \
	-Wstrict-prototypes \
	-Wmissing-prototypes \
	-Waggregate-return \
	-Wnested-externs \
	-fno-asm -fno-builtin -fno-inline

.IF $(CC) == gcc
GCC_FLAGS	= -Wall -funsigned-char $(GCC_PARANOIA)
.ELSE
GCC_FLAGS	=
.END


CFLAGS	+= $(OPTIMIZE) $(GCC_FLAGS) $(DEFINES)

SUNOS_H	= unix/sunos.h
OBJS	= main.o command.o escape.o io.o low.o macros.o strings.o \
	  text.o $(UX_OBJ)
SRCS	= main.c command.c escape.c io.c low.c macros.c strings.c \
	  text.c $(UX_SRC)

default: nroff

nroff: $(OBJS) $(REZ_OBJ)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LDLIBS)
	copyfork $(REZ_OBJ) $@ -r

clobber:
	$(RM) -f nroff $(OBJS) 
#	$(RM) -f nroff $(OBJS) *~ core tca.* *.tqs insight.log

dep:
	gcc -MM $(CFLAGS) *.c > depend

callchart:
	cflow $(SRCS) > $@

install:
	$(RM) -f /bin/nroff
	$(INSTALL) -d $(BINDIR) $(TMACDIR) $(MANDIR)/man1 $(MANDIR)/man7
	$(INSTALL) -m755 nroff $(BINDIR)
	$(INSTALL) -m644 tmac.an $(TMACDIR)
	$(INSTALL) -m644 tmac.s $(TMACDIR)
	$(INSTALL) -m644 nroff.1 $(MANDIR)/man1
	$(INSTALL) -m644 man.7 $(MANDIR)/man7
	$(INSTALL) -m644 ms.7 $(MANDIR)/man7

#
# additional dependancies
#
command.o::	$(SUNOS_H) nroff.h config.h
escape.o::	$(SUNOS_H) nroff.h config.h
io.o::	 	$(SUNOS_H) nroff.h config.h macros.h io.h
low.o::	 	$(SUNOS_H) nroff.h config.h
macros.o::	$(SUNOS_H) nroff.h config.h macros.h
main.o::	$(SUNOS_H) nroff.h config.h macros.h
strings.o::	$(SUNOS_H) nroff.h config.h
text.o::	$(SUNOS_H) nroff.h config.h io.h
