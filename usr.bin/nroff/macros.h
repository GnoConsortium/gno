/*
 * $Id: macros.h,v 1.1 1997/03/14 06:22:27 gdr Exp $
 *
 * PUTBAK
 *
 *	Push character back into input stream. we use the push-back buffer
 *	stored with macros.
 */
#define PUTBAK(c) \
{ \
    if (mac.ppb == NULL) {  /* first time executing this code */ \
	mac.ppb = mac.pbb; \
	*mac.ppb = c; \
    } else if (mac.ppb < mac.pbb + MAXPBB) { \
	mac.ppb++; \
	*(mac.ppb) = c; \
    } else { \
	errx(-1, "push back buffer overflow (%d chars)", MAXPBB); \
    } \
}

/* NGETC -- get character from input file or push back buffer */
#define NGETC(infp) ((mac.ppb >= mac.pbb) ? *mac.ppb-- : getc(infp))

typedef struct macros_t {
    char   *mnames[MXMDEF];	/* table of ptrs to macro names */
    int	lastp;			/* index to last mname	*/
    char   *emb;		/* next char avail in macro defn buf */
    char 	mb[MACBUF];	/* table of macro definitions */
    char   *ppb;		/* pointer into push back buffer */
    char 	pbb[MAXPBB];	/* push back buffer */
} macros_t;


extern macros_t	mac;

void initMacros (void);
void defmac (char *line, FILE *infp);
char *getmac (char *name);
void maceval (char *p, char *m);
void printmac (int opt);
int putstr (const char *name, const char *p);
char *getstr (char *name);
