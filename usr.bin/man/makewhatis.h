#define	VERSIONSTRING "1.0"

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

extern short c_flag;
extern short C_flag;
extern short f_flag;
extern short l_flag;
extern short O_flag;
extern short p_flag;
extern short v_flag;
extern short errflag;
