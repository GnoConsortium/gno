*******
* soon
*
* $Id: TO.DO,v 1.8 1998/10/31 19:14:49 gdr-ftp Exp $
*
*******

When building the binary release, include all the system stuff like
GNOSnooper, et al.

We need a namespace(5) manual page.  Here is a "bugs" entry for it:
	Apropos of building a bootable system, I've also encountered another
	bug.  One cannot use a "partition" defined in /etc/namespace as the
	home directory for any user.  For example, one cannot use "/root" as
	root's home directory unless it is a real volume.  The symptoms of 
	this are:
	        1.  You will get a error about "Device not found"
	        2.  The user's gshrc file will not have been sourced.

Make the source *.shk archive on trenco use proper file types.

The header files <curses.h>, <sys/termios.h>, and <sys/ioctl.compat.h>
have conflicting definitions for FLUSHO.

newuser, newuserv:
	- update to use /home vice /user or maybe a conf file
	- check out behavior of 'adduser', see if there's anything
	  missing

vfscanf.c:
	After a reasonable time, disable the assert calls.

Finish index in kernel reference manual.

in stack(3) man page, the user shouldn't have to allow for extra 
bytes for _assertStack(3).  Determine what is necessary.  If the
number of bytes left is less than this value, then print out
an abbreviated error message rather than calling errx.

Add implementations for Tilghman:
	strftime(3)

Create a src/gno/build.tools/mklink script for duplicating binary
files at user install time.  Current packages that need this:
	compress/zcat
	sum/cksum

As a group:
	Update utmp.5, libutil (logout, logwtmp), last(1), login(1), who(1),
	w(1), init(8), _PATH_UTMP (<utmp.h> and FAQ), rwho(1), users(1)

Add to relevent util man pages:
	.SH ATTRIBUTIONS
	This command was ported from FreeBSD source code
	for distribution with GNO/ME 2.0.6.

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
	runover		8	(obsolete?)  No, needed for single user mode.
	dialup		8


******* 
* defer
*******

investigate using dynamic libraries (RTLs)

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
