*******
* soon
*
* $Id: TO.DO,v 1.2 1997/08/08 05:09:51 gdr Exp $
*
*******

Use result of _setModeEmulation(3) in appropriate calls.  These include
	chmod	-
	fchmod	-
	creat	- done
	open	- done
	stat	- unnecessary
	fstat	- unnecessary
	lstat	- unnecessary

specifying 'chmod +x' (effectively) on a file of type TXT or SRC should
change that file to be type SRC and auxtype EXEC.

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

	For the exec* functions, there should be a test for whether
	or not the file is of type SRC and auxtype EXEC.  If that
	is the case, and if the first two characters of the file
	are '#!', then the shell should be exec'd instead.  (currently,
	shell scripts cannot be exec'd)

Fix filename mapping code:

--- inclusion start
 > A#38: The first thing to watch for is known compiler and library bugs.
 >      [...]
 >      validity of pathnames
 >      [...]
 >              - use dynamic directory delimiters.  The ':' character is
 >                always considered to be a directory separator.  The '/'
 >                character is considered to be a directory separator unless
 >                ':' is present, in which case it is part of the file name.
 >                This is the closest to GS/OS, but also has some problems
 >                with POSIX compliance.  For example, the PATH environment
 >                variable is _supposed_ to a list of pathnames delimited
 >                by the ':' character.

GS/OS behaves a little differently than this paragraph describes.  The
first ':' or '/' encountered when scanning the pathname from left to
right is assumed to be the separator.  If a '/' occurs first, then any
':'s appearing later are illegal.  The paragraph suggests that the '/'
will be considered part of the file name if a ':' appears later.  (Your
point about POSIX compliance is still valid, though.)

--- inclusion end

******* 
* defer
*******

libc/sys/syscall.c:
	for unlink, if file is open, then register a call to unlink
	(non-recursive) the file via atexit(3)

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
