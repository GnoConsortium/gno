Last updated: June 19, 1998  By: Dave Tribby

For more bug reports, see also http://www.gno.org/~gno/bugs.html

Completed items are reported in file UpdateLog.


Initialize internal shell variables to match environment variables
(echo, nodirexec, nonewline, noglob, nobeep, pushdsilent, term, and ignoreeof)
when shell first starts.

Gsh requires the v2.0.4 ltermcap to link.  Termcap has changed in v2.0.6,
and gsh should be updated to reflect this.

When building the EXE hash table, duplicate entries oughta should be ignored.

Make PATH's use consistent with forward parsing.

Identify limits built into gsh and how they can be changed; for example,
size of $PATH <= 256; max programs in hash table = 256.

Allow redirection of built-in commands' I/O even if they aren't forked.

When a background process finishes and there's text in the input buffer,
the next keypress correctly reprints the edit line but the key itself does
not get put in the buffer.

running a process in the background from inside a script (not 'source',
but executing the script as a command) causes the shell to wait for that
background process to end - not exactly what we want.

usage for alias and hash

job control monitor for defunct processes when waiting.

job control needs to save tty information.

Allow ~user construct (call routines to get user's home directory)

recursive aliases.

[ -- below this line...probably not -- ]

write new memory management.

coolish handling of setdebug by editor.

echo should use octal and hex \12 is dec, \012 is octal, \x12 is hex.

write history expansion.
