#
# This is the makefile for the describe(1) package.  It is for use
# with dmake(1).
#

# Include configuration files if this is part of the GNO build.
.INCLUDE .IGNORE:	/src/gno/paths.mk
.INCLUDE .IGNORE:	/src/gno/binconst.mk

# Use -D__STACK_CHECK__ in CFLAGS to show stack usage.
# The most recent measurements of stack usage showed:
#	describe	930 bytes
#	descc		943
#	descu		861 bytes

CFLAGS	+= -O78 -s1024 # -D__STACK_CHECK__

# If this is not part of the GNO builds, RELEASE_DIR is not defined
# so things get installed in a live file system instead.
BINDIR		= /usr/bin
SBINDIR		= /usr/sbin
MANDIR		= /usr/man
RELBIN		= $(RELEASE_DIR)$(BINDIR)
RELSBIN		= $(RELEASE_DIR)$(SBINDIR)
RELMAN		= $(RELEASE_DIR)$(MANDIR)

# If you have an old copy of GNO's libc, you may need to compile basename.c
# yourself.  (As of v2.0.6, it's part of the standard libc.)  vaend.c may
# be needed if you are using GNO v2.0.4 with ORCA/C v2.1.x
BASENAME	= # basename.o
VAEND		= # vaend.o

DESC_FILES	= describe.desc descc.desc descu.desc

# A stand-alone build might not have these defined.
INSTALL	*= /usr/bin/install
CATREZ	*= /usr/bin/catrez
.IF $(DESC_DIR) == $(NULL)
	DESC_DIR= $(RELEASE_DIR)/usr/lib
	DESC_SRC= $(DESC_DIR)/describe.src
.END

build:  $(OBJ_DIR) $(OBJ_DIR)describe $(OBJ_DIR)descc $(OBJ_DIR)descu

$(OBJ_DIR):
	$(INSTALL) -d $(OBJ_DIR)

$(OBJ_DIR)descc: descc.o $(BASENAME) descc.r
	$(CC) -o $@ $(LDLIBS) $(LDFLAGS) $(OBJ_DIR)descc.o $(BASENAME)
	$(CATREZ) -d $@ $(OBJ_DIR)descc.r

$(OBJ_DIR)describe: describe.o $(BASENAME) describe.r
	$(CC) -o $@ $(LDLIBS) $(LDFLAGS) $(OBJ_DIR)describe.o $(BASENAME)
	$(CATREZ) -d $@ $(OBJ_DIR)describe.r

$(OBJ_DIR)descu: descu.o $(BASENAME) $(VAEND) descu.r
	$(CC) -o $@ $(LDLIBS) $(LDFLAGS) $(OBJ_DIR)descu.o $(BASENAME) $(VAEND)
	$(CATREZ) -d $@ $(OBJ_DIR)descu.r

descc.o: descc.c desc.h
	$(CC) -o $@ $(CFLAGS:s/ -r / /) -a0 -c descc.c

describe.o: describe.c desc.h
	$(CC) -o $@ $(CFLAGS:s/ -r / /) -a0 -c describe.c

descu.o: descu.c desc.h
	$(CC) -o $@ $(CFLAGS:s/ -r / /) -a0 -c descu.c

.IF $(RELEASE_DIR) == $(NULL)
release:
	@echo there is no release target on a stand alone build
.ELSE
release:
	$(INSTALL) -d $(RELBIN) $(RELSBIN) $(RELMAN)/man1 $(RELMAN)/man8
	$(INSTALL) -d $(DESC_DIR)
	$(INSTALL) $(OBJ_DIR)describe $(RELBIN)
	$(INSTALL) $(OBJ_DIR)descc $(OBJ_DIR)descu $(RELSBIN)
	$(INSTALL) describe.1      $(RELMAN)/man1
	$(INSTALL) descc.8 descu.8 $(RELMAN)/man8
	$(OBJ_DIR)descu -o $(DESC_SRC) $(DESC_SRC) $(DESC_FILES)
.END

install:
	$(INSTALL) -d $(BINDIR) $(SBINDIR) $(MANDIR)/man1 $(MANDIR)/man8
	$(INSTALL) $(OBJ_DIR)describe $(BINDIR)
	$(INSTALL) $(OBJ_DIR)descc $(OBJ_DIR)descu $(SBINDIR)
	$(INSTALL) describe.1      $(MANDIR)/man1
	$(INSTALL) descc.8 descu.8 $(MANDIR)/man8

.IF $(APPLESHARE_CASE_SENSITIVE) != $(NULL)
%.r : %.rez
	$(INSTALL) $< $(TMPDIR)
	cd $(TMPDIR); $(REZ) -o $@ -c $(REZFLAGS) $(TMPDIR)/$<
	$(RM) -f $(TMPDIR)/$<
.END
