/*
 * High level routines dealing with the output to the screen.
 */
#pragma noroot
#include "less.h"
#include <unistd.h>
#include <fcntl.h>

#ifdef _ORCAC_
#define FAST
segment "LoadSegONE";
#endif

public int errmsgs;	/* Count of messages displayed by error() */
public int need_clr;

extern int sigs;
extern int sc_width;
extern int so_s_width, so_e_width;
extern int screen_trashed;
extern int any_display;
#if __MSDOS__
extern int output_mode;
#endif

static int iprintnum(int num, int radix);
static int iprintf(char *fmt, PARG *parg);

/*
 * Display the line which is in the line buffer.
 */
	public void
put_line(void)
{
	register int c;
	register int i;
	int a;
	int curr_attr;

	if (sigs)
	{
		/*
		 * Don't output if a signal is pending.
		 */
		screen_trashed = 1;
		return;
	}

	curr_attr = NORMAL;

	for (i = 0;  (c = gline(i, &a)) != '\0';  i++)
	{
		if (a != curr_attr)
		{
			/*
			 * Changing attributes.
			 * Display the exit sequence for the old attribute
			 * and the enter sequence for the new one.
			 */
			switch (curr_attr)
			{
			case UNDERLINE:	ul_exit();	break;
			case BOLD:	bo_exit();	break;
			case BLINK:	bl_exit();	break;
			}
			switch (a)
			{
			case UNDERLINE:	ul_enter();	break;
			case BOLD:	bo_enter();	break;
			case BLINK:	bl_enter();	break;
			}
			curr_attr = a;
		}
		if (curr_attr == INVIS)
			continue;
		if (c == '\b')
			putbs();
		else
			putchr(c);
	}
}

/*static char obuf[1024];*/
static char obuf[1024];
static char *ob = obuf;

/*
 * Flush buffered output.
 *
 * If we haven't displayed any file data yet,
 * output messages on error output (file descriptor 2),
 * otherwise output on standard output (file descriptor 1).
 *
 * This has the desirable effect of producing all
 * error messages on error output if standard output
 * is directed to a file.  It also does the same if
 * we never produce any real output; for example, if
 * the input file(s) cannot be opened.  If we do
 * eventually produce output, code in edit() makes
 * sure these messages can be seen before they are
 * overwritten or scrolled away.
 */
	public void
flush(void)
{
	register int n;
	register int fd;

#if __MSDOS__
	if (output_mode == 0)
	{
		*ob = '\0';
		cputs(obuf);
		ob = obuf;
		return;
	}
#endif
	n = ob - obuf;
	if (n == 0)
		return;
/*	fd = (any_display) ? 1 : 2;*/
	fd = (any_display) ? STDOUT_FILENO : STDERR_FILENO;
	if (write(fd, obuf, n) != n)
		screen_trashed = 1;
	ob = obuf;
}

/*
 * Output a character.
 */
#ifdef FAST
public void putchr(c)
	int c;
{
	static char *sz=obuf+sizeof(obuf);
        char *oo;
        asm {
		        lda ob+2
                        sta oo+2
                        lda ob
                        sta oo
                        cmp sz
                        bcc nofl
                        jsl flush
                        lda ob
                        sta oo
                nofl:	lda need_clr
	                beq noncl
                        stz need_clr
                        jsl lower_left
                        jsl clear_eol
               noncl:	lda c
		        cmp #10
                        bne notnl
                        lda #13
               notnl:	pha
		        lda [oo]
                        and #0xff00
                        ora 1,s
                        sta [oo]
                        pla
                        inc ob
            }
}
#else
public void putchr(c)
	int c;
{
	if (ob >= &obuf[sizeof(obuf)])
		flush();
	if (need_clr)
	{
		need_clr = 0;
		lower_left();
		clear_eol();
	}
#if __MSDOS__
	if (c == '\n')
		*ob++ = '\r';
#endif
#ifdef __ORCAC__
	if (c == '\n')
		*ob++ = '\r';
	else
#endif
	*ob++ = c;
}
#endif

/*
 * Output a string.
 */
#ifdef FAST
public void putstr(s)
	register char *s;
{
	static char *sz=obuf+sizeof(obuf);
        char *oo;
	int ytmp;   

	asm {
			stz ytmp
                        lda ob+2
                        sta oo+2
                        lda ob
                        sta oo
                lp:	ldy ytmp
	                lda [s],y
                        and #0xff
                        beq done
                        pha
                      /*  jsl putchr*/
                        lda oo
                        cmp sz
                        bcc nofl
                        jsl flush
                        lda ob
                        sta oo
                nofl:	lda need_clr
	                beq noncl
                        stz need_clr
                        jsl lower_left
                        jsl clear_eol
               noncl:	lda 1,s
		        cmp #10
                        bne notnl
                        lda #13
               notnl:	pha
		        lda [oo]
                        and #0xff00
                        ora 1,s
                        sta [oo]
                        pla
                        pla
                        inc ob
                        inc oo
                        inc ytmp
                        bra lp
                done:
            }
}
#else
public void putstr(s)
	register char *s;
{
	while (*s != '\0')
		putchr(*s++);
}
#endif

/*
 * Output an integer in a given radix.
 */
	static int
iprintnum(num, radix)
	int num;
	int radix;
{
	register char *s;
	int r;
	int neg;
	char buf[10];

	if (neg = (num < 0))
		num = -num;

	s = buf;
	do
	{
		*s++ = (num % radix) + '0';
	} while ((num /= radix) != 0);

	if (neg)
		*s++ = '-';
	r = s - buf;

	while (s > buf)
		putchr(*--s);
	return (r);
}

/*
 * This function implements printf-like functionality
 * using a more portable argument list mechanism than printf's.
 */
	static int
iprintf(fmt, parg)
	register char *fmt;
	PARG *parg;
{
	register char *s;
	register int n;
	register int col;

	col = 0;
	while (*fmt != '\0')
	{
		if (*fmt != '%')
		{
			putchr(*fmt++);
			col++;
		} else
		{
			++fmt;
			switch (*fmt++) {
			case 's':
				s = parg->p_string;
				parg++;
				while (*s != '\0')
				{
					putchr(*s++);
					col++;
				}
				break;
			case 'd':
				n = parg->p_int;
				parg++;
				col += iprintnum(n, 10);
				break;
			}
		}
	}
	return (col);
}

/*
 * Output a message in the lower left corner of the screen
 * and wait for carriage return.
 */
	public void
error(fmt, parg)
	char *fmt;
	PARG *parg;
{
	int c;
	int col = 0;
	static char return_to_continue[] = "  (press RETURN)";

	errmsgs++;

	if (any_display)
	{
		lower_left();
		clear_eol();
		so_enter();
		col += so_s_width;
	}

	col += iprintf(fmt, parg);

	if (!any_display)
	{
/*		putchr('\n');*/
		putchr('\r');
		return;
	}

	putstr(return_to_continue);
	so_exit();
	col += sizeof(return_to_continue) + so_e_width;

#if ONLY_RETURN
	while ((c = getchr()) != '\n' && c != '\r')
		bell();
#else
	c = getchr();
	if (c != '\n' && c != '\r' && c != ' ' && c != READ_INTR)
		ungetcc(c);
#endif
	lower_left();

	if (col >= sc_width)
		/*
		 * Printing the message has probably scrolled the screen.
		 * {{ Unless the terminal doesn't have auto margins,
		 *    in which case we just hammered on the right margin. }}
		 */
		screen_trashed = 1;

	flush();
}

static char intr_to_abort[] = "... (interrupt to abort)";

/*
 * Output a message in the lower left corner of the screen
 * and don't wait for carriage return.
 * Usually used to warn that we are beginning a potentially
 * time-consuming operation.
 */
	public void
ierror(fmt, parg)
	char *fmt;
	PARG *parg;
{
	lower_left();
	clear_eol();
	so_enter();
	(void) iprintf(fmt, parg);
	putstr(intr_to_abort);
	so_exit();
	flush();
	need_clr = 1;
}

/*
 * Output a message in the lower left corner of the screen
 * and return a single-character response.
 */
	public int
query(fmt, parg)
	char *fmt;
	PARG *parg;
{
	register int c;
	int col = 0;

	if (any_display)
	{
		lower_left();
		clear_eol();
	}

	(void) iprintf(fmt, parg);
	c = getchr();

	if (!any_display)
	{
/*		putchr('\n');*/
		putchr('\r');
		return (c);
	}

	lower_left();
	if (col >= sc_width)
		screen_trashed = 1;
	flush();

	return (c);
}
