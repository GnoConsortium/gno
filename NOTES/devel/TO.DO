*******
* soon
*
* $Id: TO.DO,v 1.1 1997/02/28 05:42:26 gdr Exp $
*
*******

Use result of _setModeEmulation(3) in appropriate calls.  These include
	chmod	-
	creat	- done
	open	- done
	stat	- unnecessary
	fstat	- unnecessary
	lstat	- unnecessary

check up on the type of nlink_t, used in stat(2)

Write a test that handles:
	- negative process numbers
	- process numbers exceeding INT_MAX and UINT_MAX
the easiest way to do this is to repeatedly exec a program that prints
it's process number.  Keep doing so until either the number wraps or
the machine crashes.

libc/sys/exec.c:
	buildCmd should single-quote strings containing whitespace.
	If the string already has single quotes, then backspace them.

	buildPath should only return files of type S16, EXEC, or EXE.

	buildPath should use the mapPath facility.

	These functions should operate closer to GS/OS when possible.
	Also need to reduce stack requirements.

******* 
* remove from / replace in Orcalib 
*******

strerror	orcalib/string.asm
perror		orcalib/stdio.a
sys_errlist	orcalib/stdio.a			problem
sys_nerr	orcalib/vars.a			problem

******* 
* defer
*******

libc/stdio/perror.c:
	use BSDish implementation for perror(3) to avoid stdio.

libc/sys/syscall.c:
	for unlink, if file is open, then register a call to unlink
	(non-recursive) the file via atexit(3)

libc/sys/trap.asm:
	Insert conchecks in trap functions to check to see if GNO 
	is active and at the right version level.  If not, they
	should abort.

libc/gen/environ.c:
	Review routines.  Force them to use GSString routines, reduce
	stack usage (depend on dynamic allocation for parm blocks).

libc/gen/err.c:
	Use syslogd if not on a tty.

libc/gen/tty.c:
	review these routines

do intro man pages:
	1	done
	2	review DEFINITIONS section
	3	done
	4	done
	5	done
	6	done
	7	done
	8	done

various man pages:
	add note under COMPATIBILITY as to whether the routines
	are thread-safe.  If they write any globals or static
	variables, they are not.

Verify that the setdebug(2) macros in gno/gno.h are correct.

Add implementations for execle; the final
envp can't appear in the prototype, but it can be found since it is 
the last argument after the first NULL pointer.
