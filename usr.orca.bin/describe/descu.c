/*
 * descu - describe(1) update utility for maintaining describe source files
 *
 * Usage:  descu [-hV] sourcefile patchfile1 [patchfile2 ...]
 *
 * Options:
 *         -h    show usage information and exit.
 *         -V    show version information
 *
 * Copyright 1995 by Devin Reade for James Brookes' describe(1) utility.
 * See the included README file and man page for details.
 *
 * $Id: descu.c,v 1.2 1996/01/22 02:40:49 gdr Exp $
 */

#pragma optimize -1

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include "desc.h"

#define _VERSION_     "v1.0.2"
#define MAX_BUFFER    65534
#define SLOTS_QUANTUM 20
#define REJECT_FILE   "descu.rej"

#ifndef __ORCAC__
ssize_t read(int, void *, size_t);
#endif
char *strerror(int);
void convert (char *);
int my_stricmp (const char *cs, const char *ct);

char *versionStr = _VERSION_;
static char *header=NULL;    /* comments before the first describe entry */
static char *trailer=NULL;   /* comments after the last describe entry */

short oflag;
short Vflag;
short errflag;

descEntry **entryArray1=NULL;
descEntry **entryArray2=NULL;
int array1SlotsAlloced=0;
int array2SlotsAlloced=0;
int array1SlotsUsed=0;
int array2SlotsUsed=0;

/*
 * inhale - read file into buffer
 *
 * Pre:  <pathname> is the path name of the file to read in
 *
 * Post: returns a malloc'd NULL-terminated buffer containing the contents
 *       of file <pathname>.  On error, returns NULL and prints a suitable
 *       message.
 *
 *       On the Apple IIgs, CR's are also converted to LF's
 */

char *inhale (char *pathname) {
  char *buffer;
  long bytecount, bytes_read;
  ssize_t i;
  int fd;

  /* open the file */
  if ((fd = open(pathname,O_RDONLY))==-1) {
    fprintf(stderr,"inhale: open of %s failed: %s\n",
            pathname,strerror(errno));
    return NULL;
  }

  /* create the buffer */
  bytecount = lseek(fd,(off_t) 0,SEEK_END);
  if (bytecount > MAX_BUFFER) {
    fprintf(stderr,"descu internal error: cannot handle files greater"
                   "than %d bytes\n due to a compiler bug.  Sorry.\n",
                   MAX_BUFFER);
    exit(-1);
  }
  lseek(fd,(off_t) 0, SEEK_SET);
  if ((buffer = malloc(bytecount+1))==NULL) {
    fprintf(stderr,"inhale: malloc of %ld-byte buffer failed for file %s:%s\n",
            bytecount+1,pathname,strerror(errno));
    close(fd);
    return NULL;
  }

  /* read file into the buffer */
  bytes_read=0;
  while (bytes_read < bytecount) {
    i = read(fd,&buffer[bytes_read],(size_t)bytecount-bytes_read);
    if (i==-1) {
      fprintf(stderr,"inhale: read failed on file %s:%s\n",
              pathname,strerror(errno));
      free(buffer);
      close(fd);
      return NULL;
    }
    bytes_read += i;
  }

  /* clean up and return buffer */
  close(fd);
  buffer[bytecount] = '\0';

#ifdef __ORCAC__
  /* convert CR to LF */
  {
    char *p;

    for (p=buffer; *p ; p++) {
      if (*p == 0x0D) *p = 0x0A;
    }
  }
#endif  

  return buffer;
}


/*
 * extract_info -- take a string buffer containing the describe information
 *                 and return a malloc'd descEntry structure containing
 *                 pointers into the buffer of the various parts.  Also
 *                 modifies the buffer so that there is a '\0' character
 *                 between the parts.
 */

descEntry *extract_info(char *source) {

  char *p;
  descEntry *entry;

  if ((entry = malloc(sizeof(descEntry))) == NULL) {
    perror("add_entry: couldn't allocate new entry");
    exit(1);
  }

  /* extract out name */
  if (((entry->name = strstr(source,NAME_SHORT))==NULL) ||
      ((p = strchr(source,'\n'))==NULL)) {
    fprintf(stderr,"bad or missing describe field: \"%s\"\n"
            "describe entry is:\n%s\n",NAME,source);
    free(entry);
    return NULL;
  }


  /* extract out data */
  entry->data = p+1;

  /* terminate the name, dropping trailing space */
  do { --p; } while (isspace(*p));
  *(p+1) = '\0';

  return entry;
}


/*
 * add_entry -- add entry to the descTable, even if it already exists.
 */

void add_entry(descEntry *entry, int initial_buffer) {
  descEntry **e, ***array;
  int *slotsAlloced, *slotsUsed;
  
  if (initial_buffer) {
    array = &entryArray1;
    slotsAlloced = &array1SlotsAlloced;
    slotsUsed = &array1SlotsUsed;
  } else {
    array = &entryArray2;
    slotsAlloced = &array2SlotsAlloced;
    slotsUsed = &array2SlotsUsed;
  }

  /* grow array if necessary */
  if (*slotsAlloced == *slotsUsed) {
    *slotsAlloced += SLOTS_QUANTUM;
    if (*array) {
      e = realloc(*array,(*slotsAlloced) * sizeof(descEntry *));
    } else {
      e = malloc((*slotsAlloced) * sizeof(descEntry *));
    }
    if (e == NULL) {
      perror("couldn't grow describe array");
      exit(1);
    }
    *array = e;
  }

  /* add in the entry */
  (*array)[*slotsUsed] = entry;
  (*slotsUsed)++;
  return;
}


/*
 * insert - insert all entries contained in buffer into the descTable.
 *          If initial_buffer is non-zero, then use any comments preceeding
 *          the first entry as the output file header, and any comments
 *          following the last entry as the output file trailer.  If
 *          initial_buffer is zero, then the respective comment blocks are
 *          ignored, effectively deleting them from the output.
 */

void insert(char *buffer, int initial_buffer) {
  char *p, *q;
  descEntry *entry;

  /* pull out the header (if nec) and init p */
  if (initial_buffer) header = buffer;
  p = strstr(buffer,NAME_SHORT);
  if(!p) return;   /* buffer doesn't have any describe entries! */
  *(p-1)='\0';

  /* add all but the last entry */
  while ((q=strstr(p+1,NAME_SHORT))!=NULL) {
    *(q-1)='\0';
    entry = extract_info(p);
    if (entry) add_entry(entry, initial_buffer);
    p=q;
  }

  /* extract out the trailer and add the last entry */
  if ((q = strstr(p,"\n#"))==NULL) {
    if (initial_buffer) trailer="";
  } else {
    if (initial_buffer) trailer=q+1;
    *q = '\0';
  }

  entry = extract_info(p);
  if (entry) add_entry(entry,initial_buffer);
  return;
}


/*
 * sortArray - do a heapsort on <array> consisting of <slotsUsed> elements.
 *             The sort is based on the field array[i]->name, sorted
 *             lexicographically ignoring case.
 */

void sortArray(descEntry **array, int slotsUsed) {

  int l, j, ir, i;
  descEntry *rra;

  if (slotsUsed==1) return; /* no need to sort one element */
  --array;                  /* fudge since the algorithm was designed */
                            /* for a unit-indexing */
  
  l = (slotsUsed>>1) + 1;
  ir = slotsUsed;
  
  /* 
   * The index l will be decremented from its initial value down to 0 during
   * the heap creation phase.  Once it reaches 0, the index ir will be
   * decremented from its initial value down to 0 during the heap selection
   * phase.
   */
  for (;;) {
    if (l > 1)              /* still in creation phase */
      rra = array[--l];
    else {                  /* in selection phase */
      rra= array[ir];       /* clear a space at the end of array */
      array[ir] = array[1]; /* retire the top of the heap into it */
      if (--ir == 1) {      /* done with the last promotion */
        array[1] = rra;
        return;
      }
    }
    i = l;           /* set up to sift down element rra to its proper place */
    j = l << 1;
    while (j<=ir) {
      if (j<ir && (my_stricmp(array[j]->name,array[j+1]->name)<0)) ++j;
      if (my_stricmp(rra->name,array[j]->name)<0) {         /* demote rra */
        array[i] = array[j];
        i = j;
        j += i;
      } else j = ir + 1;    /* this is rra's level; set j to terminate */
    }                       /* the sift-down */
    array[i] = rra;
  }
}

/*
 * int my_stricmp (const char *cs, const char *ct);
 *
 * Compare the two strings cs and ct case-insensitive. Return
 * <0 if cs<ct, 0 if cs == ct, >0 if cs>ct.
 *
 */

int my_stricmp (const char *cs, const char *ct)
{
  char a, b;

  while ((a = tolower(*cs)) && (b = tolower(*ct))) {
    if (a < b) return -1;
    if (a > b) return 1;
    cs++; ct++;
  }
  if (*cs == *ct) return 0;
  else if (*cs) return -1;
  else return 1;
}

/*
 * ns_strcmp (no-space string compare) -- compare two strings, ignoring
 *         a leading NAME_SHORT and whitespace, and ignoring trailing
 *         whitespace.
 *
 *         Returns zero if strings are equal, -1 if a<b, 1 if a>b.
 *         The following are therefore equal:
 *           "Name:   test  "
 *           "Name:  test     "
 *         The following are inequal:
 *           "Name: one"
 *           "Name: One"
 */

int ns_strcmp (char *a, char *b) {
  char *p;
  size_t len;
  
  /* strip NAME_SHORT and leading space */
  len = strlen(NAME_SHORT);
  a+=len;
  b+=len;
  while (isspace(*a)) a++;
  while (isspace(*b)) b++;

  /* strip trailing space */
  p = a + strlen(a);
  do {
    --p;
  } while (isspace(*p));
  *(p+1) = '\0';

  p = b + strlen(b);
  do {
    --p;
  } while (isspace(*p));
  *(p+1) = '\0';

  /* do the string comparison */
  while (*a && *b) {
    if (*a < *b) return -1;
    if (*a > *b) return 1;
    a++; b++;
  }
  if (*a == *b) return 0;
  else if (*a) return -1;
  else return 1;
}


void version (char *progName) {
  fprintf(stderr,
          "%s version %s Copyright 1995 Devin Reade\n"
          "Freeware.  See the manual page for copying restrictions.\n",
          progName,versionStr);
  return;
}


/*
 * Usage -- print usage info and exit
 */


void usage(char *progName) {

  if (!Vflag || errflag) {
     fprintf(stderr,
             "%s -- describe(1) source update utility\n"
             "Usage:  %s [-hV] sourcefile patchfile1 [patchfile2 ...]\n"
             "\t-h\tshow usage information\n"
             "\t-V\tshow version information\n\n",
             progName,progName);
  }
  version(progName);
  exit(1);
}

/*
 * need I say it?
 */

int main(int argc, char **argv) {
  char *buffer;
  int i, j;
  FILE *outfp, *rejfp;
  int c;
  char *outputfile=NULL;
  int compare;

#ifdef STACK_CHECK
  begin_stack_check();
#endif

  /* initialize */
  errflag=0;
  oflag=0;

  /* parse command line */
  while ((c=getopt(argc,argv,"ho:V"))!=EOF) {
    switch (c) {
    case 'o':
      outputfile = optarg;
      oflag++;
      break;

    case 'V':
           Vflag++;
      break;

    case 'h':
    default:
      errflag++;
    }
  }

  /* error and exit if necessary */
  if (errflag || (argc-optind<2)) usage(basename(argv[0]));

  /* show version info */
  if (Vflag) version(basename(argv[0]));

  /* open output (if nec) and reject file */
  if (oflag) {
    if ((outfp = fopen(outputfile,"w+"))==NULL) {
      perror("main: couldn't open output file");
      exit(1);
    }
  } else {
    outfp = stdout;
  }
  if ((rejfp = fopen(REJECT_FILE,"w+"))==NULL) {
    perror("main: couldn't open rejects file");
    exit(1);
  }

  /* read in original describe source file */
  buffer = inhale(argv[optind]);
  insert(buffer,1);

  /* insert describe patch files */
  for (optind++; optind<argc; optind++) {
    buffer = inhale(argv[optind]);           
    insert(buffer,0);
  }

  /* sort the two arrays */
  sortArray(entryArray1,array1SlotsUsed);
  sortArray(entryArray2,array2SlotsUsed);
  
  /*
   * merge the two arrays, printing out the result 
   */
  i=0; j=0;

  /* print the header */
  fprintf(outfp,"%s\n",header);

  /* first stage; merge while we have two arrays */
  while ((i<array1SlotsUsed) && (j<array2SlotsUsed)) {
    compare = ns_strcmp (entryArray1[i]->name, entryArray2[j]->name);
    if (compare < 0) {
      fprintf(outfp,"%s\n%s\n",entryArray1[i]->name,entryArray1[i]->data);
      i++;
    } else if (compare > 0) {
      fprintf(outfp,"%s\n%s\n",entryArray2[j]->name,entryArray2[j]->data);
      j++;
    } else {
      fprintf(rejfp,"%s\n%s\n",entryArray1[i]->name,entryArray1[i]->data);
      fprintf(outfp,"%s\n%s\n",entryArray2[j]->name,entryArray2[j]->data);
      i++; j++;
    }
  }

  /* second stage; print out remaining list */
  while (i<array1SlotsUsed) {
    fprintf(outfp,"%s\n%s\n",entryArray1[i]->name,entryArray1[i]->data);
    i++;
  }
  while (j<array2SlotsUsed) {
    fprintf(outfp,"%s\n%s\n",entryArray2[j]->name,entryArray2[j]->data);
    j++;
  }

  /* print the trailer */
  fprintf(outfp,"%s",trailer);

  /* close the files and exit */
  fclose(rejfp);
  if (oflag) fclose(outfp);

#ifdef STACK_CHECK
  fprintf(stderr,"stack usage: %d bytes\n",end_stack_check());
#endif

  return 0;
}
