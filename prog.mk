#
# prog.mk    version 1.1.0  (August 31, 1997)
#
# $Id: prog.mk,v 1.6 1998/02/15 19:44:03 gdr-ftp Exp $
#

# This makefile is intended for use with dmake(1) and occ(1) on Apple IIGS
# It is intended to be .INCLUDE-ed after definition of macros
#   PROG=<program name>
#   SRCS=<source file list; optional if only $(PROG).c>
#   BINDIR=<installation binary directory; default /bin>
#   MANDIR=<installation manual directory; default /usr/man>

#
# You have the option to set several macros (either in the makefile or on
# the dmake command line) that will be recognized by gno.prog.mk:
#
#  *  To turn on debugging, use
#         DEBUG=<parm>
#     where <parm> indicates the ORCA/C "#pragma debug <parm>" value.
#
#  *  To turn on compiler optimizations, use
#         OPTIMIZE=<parm>
#     where <parm> indicates the ORCA/C "#pragma optimize <parm>" value.
#     The default optimization is 72 (0x08=disable fixed parm stack repair;
#     0x40=disable var parm stack repair)
#
#  *  To set a stack size, use
#         STACK=<size>
#     where <size> indicates the ORCA/C "#pragma stacksize <size>" value.
#     The default size is 1024.
#
#  *  To define compiler macros, use
#         DEFINES=-D<name>[=<value>]
#
#  An example using command-line options to turn on debugging, set the
#  stack size to 4096, and to set the "__STACK_CHECK__" macro:
#         dmake DEBUG=25 STACK=4096 DEFINES=-D__STACK_CHECK__

.INCLUDE:	/src/gno/paths.mk
.INCLUDE:	/src/gno/binconst.mk

#
#  Check for user-specified compile/load options
#

# Was debugging requested?
.IF $(DEBUG) != $(NULL)
	CFLAGS	+= -G$(DEBUG)
.END

# Was special optimizing requested?
OPTIMIZE*= 78

# Use stack size of 768 bytes if STACK macro isn't already defined
STACK	*= 1024

# Compile and load flags passed to occ
#   -r: don't create .root file (used on all but main file)
CFLAGS	+= -r -O$(OPTIMIZE) $(DEFINES) -s$(STACK)

# If installation directories were not set, use defaults
BINDIR	*= /bin
MANDIR	*= /usr/man

RELBIN	= $(RELEASE_DIR)$(BINDIR)
RELMAN	= $(RELEASE_DIR)$(MANDIR)

.IF $(CUSTOM_RULES) == $(NULL)
.INCLUDE:	/src/gno/binrules.mk
.END

.IF $(CUSTOM_RELEASE) == $(NULL)
.INCLUDE:	/src/gno/binrelease.mk
.END
