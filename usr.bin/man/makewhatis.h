#define	VERSIONSTRING "1.1, 29 May 1994"

/* The size of the IO buffers */
#define	BUFFERSIZE	1024

/* The default name for the whatis database */
#define	WHATIS		"whatis"

/* The number of characters per tab in the whatis database */
#define	TABLENGTH 8

extern int	chdir (const char *);
extern int	system (const char *);

void			fillbuffer (char *filename);
void 			process (char *filename, char *tmp_file, FILE *whatis_fp);

extern short v_flag;
