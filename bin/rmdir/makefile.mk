# Makefile for rmdir(1) and rmdir(3) v1.0
#
# Devin Reade, <gdr@myrias.ab.ca> November 1994.
#
# Define:
#	SHELL_COMD	if you want to compile the shell command version.
#			In this case, do a 'make rmdir'.  If you want just
#			the rmdir(3) library routine, don't define
#			SHELL_COMD and do a 'make rmdir.o'
#
#	CHECK_STACK	if you want to determine stack usage.  If you select
#			This option, you must also specify the stack library,
#			nominally -l/usr/lib/stack.

CFLAGS = -w -O -DSHELL_COMD -v -s768
LDFLAGS = -v

all: rmdir

install:
	cp -f rmdir /bin
	cp -f rmdir.1 /usr/man/man1
	cp -f rmdir.2 /usr/man/man2
