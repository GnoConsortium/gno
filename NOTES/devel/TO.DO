*******
* soon
*
* $Id: TO.DO,v 1.5 1997/11/01 19:34:29 gdr Exp $
*
*******

Verify if cp or gshk is corrupting the "fudgeinstall" script.

Add implementations for Tilghman:
	strftime(3)

Incorporate Tilghman's setttimeofday(2) routine.

Update funopen, fropen, fwopen, fsettext man pages:
	If a user wants binary mode, he has to call fsetbinary
	on the resulting file pointer; this info is not carried
	over from the previous file pointer.

Create a src/gno/build.tools/dupfile script for duplicating binary
files at user install time.  Current packages that need this:
	compress

Complete mkso util

As a group:
	Update utmp.5, libutil (logout, logwtmp), last(1), login(1), who(1),
	w(1), init(8), _PATH_UTMP (<utmp.h> and FAQ), rwho(1), users(1)

Add to relevent util man pages:
	.SH ATTRIBUTIONS
	This command was ported from FreeBSD source code
	for distribution with GNO/ME 2.0.6.

Use __STACK_CHECK__ macro for stack checking code.

stdio:
	- add %b modifier to fprintf implementation (ORCA/C compatibility)

stdlib/cvt.c:
	- handle modes 0 and 1 correctly.

Use result of _setModeEmulation(3) in appropriate calls.  These include
	chmod	-
	fchmod	-
	creat	- done
	open	- done
	stat	- unnecessary
	fstat	- unnecessary
	lstat	- unnecessary

clean up <gno/kerntool.h> -- some require errno args, some don't

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


NEED TO WRITE OR DUPLICATE MAN PAGES FOR
----------------------------------------
	login		libc(3)
	login_tty
	regexp		libc(3)
	termcap		libc(3)
	queue		3
	tty		4
	grp		5
	passwd		5
	termcap		5
	ms		7
	man		7
	cron		8
	runover		8			(obsolete?)  No, needed for single user mode.
	dialup		8


******* 
* defer
*******

libc/sys/syscall.c:
	for unlink, if file is open, then register a call to unlink
	(non-recursive) the file via atexit(3)
	[not practical?]

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

Add locale implementation to libc.

various man pages:
	add note under COMPATIBILITY as to whether the routines
	are thread-safe.  If they write any globals or static
	variables, they are not.

Verify that the setdebug(2) macros in gno/gno.h are correct.

Add implementations for execle; the final
envp can't appear in the prototype, but it can be found since it is 
the last argument after the first NULL pointer.


man(1) implementation: create a database in the user's home directory
	that contains a list of all man pages and their locations, based
	on MANPATH.  Keep a magic number the file to see if MANPATH has
	changed.  *maybe* check the date of the file against the
	modification date of the respective directories and generate a
	warning if the db is out-of-date.

