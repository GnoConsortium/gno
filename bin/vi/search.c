/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

#ifdef __ORCAC__
segment "search";
#endif

#include "stevie.h"
/* modified Henry Spencer's regular expression routines */
#include "regexp.h"

#ifdef  MEGAMAX
overlay "search"
#endif

/*
 * This file contains various searching-related routines. These fall into
 * three groups: string searches (for /, ?, n, and N), character searches
 * within a single line (for f, F, t, T, etc), and "other" kinds of searches
 * like the '%' command, and 'word' searches. 
 */

/*
 * String searches 
 *
 * The actual searches are done using Henry Spencer's regular expression
 * library. 
 */

#define BEGWORD "([^a-zA-Z0-9_]|^)"	/* replaces "\<" in search strings */
#define ENDWORD "([^a-zA-Z0-9_]|$)"	/* likewise replaces "\>" */

bool_t begword;			/* does the search include a 'begin word'
				 * match */

/*
 * mapstring(s) - map special backslash sequences 
 */
static char    *
mapstring(char *s)
{
    static char     ns[MAX_COLUMNS + 1];
    register char  *p;

    begword = FALSE;

    for (p = ns; *s; s++) {
	if ((*s == '(') || (*s == ')')) {
	    *p++ = '\\';
	    *p++ = *s;
	    continue;
	}
	if (*s != '\\') {	/* not an escape */
	    *p++ = *s;
	    continue;
	}
	switch (*++s) {
	  case '/':
	    *p++ = '/';
	    break;

	  case '<':
	    strcpy(p, BEGWORD);
	    p += strlen(BEGWORD);
	    begword = TRUE;
	    break;

	  case '>':
	    strcpy(p, ENDWORD);
	    p += strlen(ENDWORD);
	    break;

	  default:
	    *p++ = '\\';
	    *p++ = *s;
	    break;
	}
    }
    *p = NUL;

    return ns;
}

static LPtr    *
bcksearch(char *str)
{
    static LPtr     infile;
    register LPtr  *p;
    regexp         *prog;
    register char  *s;
    register int    i;
    bool_t          want_start = (*str == '^');	/* looking for start of line? */
    register char  *match;

    /* make sure str isn't empty */
    if (str == NULL || *str == NUL)
	return NULL;

    prog = regcomp(str);
    if (prog == NULL) {
	emsg("Invalid search string");
	return NULL;
    }
    p = Curschar;
    dec(p);

    if (begword)		/* so we don't get stuck on one match */
	dec(p);

    i = (want_start) ? 0 : p->index;

    do {
	s = p->linep->s;

	if (regexec(prog, s, TRUE)) {	/* match somewhere on line */

	    if (want_start) {	/* could only have been one */
		infile.linep = p->linep;
		infile.index = (int) (prog->startp[0] - s);
		free((char *) prog);
		return (&infile);
	    }
	    /*
	     * Now, if there are multiple matches on this line, we have to
	     * get the last one. Or the last one before the cursor, if we're
	     * on that line. 
	     */

	    match = prog->startp[0];

	    while (regexec(prog, prog->endp[0], FALSE)) {
		if ((i >= 0) && ((prog->startp[0] - s) > i))
		    break;
		match = prog->startp[0];
	    }

	    if ((i >= 0) && ((match - s) > i)) {
		i = -1;
		continue;
	    }
	    infile.linep = p->linep;
	    infile.index = (int) (match - s);
	    free((char *) prog);
	    return (&infile);
	}
	i = -1;

    } while ((p = prevline(p)) != NULL);

    /*
     * If wrapscan isn't set, bag the search now 
     */
    if (!P(P_WS)) {
	free((char *) prog);
	return NULL;
    }
    /* search backward from the end of the file */
    p = prevline(Fileend);
    do {
	s = p->linep->s;

	if (regexec(prog, s, TRUE)) {	/* match somewhere on line */

	    if (want_start) {	/* could only have been one */
		infile.linep = p->linep;
		infile.index = (int) (prog->startp[0] - s);
		free((char *) prog);
		return (&infile);
	    }
	    /*
	     * Now, if there are multiple matches on this line, we have to
	     * get the last one. 
	     */

	    match = prog->startp[0];

	    while (regexec(prog, prog->endp[0], FALSE))
		match = prog->startp[0];

	    infile.linep = p->linep;
	    infile.index = (int) (match - s);
	    free((char *) prog);
	    return (&infile);
	}
	if (p->linep == Curschar->linep)
	    break;

    } while ((p = prevline(p)) != NULL);

    free((char *) prog);
    return NULL;
}

static LPtr    *
fwdsearch(char *str)
{
    static LPtr     infile;
    LPtr           *p;
    regexp         *prog;
    bool_t          want_start = (*str == '^');	/* looking for start of line? */

    char           *s;
    int             i;

    prog = regcomp(str);
    if (prog == NULL) {
	emsg("Invalid search string");
	return NULL;
    }
    p = Curschar;
    i = Curschar->index + 1;
    do {
	s = p->linep->s + i;
	i = 0;

	if (regexec(prog, s, i == 0)) {	/* got a match */
	    /*
	     * If we wanted the start of a line and we aren't really there,
	     * then a match doesn't count. 
	     */
	    if (want_start && (s != p->linep->s))
		continue;

	    infile.linep = p->linep;
	    infile.index = (int) (prog->startp[0] - p->linep->s);
	    free((char *) prog);
	    return (&infile);
	}
    } while ((p = nextline(p)) != NULL);

    /*
     * If wrapscan isn't set, then don't scan from the beginning of the file.
     * Just return failure here. 
     */
    if (!P(P_WS)) {
	free((char *) prog);
	return NULL;
    }
    /* search from the beginning of the file to Curschar */
    for (p = Filemem; p != NULL; p = nextline(p)) {
	s = p->linep->s;

	if (regexec(prog, s, TRUE)) {	/* got a match */
	    infile.linep = p->linep;
	    infile.index = (int) (prog->startp[0] - s);
	    free((char *) prog);
	    return (&infile);
	}
	if (p->linep == Curschar->linep)
	    break;
    }

    free((char *) prog);
    return (NULL);
}

static char    *laststr = NULL;
static int      lastsdir;

static LPtr    *
ssearch(int dir, char *str)
/*    int             dir;	/* FORWARD or BACKWARD */
/*    char           *str;	*/
{
    LPtr           *pos;

    reg_ic = P(P_IC);		/* tell the regexp routines how to search */

    if (laststr != str) {
	if (laststr != NULL)
	    free(laststr);
	laststr = strsave(str);
    }
    lastsdir = dir;

    if (dir == BACKWARD)
	pos = bcksearch(mapstring(str));
    else
	pos = fwdsearch(mapstring(str));

    /*
     * This is kind of a kludge, but its needed to make 'beginning of word'
     * searches land on the right place. 
     */
    if (pos != NULL && begword) {
	if (pos->index != 0)
	    pos->index += 1;
    }
    return pos;
}

bool_t
dosearch(int dir, char *str)
{
    LPtr           *p;

    S_CHECK_TOPCHAR_AND_BOTCHAR;

    if ((p = ssearch(dir, str)) == NULL) {
	msg("Pattern not found");
	return (FALSE);
    } else {
	LPtr            savep;

	/* if we're backing up, we make sure the line we're on */
	/* is on the screen. */
	setpcmark();
	*Curschar = savep = *p;

	return (TRUE);
    }
}

void
searchagain(int dir)
{
    if (laststr == NULL)
	beep();
    else
	dosearch(dir, laststr);

    lastsdir = dir;
}

#define OTHERDIR(x)     (((x) == FORWARD) ? BACKWARD : FORWARD)

bool_t
repsearch(bool_t flag)
{
    int             dir = lastsdir;
    bool_t          found;

    if (laststr == NULL) {
	beep();
	return FALSE;
    }
    found = dosearch(flag ? OTHERDIR(lastsdir) : lastsdir, laststr);

    /*
     * We have to save and restore 'lastsdir' because it gets munged by
     * ssearch() and winds up saving the wrong direction from here if 'flag'
     * is true. 
     */
    lastsdir = dir;

    return (found);
}

/*
 * regerror - called by regexp routines when errors are detected. 
 */
void
regerror(char *s)
{
    emsg(s);
}

/*
 * dosub(lp, up, cmd)
 *
 * Perform a substitution from line 'lp' to line 'up' using the
 * command pointed to by 'cmd' which should be of the form:
 *
 * /pattern/substitution/g
 *
 * The trailing 'g' is optional and, if present, indicates that multiple
 * substitutions should be performed on each line, if applicable.
 * The usual escapes are supported as described in the regexp docs.
 */

void
dosub(LPtr *lp, LPtr *up, char *cmd)
{
    LINE           *cp;
    char           *pat, *sub;
    regexp         *prog;
    int             nsubs;
    bool_t          do_all;	/* do multiple substitutions per line */
    int             n;

    /*
     * If no range was given, do the current line. If only one line was
     * given, just do that one. 
     */
    if (lp->linep == NULL)
	*up = *lp = *Curschar;
    else {
	if (up->linep == NULL)
	    *up = *lp;
    }

    pat = ++cmd;		/* skip the initial '/' */

    while (*cmd) {
	if (cmd[0] == '/' && cmd[-1] != '\\') {
	    *cmd++ = NUL;
	    break;
	}
	cmd++;
    }

    if (*pat == NUL) {
	emsg("NULL pattern specified");
	return;
    }
    sub = cmd;

    do_all = FALSE;

    while (*cmd) {
	if (cmd[0] == '/' && cmd[-1] != '\\') {
	    do_all = (cmd[1] == 'g');
	    *cmd = NUL;
	    break;
	}
	cmd++;
    }

    reg_ic = P(P_IC);		/* set "ignore case" flag appropriately */

    prog = regcomp(pat);
    if (prog == NULL) {
	emsg("Invalid search string");
	return;
    }
    nsubs = 0;

    ResetBuffers();
    n = RowNumber(lp);

    cp = lp->linep;
    for (; cp != Fileend->linep && cp != NULL; cp = cp->next, n++) {
	if (regexec(prog, cp->s, TRUE)) {	/* a match on this line */
	    char           *ns, *sns, *p;

	    /*
	     * Save the line that was last changed for the final cursor
	     * position (just like the real vi). 
	     */
	    Curschar->linep = cp;

	    /*
	     * Get some space for a temporary buffer to do the substitution
	     * into. 
	     */
	    sns = ns = alloc(2048);
	    if (ns == NULL)
		break;

	    *sns = NUL;

	    p = cp->s;

	    do {
		for (ns = sns; *ns; ns++);
		/*
		 * copy up to the part that matched 
		 */
		while (p < prog->startp[0])
		    *ns++ = *p++;

		regsub(prog, sub, ns);

		/*
		 * continue searching after the match 
		 */
		p = prog->endp[0];

	    } while (regexec(prog, p, FALSE) && do_all);

	    for (ns = sns; *ns; ns++);

	    /*
	     * copy the rest of the line, that didn't match 
	     */
	    while (*p)
		*ns++ = *p++;

	    *ns = NUL;

	    AppendPositionToUndoUndobuff(0, n);
	    AppendPositionToUndobuff(0, n);
	    AppendToUndoUndobuff("c$");
	    AppendToUndobuff("c$");
	    AppendToUndoUndobuff(sns);
	    AppendToUndobuff(cp->s);
	    AppendToUndoUndobuff(ESC_STR);
	    AppendToUndobuff(ESC_STR);

	    free(cp->s);	/* free the original line */
	    cp->s = strsave(sns);	/* and save the modified str */
	    cp->size = strlen(cp->s) + 1;
	    free(sns);		/* free the temp buffer */
	    nsubs++;
	}
	if (cp == up->linep)
	    break;
    }

    if (nsubs) {
	CHANGED;
	S_NOT_VALID;
	AppendPositionToUndoUndobuff(0, 1);
	AppendPositionToUndobuff(0, 1);
	beginline(TRUE);
	if (nsubs >= P(P_RP))
	    smsg("%d substitution%c", nsubs, (nsubs > 1) ? 's' : ' ');
    } else
	msg("No match");

    free((char *) prog);
}

/*
 * doglob(cmd)
 *
 * Execute a global command of the form:
 *
 * g/pattern/X
 *
 * where 'x' is a command character, currently one of the following:
 *
 * d    Delete all matching lines
 * p    Print all matching lines
 *
 * The command character (as well as the trailing slash) is optional, and
 * is assumed to be 'p' if missing.
 */

void
doglob(LPtr *lp, LPtr *up, char *cmd)
{
    LINE           *cp;

    char           *pat;
    regexp         *prog;
    int             ndone;
    char            cmdchar = NUL;	/* what to do with matching lines */
    int             nu;
    int             nuu = 0;

    /*
     * If no range was given, do every line. If only one line was given, just
     * do that one. 
     */
    if (lp->linep == NULL) {
	*lp = *Filemem;
	*up = *Fileend;
    } else {
	if (up->linep == NULL)
	    *up = *lp;
    }

    pat = ++cmd;		/* skip the initial '/' */

    while (*cmd) {
	if (cmd[0] == '/' && cmd[-1] != '\\') {
	    cmdchar = cmd[1];
	    *cmd = NUL;
	    break;
	}
	cmd++;
    }
    if (cmdchar == NUL)
	cmdchar = 'p';

    reg_ic = P(P_IC);		/* set "ignore case" flag appropriately */

    if (cmdchar != 'd' && cmdchar != 'p') {
	emsg("Invalid command character");
	return;
    }
    prog = regcomp(pat);
    if (prog == NULL) {
	emsg("Invalid search string");
	return;
    }
    msg("");
    ndone = 0;

    nu = RowNumber(lp);
    if (cmdchar == 'd') {
	ResetBuffers();
	nuu = nu;
    }
    cp = lp->linep;
    for (; cp != Fileend->linep && cp != NULL; cp = cp->next, nu++) {
	if (regexec(prog, cp->s, TRUE)) {	/* a match on this line */
	    Curschar->linep = cp;
	    Curschar->index = 0;

	    switch (cmdchar) {

	      case 'd':	/* delete the line */
		AppendPositionToUndoUndobuff(0, nuu);
		AppendToUndoUndobuff("dd");
		if (buf1line() && (ndone == 0)) {
		    AppendToUndobuff("a");
		} else if (buf1line()) {
		    AppendToUndobuff("j");
		    AppendToUndobuff("I");
		} else if (cp->next == Fileend->linep) {
		    AppendPositionToUndobuff(0, nu);
		    AppendToUndobuff("o");
		} else {
		    AppendPositionToUndobuff(0, nu);
		    AppendToUndobuff("O");
		}
		AppendToUndobuff(cp->s);
		AppendToUndobuff(ESC_STR);

		delline(1);
		break;

	      case 'p':	/* print the line */
		if (P(P_NU)) {
		    outstr(mkline(nu));
		}
		s_cursor_off();
		outstr(format_line(cp->s, (int *) NULL));
		s_cursor_on();
		outstr("\r\n");
		break;
	    }
	    ndone++;
	} else if (cmdchar == 'd') {
	    nuu++;
	}
	if (cp == up->linep)
	    break;
    }

    if (ndone) {
	switch (cmdchar) {

	  case 'd':
	    S_NOT_VALID;
	    AppendPositionToUndobuff(0, 1);
	    if (ndone >= P(P_RP))
		smsg("%d fewer line%c", ndone,
		     (ndone > 1) ? 's' : ' ');
	    break;

	  case 'p':
	    wait_return();
	    break;
	}
	stuffReadbuff("^");
    } else
	msg("No match");

    free((char *) prog);
}

/*
 * Character Searches 
 */

static char     lastc = NUL;	/* last character searched for */
static int      lastcdir;	/* last direction of character search */
static int      lastctype;	/* last type of search ("find" or "to") */

/*
 * searchc(c, dir, type) 
 *
 * Search for character 'c', in direction 'dir'. If type is 0, move to the
 * position of the character, otherwise move to just before the char. 
 */
bool_t
searchc(char c, int dir, int type)
{
    LPtr            save;

    save = *Curschar;		/* save position in case we fail */
    lastc = c;
    lastcdir = dir;
    lastctype = type;

    /*
     * On 'to' searches, skip one to start with so we can repeat searches in
     * the same direction and have it work right. 
     */
    if (type)
	(dir == FORWARD) ? oneright() : oneleft();

    while ((dir == FORWARD) ? oneright() : oneleft()) {
	if (gchar(Curschar) == c) {
	    if (type)
		(dir == FORWARD) ? oneleft() : oneright();
	    return TRUE;
	}
    }
    *Curschar = save;
    return FALSE;
}

bool_t
crepsearch(int flag)
{
    int             dir = lastcdir;
    int             rval;

    if (lastc == NUL)
	return FALSE;

    rval = searchc(lastc, flag ? OTHERDIR(lastcdir) : lastcdir, lastctype);

    lastcdir = dir;		/* restore dir., since it may have changed */

    return rval;
}

/*
 * "Other" Searches 
 */

/*
 * showmatch - move the cursor to the matching paren or brace 
 */
LPtr           *
showmatch(void)
{
    static LPtr     pos;
    int             (*move) (), inc(), dec();
    char            initc = gchar(Curschar);	/* initial char */
    char            findc;	/* terminating char */
    char            c;
    int             count = 0;

    pos = *Curschar;		/* set starting point */

    switch (initc) {

      case '(':
	findc = ')';
	move = inc;
	break;
      case ')':
	findc = '(';
	move = dec;
	break;
      case '{':
	findc = '}';
	move = inc;
	break;
      case '}':
	findc = '{';
	move = dec;
	break;
      case '[':
	findc = ']';
	move = inc;
	break;
      case ']':
	findc = '[';
	move = dec;
	break;
      default:
	return (LPtr *) NULL;
    }

    while ((*move) (&pos) != -1) {	/* until end of file */
	c = gchar(&pos);
	if (c == initc)
	    count++;
	else if (c == findc) {
	    if (count == 0)
		return &pos;
	    count--;
	}
    }
    return (LPtr *) NULL;	/* never found it */
}

/*
 * findfunc(dir) - Find the next function in direction 'dir' 
 *
 * Return TRUE if a function was found. 
 */
bool_t
findfunc(int dir)
{
    LPtr           *curr;

    S_CHECK_TOPCHAR_AND_BOTCHAR;

    curr = Curschar;

    do {
	curr = (dir == FORWARD) ? nextline(curr) : prevline(curr);

	if (curr != NULL && curr->linep->s[0] == '{') {
	    setpcmark();
	    *Curschar = *curr;
	    return TRUE;
	}
    } while (curr != NULL);

    return FALSE;
}

/*
 * The following routines do the word searches performed by the 'w', 'W',
 * 'b', 'B', 'e', and 'E' commands. 
 */

/*
 * To perform these searches, characters are placed into one of three
 * classes, and transitions between classes determine word boundaries. 
 *
 * The classes are: 
 *
 * 0 - white space 1 - letters, digits, and underscore 2 - everything else 
 */

static int      stype;		/* type of the word motion being performed */

#define C0(c)   (((c) == ' ') || ((c) == '\t') || ((c) == NUL))
#define C1(c)   (isalpha(c) || isdigit(c) || ((c) == '_'))

/*
 * cls(c) - returns the class of character 'c' 
 *
 * The 'type' of the current search modifies the classes of characters if a 'W',
 * 'B', or 'E' motion is being done. In this case, chars. from class 2 are
 * reported as class 1 since only white space boundaries are of interest. 
 */
static int
cls(char c)
{
    if (C0(c))
	return 0;

    if (C1(c))
	return 1;

    /*
     * If stype is non-zero, report these as class 1. 
     */
    return (stype == 0) ? 2 : 1;
}


/*
 * fwd_word(pos, type) - move forward one word 
 *
 * Returns the resulting position, or NULL if EOF was reached. 
 */
LPtr           *
fwd_word(LPtr *p, int type)
{
    static LPtr     pos;
    int             sclass = cls(gchar(p));	/* starting class */

    S_CHECK_TOPCHAR_AND_BOTCHAR;

    pos = *p;

    stype = type;

    /*
     * We always move at least one character. 
     */
    if (inc(&pos) == -1)
	return NULL;

    if (sclass != 0) {
	while (cls(gchar(&pos)) == sclass) {
	    if (inc(&pos) == -1)
		return NULL;
	}
	/*
	 * If we went from 1 -> 2 or 2 -> 1, return here. 
	 */
	if (cls(gchar(&pos)) != 0)
	    return &pos;
    }
    /* We're in white space; go to next non-white */

    while (cls(gchar(&pos)) == 0) {
	/*
	 * We'll stop if we land on a blank line 
	 */
	if (pos.index == 0 && pos.linep->s[0] == NUL)
	    break;

	if (inc(&pos) == -1)
	    return NULL;
    }

    return &pos;
}

/*
 * bck_word(pos, type) - move backward one word 
 *
 * Returns the resulting position, or NULL if top-of-file was reached. 
 */
LPtr           *
bck_word(LPtr *p, int type)
{
    static LPtr     pos;
    int             sclass = cls(gchar(p));	/* starting class */

    S_CHECK_TOPCHAR_AND_BOTCHAR;

    pos = *p;

    stype = type;

    if (dec(&pos) == -1)
	return NULL;

    /*
     * If we're in the middle of a word, we just have to back up to the start
     * of it. 
     */
    if (cls(gchar(&pos)) == sclass && sclass != 0) {
	/*
	 * Move backward to start of the current word 
	 */
	while (cls(gchar(&pos)) == sclass) {
	    if (dec(&pos) == -1)
		return NULL;
	}
	inc(&pos);		/* overshot - forward one */
	return &pos;
    }
    /*
     * We were at the start of a word. Go back to the start of the prior
     * word. 
     */

    while (cls(gchar(&pos)) == 0) {	/* skip any white space */
	/*
	 * We'll stop if we land on a blank line 
	 */
	if (pos.index == 0 && pos.linep->s[0] == NUL)
	    return &pos;

	if (dec(&pos) == -1)
	    return NULL;
    }

    sclass = cls(gchar(&pos));

    /*
     * Move backward to start of this word. 
     */
    while (cls(gchar(&pos)) == sclass) {
	if (dec(&pos) == -1)
	    return NULL;
    }
    inc(&pos);			/* overshot - forward one */

    return &pos;
}

/*
 * end_word(pos, type) - move to the end of the word 
 *
 * There is an apparent bug in the 'e' motion of the real vi. At least on the
 * System V Release 3 version for the 80386. Unlike 'b' and 'w', the 'e'
 * motion crosses blank lines. When the real vi crosses a blank line in an
 * 'e' motion, the cursor is placed on the FIRST character of the next
 * non-blank line. The 'E' command, however, works correctly. Since this
 * appears to be a bug, I have not duplicated it here. 
 *
 * Returns the resulting position, or NULL if EOF was reached. 
 */
LPtr           *
end_word(LPtr *p, int type)
{
    static LPtr     pos;
    int             sclass = cls(gchar(p));	/* starting class */

    S_CHECK_TOPCHAR_AND_BOTCHAR;

    pos = *p;

    stype = type;

    if (inc(&pos) == -1)
	return NULL;
    dec(&pos);

    /*
     * If we're in the middle of a word, we just have to move to the end of
     * it. 
     */
    if (cls(gchar(&pos)) == sclass && sclass != 0) {
	/*
	 * Move forward to end of the current word 
	 */
	while (cls(gchar(&pos)) == sclass) {
	    if (inc(&pos) == -1)
		return NULL;
	}
	dec(&pos);		/* overshot - forward one */
	return &pos;
    }
    /*
     * We were at the end of a word. Go to the end of the next word. 
     */

    while (cls(gchar(&pos)) == 0) {	/* skip any white space */
	if (inc(&pos) == -1)
	    return NULL;
    }

    sclass = cls(gchar(&pos));

    /*
     * Move forward to end of this word. 
     */
    while (cls(gchar(&pos)) == sclass) {
	if (inc(&pos) == -1)
	    return NULL;
    }
    dec(&pos);			/* overshot - forward one */

    return &pos;
}
