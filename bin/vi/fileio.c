/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 *
 * Code Contributions By : Jawaid Bayzar	  bazyar@cs.uiuc.edu
 *			   Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

#include "stevie.h"

segment "s_io";

#ifdef GSOS
#include <gsos.h>
#include <signal.h>
#include <gno/gno.h>
#include <fcntl.h>
FileInfoRecGS finfo = {4, 0l, 0xC3, 0xB0, 0l};
#endif

void
filemess(char *s)
{
    sprintf(IObuff, "\"%s\" %s", ((Filename == NULL) ? "" : Filename), s);
    msg(IObuff);
}

void
renum(void)
{
    LPtr           *p;
    unsigned long   l = 0;

    for (p = Filemem; p != NULL; p = nextline(p), l += LINEINC)
	p->linep->num = l;

    Fileend->linep->num = 0xffffffffL;
}

#ifdef  MEGAMAX
overlay "fileio"
#endif

#ifdef GSOS
#define BUFSIZE1 4096
char 	   *savebuf;
int	    outind;
int 	    fd;

void flushout()
{
    write(fd,savebuf,outind);
    outind = 0;
}
#endif

/* made a bunch of these things unsigned for speed- 816 be dumb */

bool_t
readfile(char *fname, LPtr *fromp, bool_t nochangename)
    /*char           *fname;
    LPtr           *fromp;
    bool_t          nochangename;	/* if TRUE, don't change the Filename */
{
    FILE           *f, *fopen();
    LINE           *curr;
    char            buf2[80];
    int             c,hitEOF = 0;
    unsigned short  IObuffsize = 0;
    unsigned long   nchars = 0;
    unsigned int    linecnt = 0;
    bool_t          wasempty = bufempty();
    unsigned        nonascii = 0;	/* count garbage characters */
    unsigned        nulls = 0;	/* count nulls */
    bool_t          incomplete = FALSE;	/* was the last line incomplete? */
    bool_t          toolong = FALSE;	/* a line was too long */
    char	   *saveb1,*locIObuff;
    unsigned	    inind,insize;

    curr = fromp->linep;
    locIObuff = IObuff;

    if (!nochangename)
	Filename = strsave(fname);

    f = fopen(fname, "r");
    if (f == NULL) {
	s_refresh(NOT_VALID);
	filemess("");
	return TRUE;
    }
#ifdef GSOS
	{
    GSString255Ptr fn;
      fn = malloc(strlen(fname)+2);
      fn->length = strlen(fname);
      strncpy(fn->text,fname,fn->length);
	  finfo.pCount = 4;
      finfo.pathname = fn;
      GetFileInfoGS(&finfo);
      free(fn);
    }
#endif
    S_NOT_VALID;
#ifdef GSOS
    savebuf = malloc(BUFSIZE1+1);
    saveb1 = savebuf;
    insize = inind = 0;
#endif
    do {
	if (inind == insize) {
	    insize = read(fileno(f), savebuf, BUFSIZE1);
            if (!insize) hitEOF = 1;
	    inind = 0;
 	}
	/*c = getc(f);*/
	c = saveb1[inind++];
        if (c == '\r') c = NL;

	if (hitEOF) {
	    if (IObuffsize == 0)/* normal loop termination */
		break;

	    /*
	     * If we get EOF in the middle of a line, note the fact and
	     * complete the line ourselves. 
	     */
	    incomplete = TRUE;
	    c = NL;
	}
	if (c & 0x80) { /* much faster check for hi bit set */
	    c &= 0x7f; /* this is faster, jesus */
	    nonascii++;
	}
	/*
	 * If we reached the end of the line, OR we ran out of space for it,
	 * then process the complete line. 
	 */
	if (c == NL || IObuffsize == (IOSIZE - 1)) {
	    LINE           *lp;

	    if (c != NL)
		toolong = TRUE;

	    locIObuff[IObuffsize++] = NUL;
	    lp = newline(IObuffsize);
	    if (lp == NULL) {
		fprintf(stderr, "not enough memory - should never happen");
		getout(1);
	    }
	    strcpy(lp->s, locIObuff);

	    curr->next->prev = lp;	/* new line to next one */
	    lp->next = curr->next;

	    curr->next = lp;	/* new line to prior one */
	    lp->prev = curr;

	    curr = lp;		/* new line becomes current */
	    IObuffsize = 0;
	    linecnt++;
	} else if (c == NUL) {
	    nulls++;		/* count and ignore nulls */
	} else {
	    locIObuff[IObuffsize++] = (char) c;	/* normal character */
	}

	nchars++;
    } while (!incomplete && !toolong);

    free(savebuf);
    fclose(f);

    /*
     * If the buffer was empty when we started, we have to go back and remove
     * the "dummy" line at Filemem and patch up the ptrs. 
     */
    if (wasempty && linecnt != 0) {
	LINE           *dummy = Filemem->linep;	/* dummy line ptr */

	Filemem->linep = Filemem->linep->next;
	Filemem->linep->prev = Filetop->linep;
	Filetop->linep->next = Filemem->linep;

	Curschar->linep = Filemem->linep;
	Topchar->linep = Filemem->linep;

	free(dummy->s);		/* free string space */
	free((char *) dummy);	/* free LINE struct */
    }
    renum();

    if (toolong) {
	s_refresh(NOT_VALID);

	sprintf(IObuff, "\"%s\" Line too long", fname);
	msg(IObuff);
	return FALSE;
    }
    s_refresh(NOT_VALID);

    sprintf(IObuff, "\"%s\" %s%d line%s, %ld character%s",
	    fname,
	    incomplete ? "[Incomplete last line] " : "",
	    linecnt, (linecnt > 1) ? "s" : "",
	    nchars, (nchars > 1) ? "s" : "");

    buf2[0] = NUL;

    if (nonascii || nulls) {
	if (nonascii) {
	    if (nulls)
		sprintf(buf2, " (%d null, %d non-ASCII)",
			nulls, nonascii);
	    else
		sprintf(buf2, " (%d non-ASCII)", nonascii);
	} else
	    sprintf(buf2, " (%d null)", nulls);
    }
    strcat(IObuff, buf2);
    msg(IObuff);

    return FALSE;
}

/*
 * writeit - write to file 'fname' lines 'start' through 'end' 
 *
 * If either 'start' or 'end' contain null line pointers, the default is to use
 * the start or end of the file respectively. 
 */
bool_t
writeit(char *fname, LPtr *start, LPtr *end)
{
    FILE           *f;
    FILE           *fopen();
    FILE           *fopenb();	/* open in binary mode, where needed */
    char           *s;
    long            nchars;
    int             lines;
    LPtr           *p;
#ifdef GSOS
    int		    linesize,inind;
    char	   *line_ptr,*saveb1;
#endif
    sprintf(IObuff, "\"%s\"", fname);
    msg(IObuff);

#ifdef GSOS
    signal(SIGTSTP,SIG_IGN);
#endif

    /*
     * Form the backup file name - change foo.* to foo.bak - use IObuff to
     * hold the backup file name 
     */
    strcpy(IObuff, fname);
    for (s = IObuff; *s && *s != '.'; s++);
    *s = NUL;
    strcat(IObuff, ".bak");

    /*
     * Delete any existing backup and move the current version to the backup.
     * For safety, we don't remove the backup until the write has finished
     * successfully. And if the 'backup' option is set, leave it around. 
     */
    rename(fname, IObuff);

    f = P(P_CR) ? fopen(fname, "w") : fopenb(fname, "w");
    if (f == NULL) {
	emsg("Can't open file for writing!");
	return FALSE;
    }
#ifdef GSOS
    savebuf = malloc(BUFSIZE1+1); /* allocate our big memory-save buffer */
    saveb1 = savebuf;
#endif
    /*
     * If we were given a bound, start there. Otherwise just start at the
     * beginning of the file. 
     */
    if (start == NULL || start->linep == NULL)
	p = Filemem;
    else
	p = start;

    lines = 0;
    nchars = 0;
#ifdef GSOS
    outind = 0;
    fd = fileno(f);
#endif

    do {
#ifndef GSOS
        fprintf(f, "%s\n", p->linep->s);
#else
        line_ptr = p->linep->s;
        asm {
            stz inind
agin:       ldy inind
	    lda [line_ptr],y
            and #0xff
            beq done

            ldy outind
            cpy #BUFSIZE1
            bcc otaydude
            pha
            jsl flushout
            pla
            ldy #0
otaydude:   sta	[saveb1],y
            iny
            sty outind
            inc inind
            jmp agin

done:	    sty linesize
	    lda #13
            ldy outind
            sta [saveb1],y
            iny
            sty	outind

            lda	linesize
            clc
            adc nchars
            sta nchars
        }
#endif
        lines++;
#ifndef GSOS
        nchars += strlen(p->linep->s) + 1;
#endif
	/*
	 * If we were given an upper bound, and we just did that line, then
	 * bag it now. 
	 */
	if (end != NULL && end->linep != NULL) {
	    if (end->linep == p->linep)
		break;
	}
    } while ((p = nextline(p)) != NULL);
#ifdef GSOS
    flushout();
    free(savebuf);
#endif
    fclose(f);

    /*
     * Remove the backup unless they want it left around 
     */
    if (!P(P_BK))
	remove(IObuff);

    sprintf(IObuff, "\"%s\" %d line%s, %ld character%s", fname,
	    lines, (lines > 1) ? "s" : "",
	    nchars, (nchars > 1) ? "s" : "");
    msg(IObuff);
    UNCHANGED;
#ifdef GSOS
	{
    GSString255Ptr fn;
    extern void stopHandler(int,int);

      fn = malloc(strlen(fname)+2);
      fn->length = strlen(fname);
      strncpy(fn->text,fname,fn->length);
	  finfo.pCount = 4;
      finfo.pathname = fn;
      SetFileInfoGS(&finfo);
      free(fn);
    }
    signal(SIGTSTP, stopHandler); /* handle ^Z with a message */
#endif
    
    return TRUE;
}
