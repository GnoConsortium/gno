#
# makefile for building gsh.  This is for use with dmake.
#
# $Id: Makefile,v 1.5 1998/06/30 17:25:01 tribby Exp $
#
# Devin Reade, February 1998.
#

PROG	= gsh
MAIN	= main
MAINSRC	= main.asm
SRCS	= $(MAINSRC) \
	  shell.asm \
	  history.asm \
	  prompt.asm \
	  cmd.asm \
	  expand.asm \
	  invoke.asm \
	  shellutil.asm \
	  builtin.asm \
	  hash.asm \
	  alias.asm \
	  dir.asm \
	  shellvar.asm \
	  jobs.asm \
	  sv.asm \
	  stdio.asm \
	  orca.asm \
	  edit.asm \
	  term.asm \
	  bufpool.asm
# unused:	mmdebug.asm

# Note: STACK variable does not control size of stack/direct-page segment
#       since this is assembly language. Edit stack segment in main.asm
#       to change this size.

# This is a problem.  gsh was written to use the old termcap library.
# Not only do we no longer build this library, but we don't even have the
# sources for it.  This needs to be changed.  For the moment, use the
# ltermcap library from GNO v2.0.4.
#
# LDADD	= -ltermcap
LDLIBS	+= -l/trenco3/lib.206/ltermcap
###LDLIBS	+= -l/h1/orca/libraries/ltermcap
###LDLIBS	+= -l/usr/lib/libtermcap

# Make sure our macro files get built before trying to assemble source files.
# LOCAL_SETUP	= macros

.INCLUDE : /src/gno/prog.mk

# We give a custom implicit rule for *.o files so that we don't get
# duplicate "dummy" symbols in the link step.
%.o: %.asm
	$(AS) -o $@ $(ASFLAGS) $(__OFLAG) $<
	-$(RM) -f $(OBJ_DIR)$*.root

LOCAL_MACRO_SRC	= gsh.mac
MACRO_SRC = \
	$(LOCAL_MACRO_SRC)

# Everything should be in gsh.mac. If useful, the following could be added
#	/usr/ainclude/m16.gno \
#	$(ORCA_DIST)/libraries/orcainclude/m16.orca \
#	$(ORCA_DIST)/libraries/orcainclude/m16.gs.os \
#	$(ORCA_DIST)/libraries/orcainclude/m16.tools \
#	$(ORCA_DIST)/libraries/orcainclude/m16.shell \
#	$(ORCA_DIST)/libraries/ainclude/m16.pascal \
#	$(ORCA_DIST)/libraries/ainclude/m16.debug

MACROS	= \
	$(OBJ_DIR)main.mac \
	$(OBJ_DIR)alias.mac \
	$(OBJ_DIR)bufpool.mac \
	$(OBJ_DIR)builtin.mac \
	$(OBJ_DIR)cmd.mac \
	$(OBJ_DIR)dir.mac \
	$(OBJ_DIR)edit.mac \
	$(OBJ_DIR)expand.mac \
	$(OBJ_DIR)hash.mac \
	$(OBJ_DIR)history.mac \
	$(OBJ_DIR)invoke.mac \
	$(OBJ_DIR)jobs.mac \
	$(OBJ_DIR)mmdebug.mac \
	$(OBJ_DIR)orca.mac \
	$(OBJ_DIR)prompt.mac \
	$(OBJ_DIR)shell.mac \
	$(OBJ_DIR)shellutil.mac \
	$(OBJ_DIR)shellvar.mac \
	$(OBJ_DIR)stdio.mac \
	$(OBJ_DIR)sv.mac \
	$(OBJ_DIR)term.mac

macros:	$(MACROS)

$(OBJ_DIR)%.mac: %.asm
	@echo making $@
	@$(MACGEN) $(MACGEN_FLAGS) $*.asm $@ $(MACRO_SRC)

$(MACROS): $(LOCAL_MACRO_SRC)

$(OBJ_DIR)alias.mac: $(MACRO_SRC)
	$(MACGEN) $(MACGEN_FLAGS) alias.asm $@ $(MACRO_SRC)

#bork:
#	@echo OBJS are $(OBJS)
#	gsh -c ${mktmp echo $(JINCL) $(JDEFS) $< > $@}

$(ODIR)alias.o:		$(OBJ_DIR)alias.mac
$(ODIR)bufpool.o:	$(OBJ_DIR)bufpool.mac
$(ODIR)builtin.o:	$(OBJ_DIR)builtin.mac
$(ODIR)cmd.o:		$(OBJ_DIR)cmd.mac
$(ODIR)dir.o:		$(OBJ_DIR)dir.mac
$(ODIR)edit.o:		$(OBJ_DIR)edit.mac
$(ODIR)expand.o:	$(OBJ_DIR)expand.mac
$(ODIR)hash.o:		$(OBJ_DIR)hash.mac
$(ODIR)history.o:	$(OBJ_DIR)history.mac
$(ODIR)invoke.o:	$(OBJ_DIR)invoke.mac
$(ODIR)jobs.o:		$(OBJ_DIR)jobs.mac
$(ODIR)main.o:		$(OBJ_DIR)main.mac
$(ODIR)mmdebug.o:	$(OBJ_DIR)mmdebug.mac
$(ODIR)orca.o:		$(OBJ_DIR)orca.mac
$(ODIR)prompt.o:	$(OBJ_DIR)prompt.mac
$(ODIR)shell.o:		$(OBJ_DIR)shell.mac
$(ODIR)shellutil.o:	$(OBJ_DIR)shellutil.mac
$(ODIR)shellvar.o:	$(OBJ_DIR)shellvar.mac
$(ODIR)stdio.o:		$(OBJ_DIR)stdio.mac
$(ODIR)sv.o:		$(OBJ_DIR)sv.mac
$(ODIR)term.o:		$(OBJ_DIR)term.mac
