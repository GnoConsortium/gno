/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

#include "stevie.h"
#include <texttool.h>
extern char    *Version;

static int      helprow;

#ifdef  HELP

#ifdef  MEGAMAX
overlay "help"
#endif

static void     longline(char *);

bool_t
help(void)
{
char s;
    toutstr(T_ED);
    windgoto(helprow = 0, 0);

    longline("\
Positioning within file\n\
=======================\n\
^F\t\tForward screenfull             Worked on by:\n\
^B\t\tBackward screenfull                Tim Thompson\n");
    longline("\
^D\t\tscroll down half screen            Tony Andrews\n\
^U\t\tscroll up half screen              G.R. (Fred) Walter\n");
    longline("\
G\t\tGoto line (end default)\n\
]]\t\tnext function\n\
[[\t\tprevious function\n\
/re\t\tnext occurence of regular expression 're'\n");
    longline("\
?re\t\tprior occurence of regular expression 're'\n\
n\t\trepeat last / or ?\n\
N\t\treverse last / or ?\n\
%\t\tfind matching (, ), {, }, [, or ]\n");
    longline("\
\n\
Adjusting the screen\n\
====================\n\
^L\t\tRedraw the screen\n\
^E\t\tscroll window down 1 line\n\
^Y\t\tscroll window up 1 line\n");
    longline("\
z<RETURN>\tredraw, current line at top\n\
z-\t\t... at bottom\n\
z.\t\t... at center\n");
/*  s = ReadChar(0);

    windgoto(0, 32);
    longline(Version); */
#ifdef AMIGA
    longline(" ");
    longline(__DATE__);
    longline(" ");
    longline(__TIME__);
#endif

    windgoto(helprow = Rows - 2, 47);
    longline("<Press space bar to continue>\n");
    windgoto(helprow = Rows - 1, 47);
    longline("<Any other key will quit>");

    if (vgetc() != ' ')
	return TRUE;

    toutstr(T_ED);
    windgoto(helprow = 0, 0);

    longline("\
Character Positioning\n\
=====================\n\
^\t\tfirst non-white\n\
0\t\tbeginning of line\n\
$\t\tend of line\n\
h\t\tbackward\n");
    longline("\
l\t\tforward\n\
^H\t\tsame as h\n\
space\t\tsame as l\n\
fx\t\tfind 'x' forward\n");
    longline("\
Fx\t\tfind 'x' backward\n\
tx\t\tupto 'x' forward\n\
Tx\t\tupto 'x' backward\n\
;\t\tRepeat last f, F, t, or T\n");
    longline("\
,\t\tinverse of ;\n\
|\t\tto specified column\n\
%\t\tfind matching (, ), {, }, [, or ]\n");

    windgoto(helprow = Rows - 2, 47);
    longline("<Press space bar to continue>\n");
    windgoto(helprow = Rows - 1, 47);
    longline("<Any other key will quit>");

    if (vgetc() != ' ')
	return TRUE;

    toutstr(T_ED);
    windgoto(helprow = 0, 0);

    longline("\
Line Positioning\n\
================\n\
H\t\thome window line\n\
L\t\tlast window line\n\
M\t\tmiddle window line\n");
    longline("\
+\t\tnext line, at first non-white\n\
-\t\tprevious line, at first non-white\n\
CR\t\treturn, same as +\n\
j\t\tnext line, same column\n\
k\t\tprevious line, same column\n");

    longline("\
\n\
Marking and Returning\n\
=====================\n\
``\t\tprevious context\n\
''\t\t... at first non-white in line\n");
    longline("\
mx\t\tmark position with letter 'x'\n\
`x\t\tto mark 'x'\n\
'x\t\t... at first non-white in line\n");

    windgoto(helprow = Rows - 2, 47);
    longline("<Press space bar to continue>\n");
    windgoto(helprow = Rows - 1, 47);
    longline("<Any other key will quit>");

    if (vgetc() != ' ')
	return TRUE;

    toutstr(T_ED);
    windgoto(helprow = 0, 0);

    longline("\
Insert and Replace\n\
==================\n\
a\t\tappend after cursor\n\
i\t\tinsert before cursor\n\
A\t\tappend at end of line\n\
I\t\tinsert before first non-blank\n");
    longline("\
o\t\topen line below\n\
O\t\topen line above\n\
rx\t\treplace single char with 'x'\n\
R\t\treplace characters (not yet)\n\
~\t\treplace character under cursor with other case\n");

    longline("\
\n\
Words, sentences, paragraphs\n\
============================\n\
w\t\tword forward\n\
b\t\tback word\n\
e\t\tend of word\n\
)\t\tto next sentence (not yet)\n\
}\t\tto next paragraph (not yet)\n");
    longline("\
(\t\tback sentence (not yet)\n\
{\t\tback paragraph (not yet)\n\
W\t\tblank delimited word\n\
B\t\tback W\n\
E\t\tto end of W");

    windgoto(helprow = Rows - 2, 47);
    longline("<Press space bar to continue>\n");
    windgoto(helprow = Rows - 1, 47);
    longline("<Any other key will quit>");

    if (vgetc() != ' ')
	return TRUE;

    toutstr(T_ED);
    windgoto(helprow = 0, 0);

    longline("\
Undo  &  Redo\n\
=============\n\
u\t\tundo last change\n\
U\t\trestore current line (not yet)\n\
.\t\trepeat last change\n");

    longline("\
\n\
File manipulation\n\
=================\n");
    longline("\
:w\t\twrite back changes\n\
:wq\t\twrite and quit\n\
:x\t\twrite if modified, and quit\n\
:q\t\tquit\n\
:q!\t\tquit, discard changes\n\
:e name\t\tedit file 'name'\n");
    longline("\
:e!\t\tre-edit, discard changes\n\
:e #\t\tedit alternate file\n\
:w name\t\twrite file 'name'\n");
    longline("\
:n\t\tedit next file in arglist\n\
:n args\t\tspecify new arglist (not yet)\n\
:rew\t\trewind arglist\n\
:f\t\tshow current file and lines\n");
    longline("\
:f file\t\tchange current file name\n\
:ta tag\t\tto tag file entry 'tag'\n\
^]\t\t:ta, current word is tag");

    windgoto(helprow = Rows - 2, 47);
    longline("<Press space bar to continue>\n");
    windgoto(helprow = Rows - 1, 47);
    longline("<Any other key will quit>");

    if (vgetc() != ' ')
	return TRUE;

    toutstr(T_ED);
    windgoto(helprow = 0, 0);

    longline("\
Operators (double to affect lines)\n\
==================================\n\
d\t\tdelete\n\
c\t\tchange\n");
    longline("\
<\t\tleft shift\n\
>\t\tright shift\n\
y\t\tyank to buffer\n");

    longline("\n\
Yank and Put\n\
============\n\
p\t\tput back text\n\
P\t\tput before\n\
Y\t\tyank lines");

    windgoto(helprow = Rows - 2, 47);
    longline("<Press space bar to continue>\n");
    windgoto(helprow = Rows - 1, 47);
    longline("<Any other key will quit>");

    if (vgetc() != ' ')
	return TRUE;

    toutstr(T_ED);
    windgoto(helprow = 0, 0);

    longline("\n\
Miscellaneous operations\n\
========================\n\
C\t\tchange rest of line\n\
D\t\tdelete rest of line\n\
s\t\tsubstitute chars\n");
    longline("\
S\t\tsubstitute lines (not yet)\n\
J\t\tjoin lines\n\
x\t\tdelete characters\n\
X\t\t... before cursor\n\
:!cmd\t\tsystem(\"cmd\")\n\
:[range]s/search/replace/[g]\n\
:[range]g/search[/p|/d]\n\
:[range]d\tdelete range of lines\n");

    windgoto(helprow = Rows - 1, 47);
    longline("<Press any key>");

    vgetc();

    return TRUE;
}

static void
longline(char *p)
{
# ifdef AMIGA
    outstr(p);
# else
    char           *s;

    for (s = p; *s; s++) {
	if (*s == '\n')
	    windgoto(++helprow, 0);
	else
	    outchar(*s);
    }
# endif
}
#else

bool_t
help(void)
{
    toutstr(T_ED);
    windgoto(0, 0);

    outstr(Version);
    outstr("\n\nWorked on by:\n");
    outstr("\tTim Thompson\n");
    outstr("\tTony Andrews\n");
    outstr("\tG.R. (Fred) Walter\n");
    outstr("\nSorry, help not configured\n");
    outstr("\n<Press any key>");

    vgetc();

    return TRUE;
}
#endif
