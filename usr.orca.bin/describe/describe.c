/*
 * describe(1) -- Copyright 1993-1995 James Brookes.  See the README and
 *                man page for details.
 *
 * Don't use ORCA/C's bit 5 (loop invariant removal) optimization; it
 * kills code somewhere in this file, resulting in a system panic.
 *
 * $Id: describe.c,v 1.5 1997/09/24 06:34:58 gdr Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef __GNO__
#include <gno/gno.h>
#endif
#include "desc.h"

/* prototypes */

void usage(char *callname);
void print_entry(FILE *FInPtr, long int index);
void myprintf(char *string, int wordwrap_size);
char *strdup (const char *str);

int Vflag;

void version (char *callname) {
  Vflag++;
  fprintf(stderr,"%s version %s\n",callname,_VERSION_);
  return;
}

void usage(char *callname) {
  if (!Vflag) version(callname);
  fprintf(stderr,"usage: %s [-hv] <utility_name>\n",callname);
  exit(-1);
}

void myprintf(char *string, int wordwrap_size) {
  int length = 0;
  char *headString, *tailString;
  
  headString = tailString = string;
  printf("\n");
  while (1) {
    tailString++;  length++;
    if (*tailString == '\0') {
      printf("%s",headString);
      return;
    } else if (length == wordwrap_size) {
      while (*tailString != ' ')
        tailString--;
      *tailString = '\0';
      printf("%s\n",headString);
      headString = tailString+1; 
      length = 0;
    }
  }
}

void print_entry(FILE *FInPtr, long int index) {
  char *buffer;
  int i;
  
  if ((buffer = (char *) malloc (1024)) == 0)
  {
    fprintf(stderr,"couldn't allocate buffer\n");
    exit (-1);
  }

  fseek(FInPtr,index,SEEK_SET);
  
  printf("%s",VERSION);
  fgets(buffer,MAX_LINE_LENGTH,FInPtr);
  printf("%s",buffer);

  printf("%s",SHELL);
  fgets(buffer,MAX_LINE_LENGTH,FInPtr);
  printf("%s",buffer);

  printf("%s",AUTHOR);
  fgets(buffer,MAX_LINE_LENGTH,FInPtr);
  printf("%s",buffer);
  
  printf("%s",CONTACT);
  fgets(buffer,MAX_LINE_LENGTH,FInPtr);
  printf("%s",buffer);
  
  printf("%s",WHERE);
  fgets(buffer,MAX_LINE_LENGTH,FInPtr);
  printf("%s",buffer);
  
  printf("%s",FTP);
  fgets(buffer,MAX_LINE_LENGTH,FInPtr);
  printf("%s",buffer);
  
  fgets(buffer,1024,FInPtr);
  myprintf(buffer,75);
  
  free(buffer);
#ifdef __STACK_CHECK__
  printf("Stack: %d\n", _endStackCheck());
#endif
  exit(0);
}

int main (int argc, char **argv) {
  FILE *FInPtr;
  char searchName[NAME_LEN];
  int2 verbose, numOfEntries, cmp, offset1, offset2, check, i;
  nameEntry nameStruct;
  int c, errflag;
  char *p, *tmp;
  char *db_path;

#ifdef __STACK_CHECK__
  _beginStackCheck();
#endif
   
  verbose = FALSE;
  Vflag   = FALSE;
  errflag = FALSE;

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

  while ((c = getopt(argc,argv,"hvV")) != EOF) {
    switch (c) {
    case 'v':
      verbose = TRUE;
      break;
    case 'V':
      version(basename(argv[0]));
      break;
    case 'h':
      default:
      errflag = TRUE;
      break;
    }
  }
  if (errflag || (argc-optind != 1))
  {
    free (db_path);
    usage(basename(argv[0]));
  }


  if ((FInPtr = fopen(db_path,"rb")) == NULL) {
    perror("couldn't open database");
    free (db_path);
    exit(-1);
  }
  fread(&numOfEntries,2,1,FInPtr);
  offset1 = 0;
  offset2 = numOfEntries-1;

  strcpy(searchName,argv[optind]); 
  i=0;
  p = searchName;
  while (*p) {
    *p = tolower(*p);
    p++;
  }

  if (verbose)
    printf("Searching...\n");

  while (1) {
    check = ((offset2-offset1)/2) + offset1;
    fseek(FInPtr,2+(check*sizeof(nameEntry)),SEEK_SET);
    fread(&nameStruct,sizeof(nameEntry),1,FInPtr);

    if((tmp = strdup(nameStruct.name)) == 0)
    {
      fprintf(stderr,"couldn't copy name string\n");
      free (db_path);
      exit (-1);
    }
    p = nameStruct.name;
    while (*p) {
      *p = tolower(*p);
      p++;
    }
    cmp = strcmp(nameStruct.name,searchName);

    if (verbose)
      printf("  checked %s\n",tmp);

    if (cmp > 0) { /* name bigger than searchName */
      offset2 = check-1;
    } else if (cmp < 0) { /* name smaller than searchName */
      offset1 = check+1;
    } else {
      if (verbose) {
        printf("Found entry %s!\n",tmp);
#ifdef __STACK_CHECK__
        printf("Stack: %d\n", _endStackCheck());
#endif
        free (db_path);
        free (tmp);
        exit(0); 
      }

      printf("%s%s\n",NAME,tmp);
      free (db_path);
      free (tmp);
      print_entry(FInPtr,nameStruct.offset);
    }

    if (offset1 > offset2) {
      printf("Entry '%s' not found in describe database.\n",searchName);
#ifdef __STACK_CHECK__
      printf("Stack: %d\n", _endStackCheck());
#endif
      free (db_path);
      free (tmp);
      exit(1);
    }
  }
}
