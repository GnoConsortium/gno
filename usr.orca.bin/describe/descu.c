/*
 * descu - describe(1) update utility for maintaining describe source files
 *
 * Usage:  descu [-hV] [-o outfile] sourcefile patchfile1 [patchfile2 ...]
 *
 * Options:
 *         -h         show usage information and exit.
 *         -o file    send output to <file> rather than stdout
 *         -V         show version information
 *
 * Copyright 1995-1997 by Devin Reade for James Brookes' describe(1) utility.
 * See the included README file and man page for details.
 *
 * $Id: descu.c,v 1.7 1998/01/17 07:20:07 gdr Exp $
 */

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
#include <time.h>
#ifdef __GNO__
#include <gno/gno.h>
#endif
#include "desc.h"

#define MAX_BUFFER    65534
#define SLOTS_QUANTUM 64
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

char *
inhale (char *pathname) {
  char *buffer;
  long bytecount, bytes_read;
  ssize_t i;
  int fd;

  /* open the file */
  if ((fd = open(pathname,O_RDONLY))==-1) {
    fprintf(stderr,"Warning: open of %s failed: %s\n",
            pathname,strerror(errno));
    return NULL;
  }

  /* create the buffer */
  bytecount = lseek(fd,(off_t) 0,SEEK_END);
  if (bytecount > MAX_BUFFER) {
    fprintf(stderr,"descu internal error: cannot handle files greater "
                   "than %d bytes\n due to a compiler bug.  Sorry.\n",
                   MAX_BUFFER);
    exit(-1);
  }
  lseek(fd,(off_t) 0, SEEK_SET);
  if ((buffer = malloc(bytecount+1))==NULL) {
    fprintf(stderr,"error: malloc of %ld-byte buffer failed for file %s:%s\n",
            bytecount+1,pathname,strerror(errno));
    close(fd);
    return NULL;
  }

  /* read file into the buffer */
  bytes_read=0;
  while (bytes_read < bytecount) {
    i = read(fd,&buffer[bytes_read],(size_t)bytecount-bytes_read);
    if (i==-1) {
      fprintf(stderr,"error: read failed on file %s:%s\n",
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

descEntry *
extract_info(char *source) {

  char *p, *q, *r;
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

  /* drop trailing blank lines, except for one */
  p = r = entry->data;
  p += strlen(p);
  q = p - 1;
  while ((q >= r) && isspace(*q)) {
	*q-- = '\0';
  }
  q++;
  if (q < p) {
	*q++ = '\n';
  }
#if 0
  if (q < p) {
	*q++ = '\n';
  }
#endif
  if (q < p) {
	*q = '\0';
  }

  /* eliminate whitespace at the beginning of lines */
  p = entry->data;
  for (;;) {
	/* skip to next newline */
  	while (*p && *p != '\n') p++;
	if (*p == '\0') break;
        p++;
	while (*p == '\n') p++;
	if (!isspace(*p)) continue;

	/* move q to first non-whitespace character */
	q = p;
	while (isspace(*q)) q++;
	if (*q == '\0') break;
	
	/* shift the buffer */
	r = p;
	while (*q) {
		*r++ = *q++;
	}
	*r = '\0';
  }

  return entry;
}


/*
 * add_entry -- add entry to the descTable, even if it already exists.
 */

void
add_entry(descEntry *entry, int initial_buffer) {
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
  static char *emptyString = "";	/* we may return this, so keep it static */

  /* pull out the header (if nec) and init p */
  if (initial_buffer) header = buffer;
  p = strstr(buffer,NAME_SHORT);
  if (p == NULL) {
    return;   /* buffer doesn't have any describe entries! */
  }
  if (initial_buffer) {
    if (p == buffer) {
	    /* there is no header */
      header = NULL;
    } else {
      *(p-1)='\0';
    }
  }

  /* add all but the last entry */
  while ((q=strstr(p+1,NAME_SHORT))!=NULL) {
    *(q-1)='\0';
    entry = extract_info(p);
    if (entry) add_entry(entry, initial_buffer);
    p=q;
  }

  /* extract out the trailer and add the last entry */
  if ((q = strstr(p,"\n#"))==NULL) {
    if (initial_buffer) trailer=emptyString;
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

  if (slotsUsed <= 1) return; /* no need to sort one element */
  --array;                    /* fudge since the algorithm was designed */
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
  if (*cs == *ct) return 0; /* cs and ct of same length */
  else if (*cs) return 1; /* cs longer than ct */
  else return -1; /* cs shorter than ct */
}

/*
 * ns_stricmp (no-space string compare) -- compare two strings
 *    case-insensitive, ignoring a leading NAME_SHORT and whitespace,
 *    and ignoring trailing whitespace.
 *
 *         Returns zero if strings are equal, -1 if a<b, 1 if a>b.
 *         The following are therefore equal:
 *           "Name:   test  "
 *           "Name:  test     "
 *         The following are inequal:
 *           "Name: one"
 *           "Name: two"
 */

int ns_stricmp (char *a, char *b) {
  char *p;
  char ca, cb;
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
  while ((ca = tolower(*a)) && (cb = tolower(*b))) {
    if (ca < cb) return -1;
    if (ca > cb) return 1;
    a++; b++;
  }
  if (*a == *b) return 0; /* a and b of same length */
  else if (*a) return 1; /* a longer than b */
  else return -1; /* a shorter than b */
}


void version (char *progName) {
  fprintf(stderr,
          "%s version %s Copyright 1995-1997 Devin Reade\n"
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
             "Usage:  %s [-hV] [-o outfile] sourcefile patchfile1 [patchfile2 ...]\n"
             "\t-h\t\tshow usage information\n"
             "\t-o outfile\tsend output to <outfile> rather than stdout\n"
             "\t-V\t\tshow version information\n\n",
             progName,progName);
  }
  version(progName);
  exit(1);
}

/*
 * need I say it?
 */

int main(int argc, char **argv) {
  static char *revisionMagic = "\n# Last revision:";
  time_t t;
  char *buffer;
  int i, j;
  FILE *outfp, *rejfp, *temp = NULL;
  int c;
  char *outputfile=NULL;
  char *p;
  int compare;

#ifdef __STACK_CHECK__
  _beginStackCheck();
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

  /*
   * open output file if necessary.  If the output filename matches
   * that of the first input file, then dump stuff to a temporary file.
   */
  if (oflag) {
    if (!strcmp(outputfile, argv[optind])) {
	    if ((outfp = tmpfile()) == NULL) {
	      perror("unable to open temporary file");
        exit(1);
      }
      temp = outfp;
    } else {
      if ((outfp = fopen(outputfile,"w+")) == NULL) {
        perror("couldn't open output file");
        exit(1);
      }
    }
  } else {
    outfp = stdout;
  }

  /* open the rejects file */
  if ((rejfp = fopen(REJECT_FILE,"w+"))==NULL) {
    perror("couldn't open rejects file");
    exit(1);
  }

  /* read in original describe source file */
  if ((buffer = inhale(argv[optind])) != NULL) {
    insert(buffer,1);
  }

  /* insert describe patch files */
  for (optind++; optind<argc; optind++) {
    if ((buffer = inhale(argv[optind])) != NULL) {
      insert(buffer,0);
    }
  }

  /* sort the two arrays */
  sortArray(entryArray1,array1SlotsUsed);
  sortArray(entryArray2,array2SlotsUsed);
  
  /*
   * merge the two arrays, printing out the result 
   */
  i=0; j=0;

  /* print the header, if it exists */
  if (header != NULL) {
    if ((p = strstr(header, revisionMagic)) != NULL) {
      /* found the "Last revision" line?  Update it */
      *p = '\0';
      p += strlen(revisionMagic);
      while (*p && *p != '\n') {
	p++;
      }
      if (*p) {
	p++;
      }
      time(&t);
      fprintf(outfp,"%s%s %s%s\n", header, revisionMagic,
	      asctime(gmtime(&t)), p);
    } else {
      fprintf(outfp,"%s\n", header);
    }
  }

  /* first stage; merge while we have two arrays */
  while ((i<array1SlotsUsed) && (j<array2SlotsUsed)) {
    compare = ns_stricmp (entryArray1[i]->name, entryArray2[j]->name);
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

  /* print the trailer, if it exists */
  if (trailer != NULL) {
    fprintf(outfp,"%s",trailer);
  }

  /* close the files and exit */
  fclose(rejfp);
  if (oflag) {
    if (temp != NULL) {
      /* temp and outfp refer to the same FILE struct at this point */
#define BUFFERSIZE 4096
      char *buf;
      size_t count;

      if ((outfp = fopen(outputfile,"w+")) == NULL) {
        perror("couldn't open output file");
        exit(1);
      }
      rewind(temp);
      if ((buf = malloc(BUFFERSIZE)) == NULL) {
	      perror("couldn't allocate buffer for file copy");
        exit(1);
      }
      while ((count = fread(buf, 1, BUFFERSIZE, temp)) > 0) {
      	fwrite(buf, 1, count, outfp);
      }
      fclose(temp);
    }
    fclose(outfp);
  }

#ifdef __STACK_CHECK__
  fprintf(stderr,"stack usage: %d bytes\n", _endStackCheck());
#endif

  return 0;
}
