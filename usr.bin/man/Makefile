#
# Devin Reade, 1998
#
# $Id: Makefile,v 1.4 1999/01/11 06:05:34 gdr-ftp Exp $
#

CUSTOM_RULES	= true		# provide our own build rules
CUSTOM_RELEASE	= true		# provide our own release and install targets

#
# Define:	-DDEBUG to produce more debugging info and checks
#
#		-DSTACK_CHECK to show stack usage.  If you use this
#		one, ensure you add -l/usr/lib/stack to your LDLIBS.
#
DEFINES		+= 
STACK		= 1280
LDADD		= -lcontrib
BINDIR		= /usr/bin
SBINDIR		= /usr/sbin
DESC		= manpack.desc

.INCLUDE:	/src/gno/prog.mk

RELSBIN	= $(RELEASE_DIR)$(SBINDIR)

PROGS1		= $(OBJ_DIR)apropos $(OBJ_DIR)man $(OBJ_DIR)whatis
PROGS8		= $(OBJ_DIR)catman $(OBJ_DIR)makewhatis
PROGS		= $(PROGS1) $(PROGS8)

APROPOS_OBJ	= apropos.o apropos2.o util.o globals.o
CATMAN_OBJ	= catman.o util.o globals.o common.o
MAKEWHATIS_OBJ	= makewhatis.o fillbuffer.o process.o
MAN_OBJ		= man.o man2.o apropos2.o util.o globals.o common.o
WHATIS_OBJ	= whatis.o apropos2.o util.o globals.o

MAINCFLAGS	= $(CFLAGS:s/ -r / /g)

build: $(OBJ_DIR) $(LOCAL_SETUP) $(PROGS)

$(OBJ_DIR)apropos: $(APROPOS_OBJ) apropos.r
	$(CC) $(LDFLAGS) $(OBJ_DIR){$(APROPOS_OBJ)} $(LDLIBS) -o $@
	$(CATREZ) -d $@ $*.r
                
$(OBJ_DIR)catman: $(CATMAN_OBJ) catman.r
	$(CC) $(LDFLAGS) $(OBJ_DIR){$(CATMAN_OBJ)} $(LDLIBS) -o $@
	$(CATREZ) -d $@ $*.r
                          
$(OBJ_DIR)makewhatis: $(MAKEWHATIS_OBJ) makewhatis.r
	$(CC) $(LDFLAGS) $(OBJ_DIR){$(MAKEWHATIS_OBJ)} $(LDLIBS) -o $@
	$(CATREZ) -d $@ $*.r
                          
$(OBJ_DIR)man: $(MAN_OBJ) man.r
	$(CC) $(LDFLAGS) $(OBJ_DIR){$(MAN_OBJ)} $(LDLIBS) -o $@
	$(CATREZ) -d $@ $*.r
                          
$(OBJ_DIR)whatis: $(WHATIS_OBJ) whatis.r
	$(CC) $(LDFLAGS) $(OBJ_DIR){$(WHATIS_OBJ)} $(LDLIBS) -o $@
	$(CATREZ) -d $@ $*.r

apropos.o:	apropos.c;	$(CC) -o $@ $(MAINCFLAGS) -a0 -c apropos.c
catman.o:	catman.c;	$(CC) -o $@ $(MAINCFLAGS) -a0 -c catman.c
makewhatis.o:	makewhatis.c;	$(CC) -o $@ $(MAINCFLAGS) -a0 -c makewhatis.c
man.o:		man.c;		$(CC) -o $@ $(MAINCFLAGS) -a0 -c man.c
whatis.o:	whatis.c;	$(CC) -o $@ $(MAINCFLAGS) -a0 -c whatis.c

$(OBJ_DIR):;	$(INSTALL) -d $(OBJ_DIR)

M1	= man.1 whatis.1
M8	= catman.8 makewhatis.8
DIRS	= $(SBINDIR) $(BINDIR) $(MANDIR)/man1 $(MANDIR)/man8
RELDIRS	= $(RELSBIN) $(BINDIR) $(MANDIR)/man1 $(MANDIR)/man8 $(DESC_DIR)

release: $(PROGS) $(M1) $(M8) $(DESC)
	$(INSTALL) -d $(RELSBIN) $(RELBIN) $(RELMAN)/man1 $(RELMAN)/man8
	$(INSTALL) -d $(DESC_DIR) $(MKSO_DIR)
	$(INSTALL) $(PROGS1) $(RELBIN)
	$(INSTALL) $(PROGS8) $(RELSBIN)
	$(INSTALL) $(M1) $(RELMAN)/man1
	$(INSTALL) $(M8) $(RELMAN)/man8
	$(INSTALL) mkso.data $(MKSO_DIR)/mkso.man
	$(DESCU) -o $(DESC_SRC) $(DESC_SRC) $(DESC)
                                  
install: $(PROGS) $(M1) $(M8)
	$(INSTALL) -d $(SBINDIR) $(BINDIR) $(MANDIR)/man1 $(MANDIR)/man8
	$(INSTALL) $(PROGS1) $(BINDIR)
	$(INSTALL) $(PROGS8) $(SBINDIR)
	$(INSTALL) $(M1) $(MANDIR)/man1
	$(INSTALL) $(M8) $(MANDIR)/man8

.IF $(APPLESHARE_CASE_SENSITIVE) != $(NULL)
%.r : %.rez
	$(INSTALL) $< common.rez $(TMPDIR)
	cd $(TMPDIR); $(REZ) -o $@ -c $(REZFLAGS) $(TMPDIR)/$<
	$(RM) -f $(TMPDIR)/$< $(TMPDIR)/common.rez
.END

# additional dependancies       
apropos.o:	man.h
apropos2.o:	man.h
catman.o:	man.h
common.o:	man.h
fillbuffer.o:	man.h
globals.o:	man.h
makewhatis.o:	man.h
man.o:		man.h
man2.o:		man.h
process.o:	man.h
util.o:		man.h
whatis.o:	man.h
apropos.r:	common.rez
catman.r:	common.rez
makewhatis.r:	common.rez
man.r:		common.rez
whatis.r:	common.rez
