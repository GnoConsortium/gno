/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

#ifdef __ORCAC__
segment "seg2";
#include <stdarg.h>
#endif
#include "stevie.h"

static char    *altfile = NULL;	/* alternate file */
static int      altline;	/* line # in alternate file */

static char    *nowrtmsg = "No write since last change (use ! to override)";

extern char   **files;		/* used for "n" and "rew" */
extern int      curfile;
extern int      numfiles;

#ifdef WILD_CARDS
char          **cmd_files = NULL;	/* list of input files */
int             cmd_numfiles = 0;	/* number of input files */
#endif

/*
 * The next two variables contain the bounds of any range given in a command.
 * If no range was given, both contain null line pointers. If only a single
 * line was given, u_pos will contain a null line pointer. 
 */
static LPtr     l_pos, u_pos;

static bool_t   interactive;	/* TRUE if we're reading a real command line */

static bool_t   doecmd(char *);
static void     badcmd(void);
static void     doshell(void);
static void     get_range(char **);
static LPtr    *get_line(char **);

#ifdef  MEGAMAX
overlay "cmdline"
#endif

/*
 * readcmdline() - accept a command line starting with ':', '/', or '?' 
 *
 * readcmdline() accepts and processes colon commands and searches. If
 * 'cmdline' is null, the command line is read here. Otherwise, cmdline
 * points to a complete command line that should be used. This is used in
 * main() to handle initialization commands in the environment variable
 * "EXINIT". 
 */
void
readcmdline(char firstc, char *cmdline)
    /*char            firstc;	/* either ':', '/', or '?' */
    /*char           *cmdline;	/* optional command string */
{
    char            c;
    char 			buff[CMDBUFFSIZE];
    char 			cmdbuf[CMDBUFFSIZE];
    char 			argbuf[CMDBUFFSIZE];
    char           *p, *q, *cmd, *arg;
    bool_t          literal_next_flag = FALSE;

    /*
     * Clear the range variables. 
     */
    l_pos.linep = (LINE *) NULL;
    u_pos.linep = (LINE *) NULL;

    interactive = (cmdline == NULL);

    if (interactive)
	gotocmdline(YES, firstc);
    p = buff;
    if (firstc != ':')
	*p++ = firstc;

    if (interactive) {
	/* collect the command string, handling '\b' and @ */
	for (;;) {
	    c = vgetc();
	    if (c == CTRL('V') && !literal_next_flag) {
		literal_next_flag = TRUE;
		outchar('^');
		continue;
	    }
	    if (c == '\n' || ((c == '\r' || c == ESC) && (!literal_next_flag)))
		break;
	    if ((c == '\b') && (!literal_next_flag)) {
		if (p > buff + (firstc != ':')) {
		    p--;
		    /*
		     * this is gross, but it relies only on 'gotocmdline' 
		     */
		    gotocmdline(YES, firstc == ':' ? ':' : NUL);
		    for (q = buff; q < p; q++)
			outstr(chars[*q].ch_str);
		} else {
		    msg("");
		    return;	/* back to cmd mode */
		}
		continue;
	    }
	    if ((c == '@') && (!literal_next_flag)) {
		p = buff;
    		if (firstc != ':')
		    *p++ = firstc;
		gotocmdline(YES, firstc);
		continue;
	    }
	    if (literal_next_flag) {
		literal_next_flag = FALSE;
		outchar('\b');
	    }
	    outstr(chars[c].ch_str);
	    *p++ = c;
	}
	*p = '\0';
    } else {
	if (strlen(cmdline) > CMDBUFFSIZE - 2)	/* should really do something
						 * better here... */
	    return;
	strcpy(p, cmdline);
    }

    /* skip any initial white space */
    for (cmd = buff; *cmd != NUL && isspace(*cmd); cmd++);

    /* search commands */
    c = *cmd;
    if (c == '/' || c == '?') {
	cmd++;
	/* was the command was '//' or '??' (I.E. repeat last search) */
	if ((*cmd == c) || (*cmd == NUL)) {
	    if (c == '/')
		searchagain(FORWARD);
	    else
		searchagain(BACKWARD);
	    return;
	}
	/* If there is a matching '/' or '?' at the end, toss it */
	p = strchr(cmd, NUL);
	if (*(p - 1) == c && *(p - 2) != '\\')
	    *(p - 1) = NUL;
	dosearch((c == '/') ? FORWARD : BACKWARD, cmd);
	return;
    }
    /*
     * Parse a range, if present (and update the cmd pointer). 
     */
    get_range(&cmd);
    if (l_pos.linep != NULL) {
	if (LINEOF(&l_pos) > LINEOF(&u_pos)) {
	    emsg("Invalid range");
	    return;
	}
    }
    strcpy(cmdbuf, cmd);	/* save the unmodified command */

    /* isolate the command and find any argument */
    for (p = cmd; *p != NUL && !isspace(*p); p++);
    if (*p == NUL)
	arg = NULL;
    else {
	*p = NUL;
	for (p++; *p != NUL && isspace(*p); p++);
	if (*p == NUL) {
	    arg = NULL;
	} else {
	    strcpy(argbuf, p);
	    arg = argbuf;
	}
    }

    if (strcmp(cmd, "q!") == 0) {
	getout(0);
    }
    if (strcmp(cmd, "q") == 0) {
	if (Changed) {
	    emsg(nowrtmsg);
	} else {
	    getout(0);
	}
	return;
    }
    if (strcmp(cmd, "w") == 0) {
	if (arg == NULL) {
	    if (Filename != NULL) {
		if (!writeit(Filename, &l_pos, &u_pos)) {
		    emsg("Problems occured while writing output file");
		}
	    } else {
		emsg("No output file");
	    }
	} else {
	    (void) writeit(arg, &l_pos, &u_pos);
	}
	return;
    }
    if (strcmp(cmd, "wq") == 0) {
	if (Filename != NULL) {
	    if (writeit(Filename, (LPtr *) NULL, (LPtr *) NULL)) {
		getout(0);
	    }
	} else {
	    emsg("No output file");
	}
	return;
    }
    if (strcmp(cmd, "x") == 0) {
	if (Changed) {
	    if (Filename != NULL) {
		if (!writeit(Filename, (LPtr *) NULL, (LPtr *) NULL)) {
		    emsg("Problems occured while writing output file");
		    return;
		}
	    } else {
		emsg("No output file");
		return;
	    }
	}
	getout(0);
    }
    if (strcmp(cmd, "f") == 0 && arg == NULL) {
	fileinfo();
	return;
    }
    if (*cmd == 'n') {
	if ((curfile + 1) < numfiles) {
	    /*
	     * stuff ":e[!] FILE\n" 
	     */
	    stuffReadbuff(":e");
	    if (cmd[1] == '!')
		stuffReadbuff("!");
	    stuffReadbuff(" ");
	    stuffReadbuff(files[++curfile]);
	    stuffReadbuff("\n");
	} else
	    emsg("No more files!");
	return;
    }
    if (*cmd == 'p') {
	if (curfile > 0) {
	    /*
	     * stuff ":e[!] FILE\n" 
	     */
	    stuffReadbuff(":e");
	    if (cmd[1] == '!')
		stuffReadbuff("!");
	    stuffReadbuff(" ");
	    stuffReadbuff(files[--curfile]);
	    stuffReadbuff("\n");
	} else
	    emsg("No more files!");
	return;
    }
    if (strncmp(cmd, "rew", 3) == 0) {
	if (numfiles <= 1)	/* nothing to rewind */
	    return;
	curfile = 0;
	/*
	 * stuff ":e[!] FILE\n" 
	 */
	stuffReadbuff(":e");
	if (cmd[3] == '!')
	    stuffReadbuff("!");
	stuffReadbuff(" ");
	stuffReadbuff(files[0]);
	stuffReadbuff("\n");
	return;
    }
    if (strcmp(cmd, "e") == 0) {
	if (Changed) {
	    emsg(nowrtmsg);
	} else {
	    if (strcmp(arg, "%") == 0) {
		(void) doecmd(NULL);
		return;
	    }
#ifdef WILD_CARDS
	    if (strcmp(arg, "#") != 0) {
		ExpandWildCards(1, &arg, &cmd_numfiles, &cmd_files);
		if (cmd_numfiles == 0) {
		    emsg("Can't open file");
		    return;
		} else if (cmd_numfiles == 1) {
		    arg = cmd_files[0];
		} else {
		    emsg("Too many file names");
		}
	    }
#endif
	    (void) doecmd(arg);
	}
	return;
    }
    if (strcmp(cmd, "e!") == 0) {
	if (strcmp(arg, "%") == 0) {
	    if (!doecmd(NULL))
	        ResetBuffers();
	    return;
	}
#ifdef WILD_CARDS
	if (strcmp(arg, "#") != 0) {
	    ExpandWildCards(1, &arg, &cmd_numfiles, &cmd_files);
	    if (cmd_numfiles == 0) {
		emsg("Can't open file");
		return;
	    } else if (cmd_numfiles == 1) {
		arg = cmd_files[0];
	    } else {
		emsg("Too many file names");
	    }
	}
#endif
	if (!doecmd(arg))
	    ResetBuffers();
	return;
    }
    if (strcmp(cmd, "f") == 0) {
	Filename = strsave(arg);
	filemess("");
	return;
    }
    if (strcmp(cmd, "r") == 0 || strcmp(cmd, ".r") == 0) {
	if (arg == NULL) {
	    badcmd();
	    return;
	}
#ifdef WILD_CARDS
	if (strcmp(arg, "#") != 0) {
	    ExpandWildCards(1, &arg, &cmd_numfiles, &cmd_files);
	    if (cmd_numfiles == 0) {
		emsg("Can't open file");
		return;
	    } else if (cmd_numfiles == 1) {
		arg = cmd_files[0];
	    } else {
		emsg("Too many file names");
	    }
	}
#endif
	if (readfile(arg, Curschar, 1)) {
	    emsg("Can't open file");
	    return;
	}
	ResetBuffers();
	CHANGED;
	return;
    }
    if (strcmp(cmd, ".=") == 0) {
	smsg("line %d", cntllines(Filemem, Curschar));
	return;
    }
    if (strcmp(cmd, "$=") == 0) {
	smsg("%d", cntllines(Filemem, Fileend) - 1);
	return;
    }
    if (strncmp(cmd, "ta", 2) == 0) {
	dotag(arg, cmd[2] == '!');
	return;
    }
    if (strcmp(cmd, "set") == 0) {
	doset(arg, interactive);
	return;
    }
    if (strcmp(cmd, "help") == 0) {
	if (help())
	    s_clear();
	return;
    }
    if (strcmp(cmd, "version") == 0) {
	extern char    *Version;

	msg(Version);
	return;
    }
    if (strcmp(cmd, "sh") == 0) {
	doshell();
	return;
    }
    if (strncmp(cmd, "d", 1) == 0) {
	LINE           *cp;
	int             n;

	if (l_pos.linep == NULL)
	    l_pos = *Curschar;
	if (u_pos.linep == NULL)
	    u_pos = l_pos;

	ResetBuffers();
	n = RowNumber(&l_pos);
	AppendPositionToUndoUndobuff(0, n);
	AppendPositionToUndobuff(0, n);
	if ((Filetop->linep->next == l_pos.linep) &&
	    (u_pos.linep->next == Fileend->linep))
	    AppendToUndobuff("a");
	else if (u_pos.linep->next == Fileend->linep)
	    AppendToUndobuff("o");
	else
	    AppendToUndobuff("O");

	n = 0;
	cp = l_pos.linep;
	for (; cp != NULL && cp != Fileend->linep; cp = cp->next) {
	    AppendToUndobuff(cp->s);
	    n++;
	    if (cp == u_pos.linep)
		break;
	    AppendToUndobuff(NL_STR);
	}
	AppendToUndobuff(ESC_STR);

	if (n > 1)
	    AppendNumberToUndoUndobuff(n);
	AppendToUndoUndobuff("dd");

	*Curschar = l_pos;
	delline(n);
	S_NOT_VALID;
	return;
    }
    if (strncmp(cmd, "s/", 2) == 0) {
	dosub(&l_pos, &u_pos, cmdbuf + 1);
	return;
    }
    if (strncmp(cmd, "g/", 2) == 0) {
	doglob(&l_pos, &u_pos, cmdbuf + 1);
	return;
    }
    if (cmd[0] == '!') {
	if (cmd[1] == '\0') {
	    emsg("Incomplete shell escape command");
	    return;
	}
	outstr("\n");
	flushbuf();
#ifdef BSD
	set_ostate();
#endif
#ifdef UNIX
	set_ostate();
#endif
	(void) system(&cmd[1]);
#ifdef BSD
	set_nstate();
#endif
#ifdef UNIX
	set_nstate();
#endif
	wait_return();
	return;
    }
    /*
     * If we got a line, but no command, then go to the line. 
     */
    if (*cmd == NUL && l_pos.linep != NULL) {
	if (u_pos.linep != NULL)
	    *Curschar = u_pos;
	else
	    *Curschar = l_pos;

	S_CHECK_TOPCHAR_AND_BOTCHAR;

	return;
    }
    badcmd();
}

/*
 * get_range - parse a range specifier 
 *
 * Ranges are of the form: 
 *
 * addr[,addr] 
 *
 * where 'addr' is: 
 *
 * %          (entire file)
 * $  [+-NUM]
 * 'x [+-NUM] (where x denotes a currently defined mark)
 * .  [+-NUM]
 * NUM 
 *
 * The pointer *cp is updated to point to the first character following the
 * range spec. If an initial address is found, but no second, the upper bound
 * is equal to the lower. 
 */
static void
get_range(char **cp)
{
    LPtr           *l;
    char           *p;

    if (**cp == '%') {
	l_pos.index = 0;
	l_pos.linep = Filetop->linep->next;
	u_pos.index = 0;
	u_pos.linep = Fileend->linep->prev;
	(*cp)++;
	return;
    }
    if ((l = get_line(cp)) == NULL)
	return;

    l_pos = *l;

    for (p = *cp; *p != NUL && isspace(*p); p++);

    *cp = p;

    if (*p != ',') {		/* is there another line spec ? */
	u_pos = l_pos;
	return;
    }
    *cp = ++p;

    if ((l = get_line(cp)) == NULL) {
	u_pos = l_pos;
	return;
    }
    u_pos = *l;
}

static LPtr    *
get_line(char **cp)
{
    static LPtr     pos;
    LPtr           *lp;
    char           *p, c;
    int             lnum;

    pos.index = 0;		/* shouldn't matter... check back later */

    p = *cp;
    /*
     * Determine the basic form, if present. 
     */
    switch (c = *p++) {

      case '$':
	pos.linep = Fileend->linep->prev;
	break;

      case '.':
	pos.linep = Curschar->linep;
	break;

      case '\'':
	if ((lp = getmark(*p++)) == NULL) {
	    emsg("Unknown mark");
	    return (LPtr *) NULL;
	}
	pos = *lp;
	break;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
	for (lnum = c - '0'; isdigit(*p); p++)
	    lnum = (lnum * 10) + (*p - '0');

	if (lnum == 0)
	    lnum = 1;

	pos = *gotoline(lnum);
	break;

      default:
	return (LPtr *) NULL;
    }

    while (*p != NUL && isspace(*p))
	p++;

    if (*p == '-' || *p == '+') {
	bool_t          neg = (*p++ == '-');

	for (lnum = 0; isdigit(*p); p++)
	    lnum = (lnum * 10) + (*p - '0');

	if (neg)
	    lnum = -lnum;

	pos = *gotoline(cntllines(Filemem, &pos) + lnum);
    }
    *cp = p;
    return &pos;
}

static void
badcmd(void)
{
    if (interactive)
	emsg("Unrecognized command");
}

/*
 * dotag(tag, force) - goto tag 
 */
void
dotag(char *tag, bool_t force)
    /*char           *tag;
    bool_t          force;*/
{
    FILE           *tp, *fopen();
    char            lbuf[LSIZE];
    char           *fname, *str;

    if ((tp = fopen("tags", "r")) == NULL) {
	emsg("Can't open tags file");
	return;
    }
    while (fgets(lbuf, LSIZE, tp) != NULL) {
	if ((fname = strchr(lbuf, TAB)) == NULL) {
	    emsg("Format error in tags file");
	    return;
	}
	*fname++ = '\0';
	if ((str = strchr(fname, TAB)) == NULL) {
	    emsg("Format error in tags file");
	    return;
	}
	*str++ = '\0';

	if (strcmp(lbuf, tag) == 0) {
	    if (!force && Changed) {
		emsg(nowrtmsg);
		return;
	    }
	    if (doecmd(fname)) {
		stuffReadbuff(str);	/* str has \n at end */
		stuffReadbuff("\007");	/* CTRL('G') */
		fclose(tp);
		return;
	    }
	}
    }
    emsg("tag not found");
    fclose(tp);
}

static          bool_t
doecmd(char *arg)
{
    int             line = 1;	/* line # to go to in new file */

    if (arg != NULL) {
	/*
	 * First detect a ":e" on the current file. This is mainly for ":ta"
	 * commands where the destination is within the current file. 
	 */
	if (Filename != NULL) {
	    if (strcmp(arg, Filename) == 0) {
		if (!Changed) {
	    	    altfile = Filename;
	    	    altline = cntllines(Filemem, Curschar);
		    return TRUE;
		}
	    }
	}
	if (strcmp(arg, "#") == 0) {	/* alternate */
	    char           *s = Filename;

	    if (altfile == NULL) {
		emsg("No alternate file");
		return FALSE;
	    }
	    if (strcmp(altfile, Filename) == 0) {
		if (!Changed) {
	    	    line = altline;
	    	    altline = cntllines(Filemem, Curschar);
		    goto DO_THE_STUFF_THING;
		}
	    }
	    Filename = altfile;
	    altfile = s;
	    line = altline;
	    altline = cntllines(Filemem, Curschar);
	} else {
	    altfile = Filename;
	    altline = cntllines(Filemem, Curschar);
	    Filename = strsave(arg);
	}
    }
    if (Filename == NULL) {
	emsg("No filename");
	return FALSE;
    }
    /* clear mem and read file */
    freeall();
    filealloc();
    UNCHANGED;

    if (readfile(Filename, Filemem, 0)) {
	emsg("Can't open file");
	return FALSE;
    }
    *Topchar = *Curschar;
    if (line != 1) {
DO_THE_STUFF_THING:
	stuffnumReadbuff(line);
	stuffReadbuff("G");
    }
    setpcmark();

    return TRUE;
}

static void
doshell(void)
{
    char           *sh, *getenv();

    sh = getenv("SHELL");
    if (sh == NULL) {
	emsg("Shell variable not set");
	return;
    }
    gotocmdline(YES, NUL);

    if (system(sh) < 0) {
	emsg("Exec failed");
	return;
    }
    wait_return();
}

void
gotocmdline(bool_t clr, char firstc)
{
    windgoto(Rows - 1, 0);
    if (clr)
	toutstr(T_EL);		/* clear the bottom line */
    if (firstc)
	outchar(firstc);
}

/*
 * msg(s) - displays the string 's' on the status line 
 */
void
msg(char *s)
{
    gotocmdline(YES, NUL);
    outstr(s);
#ifdef AMIGA
    flushbuf();
#endif
#ifdef BSD
    flushbuf();
#endif
}

/* VARARGS */
#ifdef __ORCAC__
void smsg(char *s, ...)
{
static char sbuf[MAX_COLUMNS+1];
va_list ap;


    va_start(ap,s);
    vsprintf(sbuf,s,ap);
    msg(sbuf);
    va_end(ap);
}
#else
void
smsg(s, a1, a2, a3, a4, a5, a6, a7, a8, a9)
    char           *s;
    int             a1, a2, a3, a4, a5, a6, a7, a8, a9;
{
    char            sbuf[MAX_COLUMNS + 1];

    sprintf(sbuf, s, a1, a2, a3, a4, a5, a6, a7, a8, a9);
    msg(sbuf);
}
#endif

/*
 * emsg() - display an error message 
 *
 * Rings the bell, if appropriate, and calls message() to do the real work 
 */
void
emsg(char *s)
{
    UndoInProgress = FALSE;
    RedrawingDisabled = FALSE;

    if (P(P_EB))
	beep();
    toutstr(T_TI);
    msg(s);
    toutstr(T_TP);
#ifdef AMIGA
    flushbuf();
#endif
#ifdef BSD
    flushbuf();
#endif
}

void
wait_return(void)
{
    char            c;

    outstr("Press RETURN to continue");
    do {
	c = vgetc();
    } while (c != '\r' && c != '\n');

    s_clear();
}
