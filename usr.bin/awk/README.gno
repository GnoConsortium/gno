     ==========================================================
             AWK   Version 2.0: Updated for GNO/ME 2.0.6
             $Id: README.gno,v 1.1 1998/04/07 16:13:08 tribby Exp $
     ==========================================================

AWK is a powerful string processing language that is widely used in the Unix
world.  Jawaid Bazyar, the "father" of GNO/ME, created version 1.00 in
December 1994 by porting scource code available to the public from Bell Labs
(then part of AT&T; now part of Lucent Technologies). The code remains
available  on Brian W. Kernighan's Web page at URL
    http://plan9.bell-labs.com/who/bwk/awk.tar.gz

In addition to adding GNO/ME 2.0.6 compatibility, version 2.0
picks up the latest enhancements and defect fixes from Bell Labs.
(AWK 1.0 was based upon July 1993 code; 2.0 is based upon code
completed in February 1998.)

While this version of AWK has been tested against dozens of sample
AWK programs, its extensive feature set makes exhaustive testing very
difficult. It is certainly conceivable that bugs were introduced by the
porting process; if you find one, please let me know.


==========
Compiling
=========
This package requires the GNO/ME 2.0.6, ORCA/C 2.1, dmake, and occ.
The Makefile uses the standard GNO/ME 2.0.6 declaration files.

The Bell Labs package uses yacc (or bison) to compile awkgram.y to ytab.c
and ytab.h. The program maketab then uses ytab.h to produce proctab.c
For the 2.0 delivery of GNO, I did not compile awkgram.y or proctab.c
on the Apple IIGS but instead produced these on an HP-UX workstation.
Note: the ytab.c shipped with the Bell Labs code uses integers starting at
57346 to represent the symbol definitions, which causes problems for ORCA/C.
The version generated under HP-UX starts with 257.

==================================
Changes To Make AWK Work Under GNO
==================================
The majority of original changes required to make AWK work on the
Apple IIGS involved replacing large procedure-local (stack-based)
arrays and data structures with a call to malloc() at the beginning
of the function and a call to free() at the end. Between 1993 and 1998,
the Bell Labs source code evolved to remove many of the local arrays,
and fewer changes were needed to port the latest version.

In all cases, the original C code was left in place in parallel to the
new code, with #ifdef directives selecting the appropriate version.

The version 1.00 changes were:
	(1) Reduce the size of the data structure (in the case of an array
	    of possible open file pointers; reduce the system's FOPEN_MAX
	    from 32768 to 40 by redefining it (in "run.c").
	(2) Allocate all large local structures via malloc() and free.
	    (in "run.c")
	(3) Set the IIGS OMF load segment names, since AWK is bigger than
	    64K and the code must therefore be segmented.
	(4) In main.c, signal(SIGFPE,fpecatch) was removed, because the
	    IIGS floating point libraries don't send signals on floating
	    point exceptions.

Version 2.0 changes fall into several categories --

Due to changes in the way AWK allocates buffers, and the ORCA/C math.h
header files, several of the changes Jawaid Bazyar introduced were no
longer needed:

  - Most of the buffer allocation/deallocation code was already in
    the latest Bell Labs code. [run.c]

  - Due to declaration in math.h of modf(double, double*), in
    several declarations int variables changed back to double, and
    type casting "(extended)" was removed. [tran.c, run.c]

There were a few changes for compatibility with GNO/ME 2.0.6. These
were marked with "#ifdef __GNO__" to differentiate GNO changes from
ORCA/C changes (see below).

  - Declaration of initenv() was removed and the call to initenv()
    was changed to environInit(). [main.c]

  - Removed unnecessary unprototyped declarations. [ytab.c]

  - Added code to check for stack space when __STACK_CHECK__
    is definef [main.c]

  - Hash function required unsigned long (rather than unsigned)
    intermediate value [tran.c]

  - One module had to be broken into two segments, in order to
    accomodate generated code under full debug mode [run.c]

  - External nodestat is declared but never defined or used; this
    caused a link error when using full debug mode [awk.h]

  - Added ID of 2.0 when printing version [main.c]

  - A make file compatible with dmake and the GNO development
    environment was created [Makefile].

  - Resources for version information and descriptive comment were
    added [awk.rez].

  - Database entry for desc utility was added. [rez.desc]

C code translated to assembly language for performance improvement
is marked with "#if defined(__NOASM__) || !defined(__ORCAC__)". If
necessary to test with the C code rather than assembly, define the
macro __NOASM__.

  - The routine hash() was recoded, resulting in a 10% performance
    boost [tran.c].

Instances of defects in the ORCA/C compiler that required code
modification for successful compilation (marked with #ifndef __ORCAC__)

  - ORCA/C 2.1.0 reports a type conflict for the code segment
        fp = strcmp(s, "-") == 0 ? stdin : fopen(s, "r");
    so different, equivalent code was used. [run.c]

  - ORCA/C 2.1.0 could not parse string.h after macro declarations,
    so #include was moved before declarations. [lex.c]

When required to pass lint, headers were modified to be prototyped
(marked with #ifndef __STDC__) [lex.c, ytab.c]

Due to the way the C source code debuggers work, it was necessary to
comment out all "#line" directives in ytab.c.

==================
Author Information
==================
Version 1.00, December 1994:
     Jawaid Bazyar

Version 2.0, March 1998:
     Dave Tribby
     tribby@cup.hp.com
