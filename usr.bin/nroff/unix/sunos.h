#ifdef SUNOS4

#ifdef FILE
int	_filbuf(FILE *);
int	_flsbuf(unsigned char, FILE *);
int	fclose (FILE *);
int	fflush (FILE *);
int	fprintf(FILE *, const char *, ...);
#endif

#ifdef	ITIMER_REAL
time_t	time (time_t *);
#endif

int	printf(const char *, ...);
int	tolower (int);

#endif	/* SUNOS4 */

