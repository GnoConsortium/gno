/*
 * Copyright 1995-1998 by Devin Reade <gdr@trenco.gno.org>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 *
 * $Id: man.h,v 1.2 1998/03/29 07:16:09 gdr-ftp Exp $
 */

#ifndef	_STDIO_H_
#include <stdio.h>
#endif

#ifndef _GNO_CONTRIB_H_
#include <gno/contrib.h>
#endif

/*
 * Configuration info
 */

#define VERSION_STR	"3.1"

/* Defaults */
#define WHATIS		"whatis"	/* basename of the whatis database */
#define SYSCMND		"15/syscmnd"	/* full path of the ORCA syscmnd file */
#define DEFAULT_MANPATH	"/usr/man"	/* default $MANPATH */

/* The names of various programs */
#define PAGER	"/bin/more"
#define AROFF	"aroff"
#define NROFF	"nroff"
#define TROFF	"troff -t"
#define TCAT	"lpr -t"
#define CAT	"cat"
#define EQN	"eqn"
#define REFER	"refer"
#define TBL	"tbl"
#define VGRIND	"vgrind"

/* The number of characters per tab in the whatis database */
#define  TABLENGTH 8

/* The size of the IO buffers */
#define BUFFERSIZE 2048

/* File types */
#define NON 0x00
#define TXT 0x04
#define BIN 0x06
#define SRC 0xB0

#define MAN_F_MODE  1
#define MAN_K_MODE  2
#define WHATIS_MODE 3
#define ORCA_F_MODE 4
#define ORCA_K_MODE 5
#define ORCA_W_MODE 6


typedef struct Section {
	char *name;    /* section name */
	char *suffix;  /* directory suffix */
} Section;

typedef struct compressionType {
	char *suffix;
	char *extractor;
} compressionType;

typedef struct fileType {
	unsigned int type;
	unsigned long int auxtype;
} fileType, *fileTypePtr;

fileType *	getFileType (const char *file);

/*
 * from globals.c
 */

extern compressionType compressArray[];
extern char linebuf[BUFFERSIZE];
extern char linebuf2[BUFFERSIZE];

/*
 * from man.c
 */

extern Section sections[];

extern char *manpath;
extern char *pager;
extern char *tcat;
extern char *troff;
extern char *macroPackage;

extern short hyphen_flag;
extern short n_flag;
extern short t_flag;
extern short v_flag;

LC_StringArray_t	MakePathArray (char *path);
int		apropos(int argc, char **argv, int whole_line);
void		fillbuffer (char *filename);
char *		getManpath (void);
int		getSuffixIndex(char *name);
char		getcharraw (void);
int		man (int argc, char *argv[]);
char *		newerFile (char *path1, char *path2);
void		process (char *filename, char *tmp_file, FILE *whatis_fp,
			 char *sec);
char *		strcasestr (char *str, char *substr);
int		whatis(int argc, char **argv);

       
