/*
 * udl - Convert EOL formats freely between MS-DOS (CR/LF), Unix/Amiga (LF),
 *       and Apple (CR).
 *
 * Header file for routines common to both the Unix and Apple IIgs versions.
 *
 * $Id: common.h,v 1.1 1994/12/13 18:08:23 gdr Exp $
 *
 * Copyright (c) 1993-1994 Soenke Behrens
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>

#define BUFFERSIZE 0x2000
#define PATHLIST_QUANTUM 20
#define UDL_VERSION "Version 1.13"

#ifndef   FALSE
#  define FALSE 0
#  define TRUE !FALSE
#endif

/* we use this macro for macro definitions/declarations */
#ifdef    MAIN
#  define EXTERN
#else
#  define EXTERN extern
#endif

#ifndef   EXIT_FAILURE
#  define EXIT_FAILURE 1
#  define EXIT_SUCCESS 0
#endif

#ifdef HAS_ATEXIT
#  define EXIT(a) exit(a)
#else
#  define EXIT(a) { cleanup(); exit(a); }
#endif

enum file_format { tunix = 1, dos, apple, binary };

/* Since udl is so small, I dare to use some globals :) */
EXTERN char *program_name;         /* How was udl invoked? */
EXTERN char *current_file;         /* Name of current file */
EXTERN char *tempfile;             /* Name of temporary file */
EXTERN unsigned char *in_buffer;   /* My own buffering scheme instead of */
EXTERN unsigned char *out_buffer;  /*   setvbuf()                */
EXTERN int verbose;
EXTERN char filebuffer[MAXPATHLEN];       /* a scratch buffer for file names */
EXTERN char currentDirectory[MAXPATHLEN];
EXTERN char rootdir[MAXPATHLEN];          /* the initial directory */
EXTERN struct stat tstat;  /* temporary variable used to stat files          */
EXTERN int pathSlotsUsed;  /* number of used and available slots in pathList,*/
EXTERN int pathSlots;      /*   respectively.  Both are initially zero.      */
EXTERN char **pathList;    /* the list of files to process, given relative   */
                           /*   to the initial directory.  Initially NULL,   */
                           /*   and NULL terminated.                         */
EXTERN char dirbrk;  /* the character used to separate parts of a path name  */
EXTERN int recursionDepth; /* levels of subdirectories that we've traversed  */
extern int optind;         /* part of getopt library                         */
extern int opterr;

/* Prototypes of functions in common.c */

extern void convert_gs (FILE *infile, FILE *outfile);
extern void convert_messy (FILE *infile, FILE *outfile);
extern void convert_tunix (FILE *infile, FILE *outfile);
extern int convert_fast_gs (FILE *infile, FILE *outfile);
extern int convert_fast_messy (FILE *infile, FILE *outfile);
extern int convert_fast_tunix (FILE *infile, FILE *outfile);
extern enum file_format get_file_format (unsigned char *buffer);
extern FILE *tryopen (char *file, char *mode);
extern int my_fread (FILE *infile, int howmuch);
extern void my_fwrite (unsigned char *buffer, FILE *outfile, int howmuch);
extern void copy_file (char *from, char *to);
extern void cleanup (void);
extern void usage (void);
extern void build_file_list(char *file, short recurse);
extern void add_to_pathList(char *thisdir, char *file);

extern int needsgno(void);

/* not strictly necessary, but it cuts down on warnings from gcc */
#ifdef __GNUC__ 
extern char *getwd(char *);
extern char getopt(int, char **, char *);
#endif
