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

'df' lists a bunch appleshare shit.

change updatevars to do a read_variable for better performance (and
for correctness!)

-a in ps (and any other redeemable switches)

job control monitor for defunct processes when waiting.

usage for alias and hash

job control needs to save tty information.

echo should use octal and hex \12 is dec, \012 is octal, \x12 is hex.

write new memory management.

write history expansion.
