/*
 * Copyright 1995 by Devin Reade <gdr@myrias.com>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 */

/*
 * Configuration info
 */

#define WHATIS  "whatis"
#define PAGER   "/bin/more"
#define SYSCMND "15/syscmnd"

#define AROFF  "aroff"
#define NROFF  "nroff"
#define TROFF  "troff -t"
#define TCAT   "lpr -t"
#define CAT    "cat"
#define EQN    "eqn"
#define REFER  "refer"
#define TBL    "tbl"
#define VGRIND "vgrind"

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

#define BUFFERSIZE 2048

typedef struct Section_tag {
   char *name;    /* section name */
   char *suffix;  /* directory suffix */
} Section;

typedef struct {
   char *suffix;
   char *extractor;
} compressionType;

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

/*
 * from apropos2.c
 */

int  apropos(int argc, char **argv, int whole_line);

/*
 * from whatis2.c
 */

int  whatis(int argc, char **argv);

/*
 * from man2.c
 */
int man (int argc, char *argv[]);


/*
 * from common.c
 */

int getSuffixIndex(char *name);
