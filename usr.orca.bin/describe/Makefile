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
LDFLAGS	+= -v
LDLIBS	+= 

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
.IF $(INSTALL) == $(NULL)
	INSTALL	= /usr/bin/install
.END
.IF $(CATREZ) == $(NULL)
	CATREZ	= /usr/bin/catrez
.END
.IF $(DESC_DIR) == $(NULL)
	DESC_DIR= $(RELEASE_DIR)/usr/lib
	DESC_SRC= $(DESC_DIR)/describe.src
.END

build:  describe descc descu

descc:	descc.o $(BASENAME) descc.r
	$(CC) $(LDFLAGS) descc.o $(BASENAME) -o $@ $(LDLIBS)
	$(CATREZ) -d $@ descc.r

describe: describe.o $(BASENAME) describe.r
	$(CC) $(LDFLAGS) describe.o $(BASENAME) -o $@ $(LDLIBS)
	$(CATREZ) -d $@ describe.r

descu:	descu.o $(BASENAME) $(VAEND) descu.r
	$(CC) $(LDFLAGS) descu.o $(BASENAME) $(VAEND) -o $@ $(LDLIBS)
	$(CATREZ) -d $@ descu.r

descc.o:: desc.h
describe.o:: desc.h
descu.o:: desc.h

release:
	$(INSTALL) -d $(RELBIN) $(RELSBIN) $(RELMAN)/man1 $(RELMAN)/man8
	$(INSTALL) -d $(DESC_DIR)
	$(INSTALL) describe        $(RELBIN)
	$(INSTALL) descc descu     $(RELSBIN)
	$(INSTALL) describe.1      $(RELMAN)/man1
	$(INSTALL) descc.8 descu.8 $(RELMAN)/man8
	./descu -o $(DESC_SRC) $(DESC_SRC) $(DESC_FILES)

install:
	$(INSTALL) -d $(BINDIR) $(SBINDIR) $(MANDIR)/man1 $(MANDIR)/man8
	$(INSTALL) describe        $(BINDIR)
	$(INSTALL) descc descu     $(SBINDIR)
	$(INSTALL) describe.1      $(MANDIR)/man1
	$(INSTALL) descc.8 descu.8 $(MANDIR)/man8
