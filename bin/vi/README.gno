This version of the STEVIE sources and executable requires the GNO
multitasking environment, because STEVIE uses some GNO system calls
to enable it to work more like the real 'VI'.

This archive contains only the Stevie executable.  Look for the source
archive in the same place you got this one.

This version of STEVIE has been modified to use the termcap library,
allowing STEVIE to operate on a wide range of terminal types, including
gno, vt100, proterm special, etc.
Apple IIgs BBS's now have a full-screen editor for posts, etc.

The termcap file is _not_ provided; look in the same place you got
Stevie for the latest termcap file.
The termcap file should be placed in your $home directory
(see your gshrc file for where $home is) or in directory 31:etc
(choose an existing directory or create a new one, and set prefix 31
in your gshrc).

It shouldn't be too hard to modify STEVIE to work under ORCA, but considering
all the other advantages of GNO, and the growing library of software that
requires it, I recommend you purchase GNO immediately (ok, so this is a plug).

Any comments, suggestions, and bug reports should be sent to:

Internet	bazyar@cs.uiuc.edu
America OnLine	GNO Jawaid
GEnie		PROCYON.INC
Delphi		JBazyar

You know exactly where to send any flames :-)

Jawaid Bazyar


CHANGES

2/3/94
Fixed a MAJOR buffer overflow bug that was causing Stevie to barf a lot
when doing delete lines, undos, etc. Basically, my screen output buffering
routines were not checking for the end of buffer.

7/20/92
Added auto screen-redraw when Stevie is unsuspended from the shell

Fixed termcap support again- we forgot to call tputs in _one_
location, causing cursor goto's to have a '5' placed in front 
of them.  Also took care of the problem where Stevie wouldn't
scroll with vt100 (you need a new termcap file dated 7/19/92 or
later for this).

I now use one of GNO's TextTools features so I know the difference
between right-arrow and control-U.  Happy, Matt? :-)

Sped up screen output slightly by doing buffering and going directly
through the GNO console rather than TextTools.

Claimed Stevie as our own; we won't bother to keep ours current with
the original because the authors cannot be located.

Started optimizing some stuff; done mainly for space considerations
right now (i.e. only turning off stack 'protection')

Sped up file saves ~400% by assembly-izing and using a large, custom
file I/O buffer. (normal.c: 8 seconds to less than 2)

Sped up file loads ~60-80% by using a large, custom file I/O buffer.
any further load speed increases will require changing Stevie's memory
allocation scheme-a proposition that will probably mean rewriting most
of the code in Stevie. (normal.c: 10 seconds to 6)

Typing the suspend character while in insert or replace mode adds
that character to the buffer, rather than suspending Stevie. (e.g.
typing '^Z' will insert a '^Z'.)

4/21/92

Fixed termcap support- now correctly calls termcap routine 'tputs'
to output all term control codes.  Thus, Stevie now works with the
VT100 termcap entry (which has special termcap delay codes, etc in it)
