/*
 * Copyright 1995 by Devin Reade <gdr@myrias.com>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 */

#define VERSIONSTRING "1.2"
#define  ISGRAPH_FIX 1

/* The size of the IO buffers */
#define  BUFFERSIZE  1024

/* The default name for the whatis database */
#define  WHATIS      "whatis"

/* The number of characters per tab in the whatis database */
#define  TABLENGTH 8

#define  DEFAULT_MANPATH   "/usr/man"

extern int  chdir (const char *);
extern int  system (const char *);

void fillbuffer (char *filename);
void process (char *filename, char *tmp_file, FILE *whatis_fp, char *sec);

extern short v_flag;
