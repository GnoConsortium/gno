/*
 * udl - Convert EOL formats freely between MS-DOS (CR/LF), Unix/Amiga (LF),
 *       and Apple (CR).
 *
 * Header file for routines common to both the Unix and Apple IIgs versions.
 *
 * $Id: common.h,v 1.5 1995/02/08 05:47:50 gdr Exp $
 *
 * Copyright (c) 1993-1995 Soenke Behrens, Devin Glyn Reade
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
#define UDL_VERSION "Version 1.14"
#define STACKSIZE 2048
#define BYTES_PER_DEPTH 40
#define BASESIZE 700

#ifndef   FALSE
#  define FALSE 0
#  define TRUE !FALSE
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
extern char *program_name;         /* How was udl invoked? */
extern char *current_file;         /* Name of current file */
extern char *tempfile;             /* Name of temporary file */
extern unsigned char *in_buffer;   /* My own buffering scheme instead of */
extern unsigned char *out_buffer;  /*   setvbuf()                */
extern int verbose;
extern char filebuffer[MAXPATHLEN];       /* a scratch buffer for file names */
extern char currentDirectory[MAXPATHLEN];
extern char rootdir[MAXPATHLEN];          /* the initial directory */
extern struct stat tstat;  /* temporary variable used to stat files          */
extern int pathSlotsUsed;  /* number of used and available slots in pathList,*/
extern int pathSlots;      /*   respectively.  Both are initially zero.      */
extern char **pathList;    /* the list of files to process, given relative   */
                           /*   to the initial directory.  Initially NULL,   */
                           /*   and NULL terminated.                         */
extern char dirbrk;  /* the character used to separate parts of a path name  */
extern int recursionDepth; /* levels of subdirectories that we've traversed  */
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
extern void cleanup (void);
extern void usage (void);
extern void build_file_list(const char *file, short recurse);
extern void add_to_pathList(char *thisdir, char *file);
extern char *mktemp(const char *base);
extern char *get_path(const char *name);

extern int needsgno(void);

/* not strictly necessary, but it cuts down on warnings from gcc */
#ifdef __GNUC__ 
extern char *getwd(char *);
extern char getopt(int, char **, char *);
#endif
