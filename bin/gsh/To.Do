For more bug reports, see also http://www.gno.org/~gno/bugs.html

Gsh is not currently running for GNO v2.0.6 (when it is rebuilt; the v2.0.4
gsh works fine -- other than outstanding bug reports -- with the v2.0.6
kernel).  This obviously has to be sorted out.

Gsh requires the v2.0.4 ltermcap to link.  Termcap has changed in v2.0.6,
and gsh should be updated to reflect this.

There is a lot of duplication in the macro files.  They should be generated
with macgen from a common source file.  (The constructs to do this are
already in the makefile, but commented out.)

Need to modify Tim's mutex code to be:

t	lda	#1
	tsb	mutex
	beq	go
	cop	$7F
	bra	t
go	; execute code here
------------------------------------

coolish handling of setdebug by editor.

When building the EXE hash table, duplicate entries oughta should be ignored.

When a background process finishes and there's text in the input buffer,
the next keypress correctly reprints the edit line but the key itself does
not get put in the buffer.

running a process in the background from inside a script (not 'source',
but executing the script as a command) causes the shell to wait for that
background process to end - not exactly what we want.

recursive aliases.

multiple files for 'edit'.

'df' lists a bunch appleshare shit.  [gdr:  There is now a stand-alone df
implementation.  The builtin should either use code from the other 
implementation, or be declared obsolecent.

change updatevars to do a read_variable for better performance (and
for correctness!)

-a in ps (and any other redeemable switches)

job control monitor for defunct processes when waiting.

usage for alias and hash

job control needs to save tty information.

echo should use octal and hex \12 is dec, \012 is octal, \x12 is hex.

write new memory management.

write history expansion.
