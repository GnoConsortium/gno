#ifdef __CCFRONT__
#line 2
#endif
/*
 * udl - Convert EOL formats freely between MS-DOS (CR/LF), Unix/Amiga (LF),
 *       and Apple (CR).
 *
 * Contains definitions of global variables declared in common.h
 *
 * $Id: globals.c,v 1.2 1995/02/08 05:47:52 gdr Exp $
 *
 * Copyright (c) 1993-1995 Soenke Behrens, Devin Glyn Reade
 */

#ifdef GNO
#pragma noroot
#endif

#include "common.h"

char *program_name;         /* How was udl invoked? */
char *current_file;         /* Name of current file */
char *tempfile;             /* Name of temporary file */
unsigned char *in_buffer;   /* My own buffering scheme instead of */
unsigned char *out_buffer;  /*   setvbuf()                */
int verbose;
char filebuffer[MAXPATHLEN];       /* a scratch buffer for file names */
char currentDirectory[MAXPATHLEN];
char rootdir[MAXPATHLEN];          /* the initial directory */
struct stat tstat;  /* temporary variable used to stat files          */
int pathSlotsUsed;  /* number of used and available slots in pathList,*/
int pathSlots;      /*   respectively.  Both are initially zero.      */
char **pathList;    /* the list of files to process, given relative   */
                    /*   to the initial directory.  Initially NULL,   */
                    /*   and NULL terminated.                         */
char dirbrk;  /* the character used to separate parts of a path name  */
int recursionDepth; /* levels of subdirectories that we've traversed  */

