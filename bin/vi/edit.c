/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

#include "stevie.h"

/*
 * This flag is used to make auto-indent work right on lines where only a
 * <RETURN> or <ESC> is typed. It is set when an auto-indent is done, and
 * reset when any other editting is done on the line. If an <ESC> or <RETURN>
 * is received, and did_ai is TRUE, the line is truncated. 
 */
bool_t          did_ai = FALSE;

static int      replace_num;

void
edit(void)
{
    char            c;
    bool_t          literal_next_flag = FALSE;
    char           *replace_line;
    char           *ptr;
    int             len;
    void	    dosuspend(void);

    Prenum = 0;

    /* position the display and the cursor at the top of the file. */
    *Topchar = *Filemem;
    *Curschar = *Filemem;
    Cursrow = Curscol = 0;

    for (;;) {

	if (!RedrawingDisabled) {
	    /* Figure out where the cursor is based on Curschar. */
	    cursupdate(UPDATE_CURSOR);
	    windgoto(Cursrow, Curscol);
	}
	c = vgetc();

	if (State == NORMAL) {
	    /* We're in the normal (non-insert) mode. */

	    /* Pick up any leading digits and compute 'Prenum' */
	    if (isascii(c)) {	/* must disallow special chars from "ascii.h" */
		if ((Prenum > 0 && isdigit(c)) || (isdigit(c) && c != '0')) {
		    Prenum = Prenum * 10 + (c - '0');
		    continue;
		}
	    }
	    /* execute the command */
	    normal(c);
	    if (State == INSERT && last_command == 'R') {
		ptr = Curschar->linep->s + Curschar->index;
		len = strlen(ptr) + 1;
		replace_line = (char *) NULL;
		replace_num = 0;
		if (len > 1) {
		    replace_line = alloc((unsigned) len);
		    if (replace_line != (char *) NULL)
			strcpy(replace_line, ptr);
		}
	    }
	    Prenum = 0;
	} else {
	    if (c == CTRL('V') && !literal_next_flag) {
		literal_next_flag = TRUE;
		outchar('^');
		continue;
	    }
	    if (literal_next_flag) {
		literal_next_flag = FALSE;
		outchar('\b');
		if (c != NL) {
		    did_ai = FALSE;
		    insertchar(c);
		    continue;
		}
	    }
	    switch (c) {	/* We're in insert mode */

	      case ESC:	/* an escape ends input mode */
	doESCkey:
		dosuspend();
		/*
		 * If we just did an auto-indent, truncate the line, and put
		 * the cursor back. 
		 */
		if (did_ai) {
		    Curschar->linep->s[0] = NUL;
		    Curschar->index = 0;
		    did_ai = FALSE;
		}
		set_want_col = TRUE;

		/*
		 * The cursor should end up on the last inserted character.
		 * This is an attempt to match the real 'vi', but it may not
		 * be quite right yet. 
		 */
		if (Curschar->index != 0) {
		    if (gchar(Curschar) == NUL)
			dec(Curschar);
		    else if (Insbuffptr != NULL)
			dec(Curschar);
		}
		State = NORMAL;
		msg("");

		if (!UndoInProgress) {
		    int             n;
		    char           *p;

		    if (last_command == 'o')
			AppendToUndobuff(UNDO_SHIFTJ_STR);

		    if (Insbuffptr != NULL) {
			if (last_command == 'O')
			    AppendToUndobuff("0");
			AppendToRedobuff(Insbuff);
			AppendToUndoUndobuff(Insbuff);
			n = 0;
			for (p = Insbuff; *p != NUL; p++) {
			    if (*p == NL) {
				if (n) {
				    AppendNumberToUndobuff(n);
				    AppendToUndobuff("dl");
				    n = 0;
				}
				AppendToUndobuff(UNDO_SHIFTJ_STR);
			    } else
				n++;
			}
			if (n) {
			    AppendNumberToUndobuff(n);
			    AppendToUndobuff("dl");
			}
		    }
		    if (last_command == 'c') {
			AppendToUndobuff(mkstr(last_command_char));
			AppendToUndobuff(Yankbuff);
			AppendToUndobuff(ESC_STR);
		    }
		    AppendToRedobuff(ESC_STR);
		    AppendToUndoUndobuff(ESC_STR);
		    if (last_command == 'O')
			AppendToUndobuff(UNDO_SHIFTJ_STR);

		    if (last_command == 'R' && replace_line != (char *) NULL) {
			if (replace_num > 0) {
			    if (replace_num < len) {
				AppendToUndobuff("i");
				replace_line[replace_num] = '\0';
			    } else {
				AppendToUndobuff("a");
			    }
			    AppendToUndobuff(replace_line);
			    AppendToUndobuff(ESC_STR);
			    free(replace_line);
			}
		    }
		}
		break;

	      case CTRL('D'):
		/*
		 * Control-D is treated as a backspace in insert mode to make
		 * auto-indent easier. This isn't completely compatible with
		 * vi, but it's a lot easier than doing it exactly right, and
		 * the difference isn't very noticeable. 
		 */
	      case BS:
		/* can't backup past starting point */
		if (Curschar->linep == Insstart->linep &&
		    Curschar->index <= Insstart->index) {
		    beep();
		    break;
		}
		/* can't backup to a previous line */
		if (Curschar->linep != Insstart->linep &&
		    Curschar->index <= 0) {
		    beep();
		    break;
		}
		did_ai = FALSE;
		dec(Curschar);
		delchar(TRUE, FALSE);
		/*
		 * It's a little strange to put backspaces into the redo
		 * buffer, but it makes auto-indent a lot easier to deal
		 * with. 
		 */
		AppendToInsbuff(BS_STR);
		if (!RedrawingDisabled)	/* screen will be fixed later */
		    S_LINE_NOT_VALID;
		break;

	      case CR:
	      case NL:
		AppendToInsbuff(NL_STR);
		if (!OpenForward(!RedrawingDisabled))
		    goto doESCkey;	/* out of memory */
		break;

	      default:
		did_ai = FALSE;
		insertchar(c);
		break;
	    }
	}
    }
}

/*
 * Special characters in this context are those that need processing other
 * than the simple insertion that can be performed here. This includes ESC
 * which terminates the insert, and CR/NL which need special processing to
 * open up a new line. This routine tries to optimize insertions performed by
 * the "redo", "undo" or "put" commands, so it needs to know when it should
 * stop and defer processing to the "normal" mechanism. 
 */
#define ISSPECIAL(c)    ((c) == BS || (c) == NL || (c) == CR || (c) == ESC)

void
insertchar(char c)
{
    /*
     * If there's any pending input, grab up to MAX_COLUMNS at once. 
     */
    if (anyinput() && (last_command != 'R' || (gchar(Curschar) == NUL))) {
	char            p[MAX_COLUMNS + 1];
	int             i;

	p[0] = c;
	i = 1;
	c = vpeekc();
	while (!ISSPECIAL(c) && anyinput() && (i < MAX_COLUMNS)) {
	    p[i++] = vgetc();
	    c = vpeekc();
	}
	p[i] = '\0';
	insstr(p);
	replace_num += i;
	AppendToInsbuff(p);
    } else {
	inschar(c);
	replace_num++;
	AppendToInsbuff(mkstr(c));
    }

    if (!RedrawingDisabled)	/* screen will be fixed later */
	S_LINE_NOT_VALID;
}

void
getout(int r)
{
    windgoto(Rows - 1, 0);
    outchar('\n');
    windexit(r);
}

void
scrolldown(int nlines)
{
    register LPtr  *p;

    S_MUST_UPDATE_BOTCHAR;
    S_CHECK_TOPCHAR_AND_BOTCHAR;

    /* Scroll up 'nlines' lines. */
    while (nlines--) {
	p = prevline(Topchar);
	if (p == NULL)
	    break;
	Topchar->linep = p->linep;
    }
    /*
     * The calling routine must make sure that Curschar is in the correct
     * place with relation to Botchar. 
     */
}

void
scrollup(int nlines)
{
    register LPtr  *p;

    S_MUST_UPDATE_BOTCHAR;
    S_CHECK_TOPCHAR_AND_BOTCHAR;

    /* Scroll down 'nlines' lines. */
    while (nlines--) {
	p = nextline(Topchar);
	if (p == NULL)
	    break;
	Topchar->linep = p->linep;
    }
    /*
     * The calling routine must make sure that Curschar is in the correct
     * place with relation to Topchar. 
     */
}

/*
 * oneright oneleft onedown oneup 
 *
 * Move one char {right,left,down,up}.  Return TRUE when sucessful, FALSE when
 * we hit a boundary (of a line, or the file). 
 */

bool_t
oneright(void)
{
    set_want_col = TRUE;

    switch (inc(Curschar)) {

      case 0:
	return TRUE;

      case 1:
	dec(Curschar);		/* crossed a line, so back up */
	/* FALLTHROUGH */
      case -1:
	return FALSE;
    }

    return FALSE;		/* PARANOIA: should never reach here */
}

bool_t
oneleft(void)
{
    set_want_col = TRUE;

    switch (dec(Curschar)) {

      case 0:
	return TRUE;

      case 1:
	inc(Curschar);		/* crossed a line, so back up */
	/* FALLTHROUGH */
      case -1:
	return FALSE;
    }

    return FALSE;		/* PARANOIA: should never reach here */
}

void
beginline(bool_t flag)
{
    while (oneleft());
    if (flag) {
	while (isspace(gchar(Curschar)) && oneright());
    }
    set_want_col = TRUE;
}

bool_t
oneup(int n)
{
    register int    k;

    S_CHECK_TOPCHAR_AND_BOTCHAR;

    for (k = 0; k < n; k++) {
	if (Curschar->linep->prev == Filetop->linep) {
	    if (k > 0)
		break;
	    else
		return FALSE;
	}
	Curschar->linep = Curschar->linep->prev;
    }

    /* try to advance to the column we want to be at */
    Curschar->index = 0;
    coladvance(Curschar, Curswant);
    return TRUE;
}

bool_t
onedown(int n)
{
    register int    k;

    S_CHECK_TOPCHAR_AND_BOTCHAR;

    for (k = 0; k < n; k++) {
	if (Curschar->linep->next == Fileend->linep) {
	    if (k > 0)
		break;
	    else
		return FALSE;
	}
	Curschar->linep = Curschar->linep->next;
    }

    /* try to advance to the column we want to be at */
    Curschar->index = 0;
    coladvance(Curschar, Curswant);
    return TRUE;
}
