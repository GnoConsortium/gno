/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

/*
 * gchar(lp) - get the character at position "lp" 
 */
#define gchar(lp) ((lp)->linep->s[(lp)->index])

/*
 * pchar(lp, c) - put character 'c' at position 'lp' 
 */
#define pchar(lp, c) ((lp)->linep->s[(lp)->index] = (c))

/*
 * pswap(a, b) - swap two position pointers
 */

#define pswap(a, b) { LPtr \
                            pswaptmp; pswaptmp = a; a = b; b = pswaptmp; }

/*
 * Position comparisons 
 */
#define lt(a, b) ((LINEOF(a) != LINEOF(b)) \
                   ? (LINEOF(a) < LINEOF(b)) : ((a)->index < (b)->index))

#define ltoreq(a, b) ((LINEOF(a) != LINEOF(b)) \
                   ? (LINEOF(a) < LINEOF(b)) : ((a)->index <= (b)->index))

#define gt(a, b) ((LINEOF(a) != LINEOF(b)) \
                   ? (LINEOF(a) > LINEOF(b)) : ((a)->index > (b)->index))

#define gtoreq(a, b) ((LINEOF(a) != LINEOF(b)) \
                   ? (LINEOF(a) > LINEOF(b)) : ((a)->index >= (b)->index))

#define equal(a, b) (((a)->linep == (b)->linep) && ((a)->index == (b)->index))

/*
 * anyinput
 *
 * Return non-zero if input is pending.
 */
#define anyinput() (Readbuffptr != NULL)

/*
 * buf1line() - return TRUE if there is only one line in file buffer
 */
#define buf1line() (Filemem->linep->next == Fileend->linep)

/*
 * bufempty() - return TRUE if the file buffer is empty 
 */
#define bufempty() (buf1line() && Filemem->linep->s[0] == NUL)

/*
 * lineempty() - return TRUE if the line is empty 
 */
#define lineempty(p) ((p)->linep->s[0] == NUL)

/*
 * startofline() - return TRUE if the given position is at start of line 
 */
#define startofline(p) ((p)->index == 0)

/*
 * endofline() - return TRUE if the given position is at end of line 
 *
 * This routine will probably never be called with a position resting on the NUL
 * byte, but handle it correctly in case it happens. 
 */
#define endofline(p) \
     ((p)->linep->s[(p)->index] == NUL || (p)->linep->s[(p)->index + 1] == NUL)

/*
 * RowNumber() - return the row number (if no UndoInProgress)
 */
#define RowNumber(p) (UndoInProgress ? 0 : cntllines(Filemem, (p)))
