
extern char	PC, *BC, *UP;
extern short	ospeed;

int	tgetent (char *bp, char *name);
int	tgetnum (char *id);
int	tgetflag (char *id);
char *	tgetstr (char *id, char **area);
char *	tgoto (char *cm, int destcol, int destline);
int	tputs (char *cp, int affcnt, int (*outc)(char));
