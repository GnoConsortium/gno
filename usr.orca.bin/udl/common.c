/*
 * udl - Convert EOL formats freely between MS-DOS (CR/LF), Unix/Amiga (LF),
 *       and Apple (CR).
 *
 * Routines common to both the Unix and Apple IIgs versions.
 *
 * $Id: common.c,v 1.7 1995/02/13 19:47:27 gdr Exp $
 *
 * Copyright (c) 1993-1995 Soenke Behrens, Devin Reade
 */

#ifdef GNO
#pragma noroot
#endif

#include "common.h"
extern char *strdup(const char *);

/*
 * convert_gs() ... convert files to use CR as EOL
 *
 * Inputs:
 *      FILE *infile    File to read from
 *      FILE *outfile   File to write to
 *
 * Outputs:
 *      None
 */

void convert_gs(FILE *infile, FILE *outfile) {
  unsigned char a;
  unsigned char *in_bufpos;
  unsigned char *out_bufpos;
  unsigned char *in_bufend;
  unsigned char *out_bufend;
  size_t file_remain;
  
  in_bufpos = in_buffer;
  out_bufpos = out_buffer;
  
  (void) fseek(infile,0L,SEEK_END);
  file_remain = ftell(infile);
  rewind(infile);
  
  in_bufend = in_buffer + my_fread(infile,BUFFERSIZE);
  out_bufend = out_buffer + BUFFERSIZE;
  
  while (file_remain != 0) {
    a = *in_bufpos;
    in_bufpos++;
    
    if (in_bufpos >= in_bufend) {
      file_remain -= in_bufend - in_buffer;
      in_bufend = in_buffer + my_fread(infile,BUFFERSIZE);
      in_bufpos = in_buffer;
    }
    /* a = fgetc (infile); */

    if(a == '\n') {
      *out_bufpos = '\r';
      out_bufpos++;
      if (out_bufpos == out_bufend) {
        my_fwrite(out_buffer,outfile,BUFFERSIZE);
        out_bufpos = out_buffer;
      }
      /* fputc('\r',outfile); */
    } else if(a == '\r') {
      *out_bufpos = '\r';
      out_bufpos++;
      if (out_bufpos == out_bufend) {
        my_fwrite(out_buffer,outfile,BUFFERSIZE);
        out_bufpos = out_buffer;
      }
      /* fputc('\r',outfile); */
      
      if (*in_bufpos == '\n' && file_remain != 0) {
        in_bufpos++;
        
        if (in_bufpos >= in_bufend) {
          file_remain -= in_bufend - in_buffer;
          in_bufend = in_buffer + my_fread(infile, BUFFERSIZE);
          in_bufpos = in_buffer;
        }
      }
      /* if ((a = fgetc (infile)) != '\n')
         ungetc (a,infile); */
    } else {
      *out_bufpos = a;
      out_bufpos++;
      if (out_bufpos == out_bufend) {
        my_fwrite(out_buffer,outfile,BUFFERSIZE);
        out_bufpos = out_buffer;
      }
      /* fputc(a,outfile); */
    }
  }
  /* Check for remainder in output buffer */
  if (out_bufpos != out_buffer)
    my_fwrite(out_buffer,outfile,out_bufpos - out_buffer);
}

/*
 * convert_messy() ... convert files to use CR/LF as EOL
 *
 * Inputs:
 *      FILE *infile    File to read from
 *      FILE *outfile   File to write to
 *
 * Outputs:
 *      None
 */

void convert_messy (FILE *infile, FILE *outfile) {
  unsigned char a;
  unsigned char *in_bufpos;
  unsigned char *out_bufpos;
  unsigned char *in_bufend;
  unsigned char *out_bufend;
  size_t file_remain;
  
  in_bufpos = in_buffer;
  out_bufpos = out_buffer;
  
  (void) fseek(infile,0L,SEEK_END);
  file_remain = ftell(infile);
  rewind(infile);
  
  in_bufend = in_buffer + my_fread(infile, BUFFERSIZE);
  out_bufend = out_buffer + BUFFERSIZE;
  
  while (file_remain != 0) {
    a = *in_bufpos;
    in_bufpos++;
    
    if (in_bufpos >= in_bufend) {
      file_remain -= in_bufend - in_buffer;
      in_bufend = in_buffer + my_fread(infile, BUFFERSIZE);
      in_bufpos = in_buffer;
    }
    /* a = fgetc (infile); */
    
    if(a == '\n') {
      *out_bufpos = '\r';
      out_bufpos++;
      if (out_bufpos == out_bufend) {
        my_fwrite(out_buffer,outfile,BUFFERSIZE);
        out_bufpos = out_buffer;
      }
      /* fputc('\r',outfile); */
      
      *out_bufpos = '\n';
      out_bufpos++;
      if (out_bufpos == out_bufend) {
        my_fwrite(out_buffer,outfile,BUFFERSIZE);
        out_bufpos = out_buffer;
      }
      /* fputc('\n',outfile); */
    } else if(a == '\r') {
      *out_bufpos = '\r';
      out_bufpos++;
      if (out_bufpos == out_bufend) {
        my_fwrite(out_buffer,outfile,BUFFERSIZE);
        out_bufpos = out_buffer;
      }
      /* fputc('\r',outfile); */

      *out_bufpos = '\n';
      out_bufpos++;
      if (out_bufpos == out_bufend) {
        my_fwrite(out_buffer,outfile,BUFFERSIZE);
        out_bufpos = out_buffer;
      }
      /* fputc('\n',outfile); */

      if (*in_bufpos == '\n' && file_remain != 0) {
        in_bufpos++;

        if (in_bufpos >= in_bufend) {
          file_remain -= in_bufend - in_buffer;
          in_bufend = in_buffer + my_fread(infile, BUFFERSIZE);
          in_bufpos = in_buffer;
        }
      }
      /* if ((a = fgetc (infile)) != '\n')
         ungetc (a,infile); */
    } else {
      *out_bufpos = a;
      out_bufpos++;
      if (out_bufpos == out_bufend) {
        my_fwrite(out_buffer,outfile,BUFFERSIZE);
        out_bufpos = out_buffer;
      }
      /* fputc(a,outfile); */
    }
  }
  /* Check for remained in output buffer */
  if (out_bufpos != out_buffer)
    my_fwrite(out_buffer,outfile,out_bufpos - out_buffer);
}

/*
 * convert_tunix() ... convert files to use LF as EOL
 *
 * Inputs:
 *      FILE *infile    File to read from
 *      FILE *outfile   File to write to
 *
 * Outputs:
 *      None
 */

void convert_tunix (FILE *infile, FILE *outfile) {
  unsigned char a;
  unsigned char *in_bufpos;
  unsigned char *out_bufpos;
  unsigned char *in_bufend;
  unsigned char *out_bufend;
  size_t file_remain;
  
  in_bufpos = in_buffer;
  out_bufpos = out_buffer;
  
  (void) fseek(infile,0L,SEEK_END);
  file_remain = ftell(infile);
  rewind(infile);
  
  in_bufend = in_buffer + my_fread(infile, BUFFERSIZE);
  out_bufend = out_buffer + BUFFERSIZE;
  
  while (file_remain != 0) {
    a = *in_bufpos;
    in_bufpos++;
    
    if (in_bufpos >= in_bufend) {
      file_remain -= in_bufend - in_buffer;
      in_bufend = in_buffer + my_fread(infile, BUFFERSIZE);
      in_bufpos = in_buffer;
    }
    /* a = fgetc (infile); */
    
    if(a == '\r') {
      *out_bufpos = '\n';
      out_bufpos++;
      if (out_bufpos == out_bufend) {
        my_fwrite(out_buffer,outfile,BUFFERSIZE);
        out_bufpos = out_buffer;
      }
      /* fputc('\n',outfile); */
      
      if (*in_bufpos == '\n' && file_remain != 0) {
        in_bufpos++;

        if (in_bufpos >= in_bufend) {
          file_remain -= in_bufend - in_buffer;
          in_bufend = in_buffer + my_fread(infile, BUFFERSIZE);
          in_bufpos = in_buffer;
        }
      }
      /* if ((a = fgetc (infile)) != '\n')
         ungetc (a,infile); */
    } else {
      *out_bufpos = a;
      out_bufpos++;
      if (out_bufpos == out_bufend) {
        my_fwrite(out_buffer,outfile,BUFFERSIZE);
        out_bufpos = out_buffer;
      }
      /* fputc(a,outfile); */
    }
  }
  /* Check for remainder in output buffer */
  if (out_bufpos != out_buffer)
    my_fwrite(out_buffer,outfile,out_bufpos - out_buffer);
}

/*
 * convert_fast_gs() ... convert files to use CR as EOL
 * Do not care about differing EOL chars in the same file,
 * do not allow '\0' bytes, and replace in-vitro if possible.
 *
 * Inputs:
 *      FILE *infile    File to read from
 *      FILE *outfile   File to write to
 *
 * Outputs:
 *      int     FALSE if no conversion took place, TRUE otherwise
 */

int convert_fast_gs(FILE *infile, FILE *outfile) {
  unsigned char a;
  unsigned char *in_bufpos;
  unsigned char *out_bufpos;
  unsigned char *in_bufend;
  unsigned char *out_bufend;
  size_t file_remain;
  enum file_format infile_type;
  
  in_bufpos = in_buffer;
  out_bufpos = out_buffer;
  
  (void) fseek(infile,0L,SEEK_END);
  file_remain = ftell(infile);
  rewind(infile);
  
  in_bufend = in_buffer + my_fread(infile,BUFFERSIZE);
  out_bufend = out_buffer + BUFFERSIZE;
  *in_bufend = '\0';
  
  infile_type = get_file_format (in_buffer);
  
  switch (infile_type) {
  case apple:
    if (verbose)
      printf("%s: %s is already in Apple format, skipping.\n",
             program_name,current_file);
    return (FALSE);
    break;

  case tunix:
    /* Replace "in-vitro", so out_buffer isn't used */
    while (file_remain != 0) {
      a = *in_bufpos;
      if (a == '\n')
        *in_bufpos++ = '\r';
      else if (a == '\0') { /* End of buffer reached */
        
        /* Write changed buffer out */
        my_fwrite(in_buffer,outfile,in_bufend - in_buffer);
        
        /* And reload it */
        file_remain -= in_bufend - in_buffer;
        in_bufend = in_buffer + my_fread(infile, BUFFERSIZE);
        *in_bufend = '\0';
        in_bufpos = in_buffer;
      } else in_bufpos++;
    }
    return (TRUE);
    break;
    
  case dos:
    /* This I couldn't speed up, so use the existing thing */
    convert_gs (infile, outfile);
    return (TRUE);
    break;

  case binary:
    return (FALSE);
    break;
  
  default:
    fprintf(stderr,"%s: Fatal internal error\n",program_name);
    exit (EXIT_FAILURE);
    break;
  } /* switch */
}

/*
 * convert_fast_messy() ... convert files to use CR/LF as EOL
 * Just check if it's already in DOS format.
 *
 * Inputs:
 *      FILE *infile    File to read from
 *      FILE *outfile   File to write to
 *
 * Outputs:
 *      int     FALSE if no conversion took place, TRUE otherwise
 */

int convert_fast_messy (FILE *infile, FILE *outfile) {
  unsigned char *in_bufpos;
  unsigned char *out_bufpos;
  unsigned char *in_bufend;
  unsigned char *out_bufend;
  size_t file_remain;
  enum file_format infile_type;
  
  in_bufpos = in_buffer;
  out_bufpos = out_buffer;
  
  (void) fseek(infile,0L,SEEK_END);
  file_remain = ftell(infile);
  rewind(infile);
  
  in_bufend = in_buffer + my_fread(infile, BUFFERSIZE);
  out_bufend = out_buffer + BUFFERSIZE;
  *in_bufend = '\0';
  
  infile_type = get_file_format (in_buffer);
  
  switch (infile_type) {
  case dos:
    if (verbose)
      printf("%s: %s is already in MS-DOS format, skipping.\n",
             program_name,current_file);
    return (FALSE);
    break;

  case tunix: /* drop through */
  case apple:
    /* Wasn't able to speed this up, call old routine */
    convert_messy (infile, outfile);
    return (TRUE);
    break;

  case binary:
    return (FALSE);
    break;

  default:
    fprintf(stderr,"%s: Fatal internal error\n",program_name);
    exit (EXIT_FAILURE);
    break;
  } /* switch */
}

/*
 * convert_fast_tunix() ... convert files to use LF as EOL
 * Do not care about differing EOL chars in the same file,
 * do not allow '\0' bytes, and replace in-vitro if possible.
 *
 * Inputs:
 *      FILE *infile    File to read from
 *      FILE *outfile   File to write to
 *
 * Outputs:
 *      int     FALSE if no conversion took place, TRUE otherwise
 */

int convert_fast_tunix (FILE *infile, FILE *outfile) {
  unsigned char a;
  unsigned char *in_bufpos;
  unsigned char *out_bufpos;
  unsigned char *in_bufend;
  unsigned char *out_bufend;
  size_t file_remain;
  enum file_format infile_type;
  
  in_bufpos = in_buffer;
  out_bufpos = out_buffer;
  
  (void) fseek(infile,0L,SEEK_END);
  file_remain = ftell(infile);
  rewind(infile);
  
  in_bufend = in_buffer + my_fread(infile, BUFFERSIZE);
  out_bufend = out_buffer + BUFFERSIZE;
  *in_bufend = '\0';
  
  infile_type = get_file_format (in_buffer);
  
  switch (infile_type) {
  case tunix:
    if (verbose)
      printf("%s: %s is already in Unix format, skipping.\n",
             program_name,current_file);
    return (FALSE);
    break;

  case apple:
    /* Replace "in-vitro", so out_buffer isn't used */
    while (file_remain != 0) {
      a = *in_bufpos;
      if (a == '\r')
        *in_bufpos++ = '\n';
      else if (a == '\0'){       /* End of buffer reached */

        /* Write changed buffer out */
        my_fwrite(in_buffer,outfile,in_bufend - in_buffer);

        /* And reload */
        file_remain -= in_bufend - in_buffer;
        in_bufend = in_buffer + my_fread(infile, BUFFERSIZE);
        *in_bufend = '\0';
        in_bufpos = in_buffer;
      } else in_bufpos++;
    }
    return (TRUE);
    break;
    
  case dos:
    /* Couldn't speed it up, so use old routine */
    convert_tunix (infile, outfile);
    return (TRUE);
    break;
    
  case binary:
    return (FALSE);
    break;

  default:
    fprintf(stderr,"%s: Fatal internal error\n", program_name);
    exit (EXIT_FAILURE);
    break;
  } /* switch */
}

/*
 * get_file_format() ... look at a buffer and find out what the EOL
 * character is. If no EOL character is found, print an error message
 * and exit.
 *
 * Inputs:
 *      unsigned char *buffer   Buffer to search through, terminated
 *                              by '\0'.
 *
 * Output:
 *      enum file_format        tunix, dos, apple, or binary
 */

enum file_format get_file_format (unsigned char *buffer) {
  unsigned char c;
  enum file_format result = 0;
  
  while ((c = *buffer++) != '\0') {
    if (c == '\n') {
      result = tunix;
      break;
    } else if (c == '\r') {
      if (*buffer == '\n')
        result = dos;
      else
        result = apple;
      break;
    }
  }
        
  if (result == 0) {
    if (verbose) 
      printf("%s: No EOL found on the first %d bytes "
             "of %s. Might be a binary file. File skipped\n",
             program_name,BUFFERSIZE, current_file);
    result = binary;
  }

  return (result);
}

/*
 * tryopen() ... try to open a file, exit if unsuccesful
 *
 * Inputs:
 *      char *name      Name of file to be opened
 *      char *mode      Mode string for fopen() call
 *
 * Output:
 *      FILE *          File identifier of successful fopen()
 */

FILE *tryopen (char *name, char *mode) {
  FILE *tmp;

  if ((tmp = fopen(name,mode)) == NULL) {
    fprintf(stderr,"%s: Unable to open file %s\n",program_name, name);
    exit (EXIT_FAILURE);
  } else
    return (tmp);
}

/*
 * my_fread() ... read data into global buffer and exit if I fail
 *
 * Inputs:
 *      FILE *infile    File to read from
 *      int howmuch     Number of bytes to read
 *
 * Output:
 *      int     Number of bytes actually read
 */

int my_fread(FILE *infile, int howmuch) {
  int result;

  result = fread(in_buffer, 1, howmuch, infile);
  if (ferror(infile)) {
    fprintf(stderr,"%s: Error while reading data\n",program_name);
    exit (EXIT_FAILURE);
  }

  return (result);
}

/*
 * my_fwrite() ... write data from global buffer to file
 *
 * Inputs:
 *      unsigned char *buffer   Buffer to write out
 *      FILE *outfile   File to write to
 *      int howmuch     Number of bytes to write
 *
 * Output:
 *      None
 */

void my_fwrite (unsigned char *buffer, FILE *outfile, int howmuch) {
  fwrite(buffer, 1, howmuch, outfile);
  if (ferror(outfile)) {
    fprintf(stderr,"%s: Error while writing data\n",program_name);
    exit (EXIT_FAILURE);
  }
  
  return;
}

/*
 * cleanup() ... called in case of an exit(). Frees memory I allocated.
 *
 * Inputs:
 *      None
 *
 * Output:
 *      None
 */

void cleanup (void) {
  char **p;

  free (program_name);
  free (current_file);
  free (in_buffer);
  free (out_buffer);
  free (tempfile);
  if (pathList) {
    p = pathList;
    while(*p) free(*p++);
    free (pathList);
  }
  
  if (tempfile)
    remove (tempfile);
}

/*
 * usage() ... print out a usage string gotten from udluse.c
 *
 * Inputs:
 *      None
 *
 * Outputs:
 *      None
 */

void usage (void) {
  extern char use1[]; /* from udluse.c */
#ifdef GNO
  extern char use2[];
#endif

  fprintf(stderr,"%s",use1);
#ifdef GNO
  if(!needsgno())
    fprintf(stderr,"%s",use2);
#endif
  
  return;
}

/*
 * build_file_list()  build the list of files to process
 *
 * Precondition:
 *      file     is the file name to be added to the pathList
 *      recurse  if non-zero, directories will be recursed
 *
 * Postcondition:
 *      pathList will be a NULL-terminated array of strings.  Each
 *      string is a partial pathname (relative to rootdir) of a file
 *      to convert.
 *
 * Note:  This is a recursive routine that uses up (3 * sizeof(char *))
 *        bytes of stack with each level of subdirectories.
 */


void build_file_list(char *file, short recurse) {
  char *thisdir;
  DIR *dir;
  struct dirent *entry;

  /* check for stack overflow */
  recursionDepth++;
#ifdef OVERFLOW_CHECK
  if ((recursionDepth * BYTES_PER_DEPTH + BASESIZE) > STACKSIZE) {
    fprintf(stderr,"%s:  Exceeded permitted nesting depth (%d levels)\n"
            "Aborted.\n",program_name,recursionDepth);
    exit(EXIT_FAILURE);
  }
#endif

  if (stat(file,&tstat)!=0) {
    fprintf(stderr,"%s:  Couldn't stat %s.  File skipped\n",program_name,file);
    --recursionDepth;
    return;
  }

  if (recurse && S_ISDIR(tstat.st_mode)) {
    char tstr[2];
    
    /*
     * It is a directory.  recurse through it.
     */

    /* save our state */
    tstr[0] = dirbrk;
    tstr[1] = '\0';
    if (*currentDirectory) {
      thisdir = strdup(currentDirectory);
    } else {
      thisdir = malloc(1);
      if (thisdir != NULL) *thisdir='\0';
    }
    if (thisdir == NULL) {
      perror("Couldn't duplicate current directory");
      exit (EXIT_FAILURE);
    }

    if (*currentDirectory) strcat(currentDirectory,tstr);
    strcat(currentDirectory,file);
    if (currentDirectory[strlen(currentDirectory)-1] == dirbrk)
      currentDirectory[strlen(currentDirectory)-1] = '\0';

    /* recurse */
    if ((dir = opendir(file)) == NULL) {
      fprintf(stderr,"%s: Couldn't open %s.  Directory skipped.\n",
              program_name,currentDirectory);
    } else {
      if (chdir(file) !=0) {
        fprintf(stderr,"couldn't cd to %s\n",currentDirectory);
        exit (EXIT_FAILURE);
      }

#ifdef READDIR_RETURNS_DOT
      entry = readdir(dir);    /* for "."  */
      entry = readdir(dir);    /* for ".." */               
#endif                             

      while ((entry = readdir(dir))!=NULL) {
        /* ignore hidden files */
#ifdef BROKEN_DIRENT_STRUCT
        if (*(entry->d_name)!='.') build_file_list((entry->d_name)-2,1);
#else
        if (*(entry->d_name)!='.') build_file_list(entry->d_name,1);
#endif
      }

      if (*thisdir) {
        if ((chdir(rootdir)!=0) || (chdir(thisdir)!=0)) {
          fprintf(stderr,"couldn't cd to %s\n",thisdir);
          exit (EXIT_FAILURE);
        }
      } else {
        if (chdir(rootdir)!=0) {
          fprintf(stderr,"couldn't cd to calling directory\n");
          exit (EXIT_FAILURE);
        }
      }
      
    }
    
    /* restore our state */
    strcpy(currentDirectory,thisdir);
    free(thisdir);
    
  } else if (S_ISREG(tstat.st_mode)) {

    /* It is a normal file.  Add it to the pathList */
    add_to_pathList(currentDirectory, file);
  }

  --recursionDepth;
  return;
}

void add_to_pathList(char *thisdir, char *file) {
  char **p;
  
  /* expand the pathList if necessary */
  if (pathSlotsUsed >= pathSlots) {
    pathSlots += PATHLIST_QUANTUM;
#if BROKEN_REALLOC
    if ((pathList==NULL) &&
        ((pathList = malloc((pathSlots+1) * sizeof(char *)))==NULL)) {
      fprintf(stderr,"%s: Couldn't expand pathList\n",program_name);
      exit (EXIT_FAILURE);
    } else {
      if ((p = realloc(pathList, (pathSlots+1) * sizeof(char *)))==NULL) {
        fprintf(stderr,"%s: Couldn't expand pathList\n",program_name);
        exit (EXIT_FAILURE);
      }
      pathList = p;
    }
#else
    if ((p = realloc(pathList,(pathSlots+1) * sizeof(char *)))==NULL) {
      fprintf(stderr,"%s: Couldn't expand pathList\n",program_name);
      exit (EXIT_FAILURE);
    } else pathList = p;
#endif
  }

  /* add in the current directory and filename to the pathList */
  pathList[pathSlotsUsed] = malloc(strlen(thisdir)+strlen(file)+2);
  if (pathList[pathSlotsUsed] == NULL) {
    fprintf(stderr,"%s: Couldn't duplicate filename %s%c%s\n",program_name,
            thisdir,dirbrk,file);
    exit (EXIT_FAILURE);
  }
  if (*thisdir) {
    sprintf(pathList[pathSlotsUsed],"%s%c%s",thisdir,dirbrk,file);
  } else {
    strcpy(pathList[pathSlotsUsed],file);
  }
  pathSlotsUsed++;
  pathList[pathSlotsUsed] = NULL;
  return;
}

/* mktemp()  construct a unique file name
 *
 * Inputs:
 *   base  Template to construct the name upon. It should
 *      be in the format "nameXXXXXX" where all "X" are replaced
 *      in such a way that the resulting name is unique. There
 *      should be at least one, at most 15 "X" in the base name.
 *      base may contain a full or partial path.
 *
 * Outputs:
 *   mktemp() returns a pointer to a dynamically allocated string
 *   containing a unique file name.
 *
 */

char *mktemp(const char *base)
{
    static char id[16] = "AAAAAAAAAAAAAAA";
    char *p1,*p2,*st;

    if ((st = malloc(strlen(base) + 1)) == NULL)
    {
      fprintf(stderr,"%s: memory allocation failure\n", program_name);
      exit (EXIT_FAILURE);
    }
    st = strcpy(st,base);
    
    if (*st == '\0')
    {
    	free (st);
    	if ((st = strdup("TXXXXXXX")) == NULL)
    	{
          fprintf(stderr,"%s: memory allocation failure\n", program_name);
          exit (EXIT_FAILURE);
    	}
    }

    /* Replace all "X" with part of ID string */
    for(p1 = st + strlen(st) - 1,p2 = &id[14];
        p1 >= st && p2 >= id && *p1 == 'X';
        p1--,p2--)
            *p1 = *p2;

    /* Update ID string to "count" one further */
    for(p1 = &id[14];p1 >= id;)
      if(*p1 == 'Z')
      {
        *p1 = 'A';
        p1--;
      } else {
        *p1 += 1;
        break;
      }

    /* Make sure the file name does not already exist */
#ifdef GNO
    if (needsgno() == TRUE) {
#endif
      if (stat(st,&tstat) == 0)
      {
    	free (st);
    	st = mktemp (base);
      }
#ifdef GNO
    } else { /* ORCA/Shell doesn't like stat one little bit */
      FILE *fp;
      if ((fp = fopen(st,"r")) != NULL)
      {
      	fclose(fp);
      	free (st);
      	st = mktemp (base);
      } else if ((fp = fopen(st,"a")) == NULL) {
      	free(st);
      	st = mktemp (base);
      } else {
      	fclose(fp);
      }
    }
#endif

    return st;
}

/* get_path() ... extract path from filename
 *
 * Inputs:
 *   name  A file name containing a full, partial or no path.
 *
 * Outputs:
 *   Pointer to a string in static memory containing the path
 *   to the given file, or an empty string if "name" contained
 *   no path. The string can hold MAXPATHLEN characters.
 */

char *get_path (const char *name)
{
  int i;

  strcpy(filebuffer, name);

  for (i = strlen(filebuffer) - 1; i > 0 && filebuffer[i] != dirbrk; i--)
    ; /* empty loop to find end of path in name */

  if (i != 0)
    ++i;
  filebuffer[i] = '\0';
  return filebuffer;
}

/* End Of File */
