#
# This makefile is intended for use with dmake(1) on Apple IIGS
# $Id: Makefile,v 1.1 1999/11/30 20:28:24 tribby Exp $
#
#	Created by Dave Tribby (beginning June 1998)

# This makefile creates the simple test programs used for gsh testing.
# These two programs are used directly by the dotests command script:
#   echoparams		echo parameters passed from command line;
#									uses reparse.asm assembly language subroutine
#   callsystem		call the system() library subroutine
#
# The remaining two programs can be run to verify the system
#   shellenv			print information on the shell environment
#   testfork			use fork/_execve/wait/kill the way gsh does


#  Make sure lint is turned on
CFLAGS	+= -w

all: echoparams callsystem shellenv testfork

# echoparams has a helper subroutine named reparse.
echoparams.o: echoparams.c

reparse.o: reparse.asm

echoparams: echoparams.c reparse.asm
	17/occ -o echoparams.o -i -w -a0 -c echoparams.c
	17/occ -o reparse.o -i -w -a0 -c reparse.asm
	17/occ -o echoparams echoparams.o reparse.o
	rm -f echoparams.root echoparams.o reparse.root reparse.o

clean:
	rm -f *.o
	rm -f *.root

clobber: clean
	rm -f echoparams callsystem shellenv testfork
        
