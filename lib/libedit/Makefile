#	@(#)Makefile	8.1 (Berkeley) 6/4/93

LIB=	libedit.a
AHDR	= vi.h emacs.h common.h
ASRC	= vi.c emacs.c common.c
GENSRCS	= $(AHDR) fcns.h fcns.c help.c help.h editline.c
SH	= /bin/sh

OSRCS=	chared.c common.c el.c emacs.c hist.c key.c map.c parse.c \
	prompt.c read.c refresh.c search.c sig.c term.c tty.c vi.c \
	help.c fcns.c help.h

SYS_ELL_H = sys.h el.h $(AHDR) fcns.h help.h

GENOBJS = help.o fcns.o
OBJS	= chared.o common.o el.o emacs.o hist.o key.o map.o parse.o \
	  prompt.o read.o refresh.o search.o sig.o term.o tty.o vi.o \
	  history.o tokenizer.o \
	  $(GENOBJS)

LDADD+= -ltermcap

# For speed and debugging
#SRCS=   ${OSRCS} tokenizer.c history.c
# For protection
SRCS=	editline.c tokenizer.c history.c

CLEANFILES+=common.h emacs.h fcns.h help.h vi.h help.c fcns.c editline.c
DEFINES	= -Dlint
#CFLAGS	+= $(DEFINES) -I../../include -I.
CFLAGS	+= $(DEFINES) -I.
CFLAGS+=#-DDEBUG_TTY -DDEBUG_KEY -DDEBUG_READ -DDEBUG -DDEBUG_REFRESH
CFLAGS+=#-DDEBUG_PASTE
CFLAGS	+= -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror \
	   -Wshadow -Wmissing-declarations -Wnested-externs # -W

build: $(LIB)

$(LIB): $(GENSRCS) $(OBJS)
	$(AR) -rcv $(LIB) $(OBJS)

clobber:
	$(RM) -f $(GENSRCS) $(OBJS) $(LIB)

vi.h: vi.c makelist
	$(SH) makelist -h vi.c > $@

emacs.h: emacs.c makelist
	$(SH) makelist -h emacs.c > $@

common.h: common.c makelist
	$(SH) makelist -h common.c > $@

fcns.h: ${AHDR} makelist
	$(SH) makelist -fh ${AHDR} > $@

fcns.c: ${AHDR} fcns.h makelist
	$(SH) makelist -fc ${AHDR} > $@

help.c: ${ASRC} makelist
	$(SH) makelist -bc ${ASRC} > $@

help.h: ${ASRC} makelist
	$(SH) makelist -bh ${ASRC} > $@

editline.c: ${OSRCS}
	$(SH) makelist -e ${OSRCS} > $@

chared.o::	$(SYS_ELL_H)
common.o::	$(SYS_ELL_H)
el.o::		$(SYS_ELL_H)
emacs.o::	$(SYS_ELL_H)
hist.o::	$(SYS_ELL_H)
key.o::		$(SYS_ELL_H)
map.o::		$(SYS_ELL_H)
parse.o::	$(SYS_ELL_H) tokenizer.h
prompt.o::	$(SYS_ELL_H)
read.o::	$(SYS_ELL_H)
refresh.o::	$(SYS_ELL_H)
search.o::	$(SYS_ELL_H)
sig.o::		$(SYS_ELL_H)
term.o::	$(SYS_ELL_H)
tty.o::		$(SYS_ELL_H) tty.h
vi.o::		$(SYS_ELL_H)

# .depend: vi.h emacs.h common.h fcns.h help.h help.c

testit:	TEST/test.o libedit.a ${DPADD} ${LIBTERMCAP}
	${CC} ${CFLAGS} TEST/test.o ${DPADD} -o $@ libedit.a ${LDADD} -ltermcap

# .include <bsd.lib.mk>
