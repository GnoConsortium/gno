Last updated: Dec. 18, 1998  By: Dave Tribby

For more bug reports, see http://www.gno.org/~gno/bugs.html

Completed items are reported in file UpdateLog.


Allow redirection of built-in commands' I/O even if they aren't forked.

Rather than have each built-in command always be either forked or
non-forked, check dynamically and only fork when it's really necessary
(background or piped).

Identify limits built into gsh and how they can be changed; for example,
command line cannot expand to > 1024 characters.

When a background process finishes and there's text in the input buffer,
the next keypress correctly reprints the edit line but the key itself does
not get put in the buffer.

Running a process in the background from inside a script (not 'source',
but executing the script as a command) causes the shell to wait for that
background process to end - not exactly what we want.

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
