/*                                                                        */
/* descc - compile info file into describe database file                  */
/*                                                                        */
/* $Id: descc.c,v 1.5 1997/09/24 06:34:58 gdr Exp $ */
/*                                                                        */
/*      v1.0.4  -  Changed location of database.  Describe is now         */
/*                 part of the base GNO distribution.  Version numbers    */
/*                 now in lockstep.                                       */
/*                 Devin Reade [Mon 22 Sep 1997]                          */
/*                                                                        */
/*	v1.0.3  -  No changes, but changed version number to keep         */
/*                 in line with descu.                                    */
/*                 Soenke Behrens [Sun Jan 28 1996]                       */
/*                                                                        */
/*      v1.0.2  -  One bug removed. Recompiled to accomodate new          */
/*                 SHELL line and DESCDB environment var                  */
/*                 Soenke Behrens [Sun Oct 22 1995]                       */
/*                                                                        */
/*      v1.0.1  -  Added -h and -V flags  [Sat May 06 1995]               */
/*                 Extracted certain #defines to "desc.h"                 */
/*                 Now uses getopt for command line parsing.              */
/*                 Fixed some potential bugs.                             */
/*                                                                        */
/*      v1.0.0  -  James Brookes   [Sat Oct 23 1993]                      */
/*                 released        [Thu Mar 31 1994]      [!!!!!!!!!!!]   */
/*                                                                        */
/*  This version implements the following features:                       */
/*                                                                        */
/*   o  Compiles a text file which follows the specifications in the      */
/*      included file 'describe.format'.  The format of the describe      */
/*      database is as follows:                                           */ 
/*                                                                        */
/*      Header                                                            */
/*                                                                        */
/*         2 bytes:  Short Int, number of Name Entries                    */
/*                                                                        */
/*      Name Entries                                                      */
/*                                                                        */
/*        34 bytes:  Null-terminated string; name of utility              */
/*         4 bytes:  Long Int, offset of record in file.                  */
/*                                                                        */
/*      Records                                                           */
/*                                                                        */
/*         8 variable-length Null-terminated strings.                     */
/*                                                                        */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef __GNO__
#include <types.h>
#include <gsos.h>
#include <gno/gno.h>
#endif
#include "desc.h"

/* prototypes */

void usage(char *callname);
void version(char *callname);
void puke(int error,int lines);
int  mygets(char *buffer, int2 *lines, FILE *FInPtr);

int Vflag;

/* version - print it out */

void version (char *callname) {
  Vflag++;
  fprintf(stderr,"%s version %s\n",callname,_VERSION_);
  return;
}

/*                             */
/* usage - you know what to do */
/*                             */

void usage (char *callname) {
  if (!Vflag) version(callname);
  fprintf(stderr,"usage: %s [-hV] <describe_sourcefile>\n",callname);
  exit(-1);
}

/*                      */
/* puke - stdlib errors */
/*                      */

void puke (int error,int lines) {
  fprintf(stderr,"\nError $%x in line %d of script\n",error,lines);
  fflush(stdout);
  exit(error);
}

/*                                                                         */
/* mygets - get a line (skipping commented lines) and increment line count */
/*                                                                         */

int mygets (char *buffer, int2 *lines, FILE *FInPtr) {
  char *p, c;

  do {
    if (fgets(buffer,MAX_LINE_LENGTH,FInPtr)==NULL) {
      return(-1);
    }
    p = buffer + strlen(buffer) - 1;        /* remove trailing \n */
    if (*p == '\n') {
      *p = '\0';
    } else {
      fprintf(stderr,"Line %d exceeds %d characters.  Remainder ignored.\n",
              *lines,MAX_LINE_LENGTH-1);
      do {
        c = fgetc(FInPtr);
      } while ((c!='\n') && !feof(FInPtr));
    }
    (*lines)++;
  } while(buffer[0] == QUOTE_CHAR || buffer[0] == '\n'); 
  return(0);
}

#ifdef __GNO__
#define	BIN	0x06

/*
 * Change the file type of <path> to BINary
 */
void
changeToBin(const char *path) {

	FileInfoRecGS	pblock;
	pblock.pCount = 4;
	if ((pblock.pathname = (GSString255Ptr) __C2GSMALLOC(path)) == NULL) {
		/* silent failure */
		return;
	}
	GetFileInfoGS(&pblock);
	if (_toolErr == 0) {
		if (pblock.fileType != BIN) {
			pblock.fileType = BIN;
			pblock.auxType = 0L;
			SetFileInfoGS(&pblock);
		}
	}
	GIfree((GSStringPtr) pblock.pathname);
	return;
}
#endif	/* __GNO__ */

/*              */
/*   Mainline   */
/*              */

int main (int argc, char **argv) {
  FILE *FInPtr, *FOutPtr;
  long int *record_locs, currLoc, endOfFile;
  char *buffer;
  int2 lines, namecount, i, j;
  nameEntry nameStruct;
  int c, errflag;
  char *db_path;

  /* initialize globals */
  Vflag=0;
  errflag=0;
#ifdef __STACK_CHECK__
  _beginStackCheck();
#endif

  assert(sizeof(int2)==2);
  assert(sizeof(int4)==4);

  /* Get database path: If DESCDB is set, use it,
     otherwise use DATABASE */

  if (getenv("DESCDB") == 0)
  {
    if ((db_path = strdup(DATABASE)) == 0)
    {
      fprintf(stderr,"couldn't allocate path variable\n");
      exit (-1);
    }
  } else {
    if ((db_path = strdup(getenv("DESCDB"))) == 0)
    {
      fprintf(stderr,"couldn't allocate path variable\n");
      exit (-1);
    }
  }
     
  /* parse command line */
  while ((c = getopt(argc, argv, "hV")) != EOF) {
    switch (c) {
    case 'V':
      version(basename(argv[0]));
      break;
    case 'h':  /*FALLTHROUGH*/
    default:
      errflag++;
      break;
    }
  }
  if (errflag || (argc-optind != 1))
  {
    free (db_path);
    usage(basename(argv[0]));
  }

  /* open input and output files */

  if ((buffer = malloc (MAX_LINE_LENGTH)) == NULL) {
    fprintf(stderr,"couldn't allocate line buffer\n");
    free (db_path);
    exit (-1);
  }
  
  if ((FInPtr = fopen(argv[argc-1],"r")) == NULL) {
    fprintf(stderr,"Error opening %s; exiting.\n",argv[argc-1]);
    free (db_path);
    free(buffer);
    exit(1);
  }

  if ((FOutPtr = fopen(db_path,"wb+")) == NULL) {
    fprintf(stderr,"Error opening database file %s; exiting.\n",db_path);
    free (db_path);
    free(buffer);
    exit(1);
  }

  /* Compile array of names */
  lines = 0;
  namecount = 0;

  /* space for # of array entries */
  fwrite((void *)&namecount,sizeof(namecount),1,FOutPtr);

  while(mygets(buffer,&lines,FInPtr) != -1) {
    if (!strncmp(buffer,NAME,FIELD_LEN)) {     /* found a match */
      strncpy(nameStruct.name,&buffer[FIELD_LEN],NAME_LEN-1);
      nameStruct.name[NAME_LEN-1] = '\0';
      fwrite((void *)&nameStruct,sizeof(nameStruct),1,FOutPtr);
      namecount++;
    }
  }

  if ((record_locs = malloc (namecount*sizeof(long int)))==NULL) {
    fprintf(stderr,"malloc of record_locs failed (%ld bytes); exiting\n",
            (long) namecount*sizeof(long int));
    exit(-1);
  }
  rewind(FInPtr);
  buffer[0] = '\0';
  lines = 0;
  fputc('\t',FOutPtr);
  /* Increment to first field */

  while (strncmp(buffer,NAME,FIELD_LEN))    /* found a match! */
    mygets(buffer,&lines,FInPtr);

  { /* BUGBUG 22/10/95 Soenke Behrens                      */ 
    /* ORCA/C does not advance the file position indicator */
    /* correctly after above fputc(). This tries to remedy */
    /* the situation. Take out once library has been fixed */
    fprintf(FOutPtr,"Junk");
    fseek(FOutPtr,-4,SEEK_CUR);
  }

  /* Write out records and keep track of their file offsets */
  for (i = 0; i < namecount; i++) {
    record_locs[i] = ftell(FOutPtr);

    /* print out <Version>, <Shell>, <Author>, <Contact>, <Where>, <FTP> */
    for (j = 0; j < FIELD_COUNT-1; j++) {
      buffer[FIELD_LEN] = '\0';
      mygets(buffer,&lines,FInPtr);
      fprintf(FOutPtr,"%s\n",&buffer[FIELD_LEN]);
    }
 
    /* handle <description> field */ 
    for (;;) {
      if (mygets(buffer,&lines,FInPtr) == -1) break;
      if (!strncmp(buffer,NAME,FIELD_LEN)) break;
      fprintf(FOutPtr,"%s ",buffer);
    }
    fputc('\n',FOutPtr);
  }

  endOfFile = ftell(FOutPtr);
  fflush(FOutPtr); /*gdr 1*/
  rewind(FOutPtr);
  fwrite((void *)&namecount,sizeof(namecount),1,FOutPtr);
  fflush(FOutPtr); /*gdr 1*/

  /* time to go through the record_locs array and backpatch in */
  /* all the record locations.  A little slower than necessary */
  /* perhaps, but it gets the job done.                        */

  for (i = 0; i < namecount; i++) {
    fread(&nameStruct,sizeof(nameStruct),1,FOutPtr);
    fseek(FOutPtr,-(sizeof(nameStruct)),SEEK_CUR);
    nameStruct.offset = record_locs[i];
    fwrite((void *)&nameStruct,sizeof(nameStruct),(size_t) 1,FOutPtr);
    fflush(FOutPtr);
  }

  fseek(FOutPtr,endOfFile,SEEK_SET);
  fclose(FOutPtr);

#ifdef __GNO__
  /* change the filetype of the database to BIN */
  changeToBin(db_path);
#endif

  free(db_path);
  free(record_locs);
  free(buffer);

#ifdef __STACK_CHECK__
  fprintf(stderr,"stack usage:  %d bytes\n", _endStackCheck());
#endif
  return 0;
}
