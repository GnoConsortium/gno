Last updated: Nov. 26, 1999  By: Dave Tribby

For more bug reports, see http://www.gno.org/~gno/bugs.html

Completed items are reported in file UpdateLog.


Determine why gsh gets SIGTTIN (background read attempted signal) when
it executes a very quickly executing child program.  Currently gsh
quits in order to avoid waiting forever, but that may not always be a
good strategy.

Rather than have each built-in command always be either forked or
non-forked, check dynamically and only fork when it's really necessary
(background or piped). Something like this:
  If process is in background (&) or piped (|), then fork.
  Otherwise: if redirecting I/O, then
                copy file descriptor(s)
                open new stdin, stdout, and/or stderr
  Execute process.
  If I/O was redirected, then
                close redirected file(s)
                reconnect stdin, stdout, and/or stderr
Note: routine "command" sets srcfile and dstfile.

Identify limits built into gsh and how they can be changed; for example,
command line cannot expand to > 1024 characters.

When a background process finishes and there's text in the input buffer,
the next keypress correctly reprints the edit line but the key itself does
not get put in the buffer.

Gsh requires the v2.0.4 ltermcap to link.  A new version of the termcap
library is proposed for GNO v2.0.6, and gsh may require updates.

job control monitor for defunct processes when waiting.

job control needs to save tty information.

Allow ~user construct (call routines to get user's home directory)

recursive aliases.

In expandvars (expand.asm):
 Add error checking if out buf gets too big (> 1024)
 Get rid of fixed buffers

[ -- below this line...probably never will be done -- ]

write new memory management.

coolish handling of setdebug by editor.

echo should use octal and hex \12 is dec, \012 is octal, \x12 is hex.

write history expansion.
