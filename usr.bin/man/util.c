/*
 * Copyright 1995 by Devin Reade <gdr@myrias.com>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 */

segment "util______";

#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <ctype.h>
#include <sgtty.h>
#include <fcntl.h>
#include "util.h"

/*
 * getManpath -- return a malloc'd copy of the MANPATH.  If MANPATH
 *               isn't defined in the environment, then use USRMAN, then
 *               MANDIR, then if all else fails, use the macro DEFAULT_MANPATH.
 *               If an error occurs, print the cause via perror and exit.
 */

char *getManpath(void) {
   char *manpath;

   manpath = getenv("MANPATH");
   if (manpath == NULL) manpath = getenv("USRMAN");
   if (manpath == NULL) manpath = getenv("MANDIR");
   if (manpath == NULL) manpath = DEFAULT_MANPATH;

   return Xstrdup(manpath,__LINE__,__FILE__);
}

/*
 * Xstrdup - a safe strdup; it will handle error conditions and exit
 *           one occurs
 *
 *           line and file are arbitrary, but are expected to be the
 *           values of __LINE__ and __FILE__ respectively.
 */                                  

char *Xstrdup(char *oldstr, int line, char *file) {
   char *newstr;

   if ((newstr = malloc(strlen(oldstr)+1)) == NULL) {
      fprintf(stderr,"Xstrdup failed at line %d in file %s: %s\n",line,
         file,strerror(errno));
      exit(1);
   }
   strcpy(newstr,oldstr);
   return newstr;
}

/*
 * Xmalloc - a safe malloc; it will handle error conditions and exit
 *           if one occurs.
 *
 *           line and file are arbitrary, but are expected to be the
 *           values of __LINE__ and __FILE__ respectively.
 */

void *Xmalloc(size_t size, int line, char *file) {
   char *p;

   if ((p = malloc(size)) == NULL) {
      fprintf(stderr,"Xmalloc failed at line %d in file %s: %s\n",line,
         file,strerror(errno));
      exit(1);
   }
   return ((void *) p);
}


/*
 * Xrealloc - a safe realloc; it will handle error conditions and exit
 *            if one occurs.
 *
 *            line and file are arbitrary, but are expected to be the
 *            values of __LINE__ and __FILE__ respectively.
 */

void *Xrealloc(void *oldptr, size_t size, int line, char *file) {
   char *p;

   if ((p = realloc(oldptr, size)) == NULL) {
      fprintf(stderr,"Xrealloc failed at line %d in file %s: %s\n",line,
         file,strerror(errno));
      exit(1);
   }
   return ((void *) p);
}


/*
 * addToStringArray -- add a string to a NULL-terminated array of strings.
 *                     If oldArray is NULL, then a new array is created,
 *                     otherwise oldArray is expanded.
 *
 *          WARNING: | Because of the allocation scheme used, any oldArray
 *                   | passed to this routine _must_ be the return value
 *                   | of a previous call to this routine.  This does not
 *                   | imply that only one array can be expanded by this
 *                   | routine (any number may be expanded).
 *
 *                     The value of macro SLOTS_QUANTUM defined below is
 *                     the number of array slots allocated at one time.  The
 *                     size of the string array is always a multiple of this
 *                     value.
 *
 *                     Returns a pointer to an array that contains the old
 *                     strings with the new one appended.  The array is
 *                     NULL-terminated.
 */

#define SLOTS_QUANTUM 10

char **addToStringArray(char **oldArray, char *string) {

   char **result;
   int slotsAlloced, slotsUsed;

   if (oldArray == NULL) {

      /*
       *  This is a new array; do the brute force approach
       */
      
      result = Xmalloc(SLOTS_QUANTUM * sizeof(char *), __LINE__, __FILE__);
      result[0] = Xstrdup(string,__LINE__,__FILE__);
      result[1] = NULL;

   } else {

      /*
       * adding to and, if necessary, expanding an old array
       */

      /* determine slotsUsed and slotsAlloced */
      for (slotsUsed=0; oldArray[slotsUsed]; slotsUsed++);

      if (slotsUsed % SLOTS_QUANTUM == SLOTS_QUANTUM-1) { /* space for NULL */
         slotsAlloced = slotsUsed+1;
      } else {
         slotsAlloced = ((slotsUsed / SLOTS_QUANTUM) + 1) * SLOTS_QUANTUM;
      }

#ifdef DEBUG
      assert(slotsUsed < slotsAlloced);
#endif

      /* expand number of slots if necessary */
      if (slotsUsed+1 < slotsAlloced) {
         /* there are enough slots; add it to this array */
         result = oldArray;
      } else {
         /* we need more slots; expand the array */
         slotsAlloced += SLOTS_QUANTUM;
         result = Xrealloc(oldArray, slotsAlloced * sizeof(char *),
                           __LINE__, __FILE__);
      }

      /* add the string to the array */
      result[slotsUsed++] = Xstrdup(string, __LINE__, __FILE__);
      result[slotsUsed] = NULL;
   }
   return result;
}
   
/*
 * makePathArray -- parse a path list and break it into a NULL-terminated
 *                  array of paths.  The original path is left unchanged.
 *
 *                  The delimiter between paths may either be a ' ' or a ':'.
 *                  The delimiter is assumed to be a ':' if <path> contains
 *                  both '/' and ':' characters but no ' ' character,
 *                  otherwise the delimiter is assumed to be a ' '.
 */

char **makePathArray(char *path) {

   char *delim, *p, *q, **result;

   /* set the delimiter */
   if ( strchr(path,' ')==NULL &&
        strchr(path,':') &&
        strchr(path,'/')) {
      delim = ":";
   } else {
      delim = " ";
   }

   /* build the array */
   p = Xstrdup(path, __LINE__, __FILE__);
   q = strtok(p,delim);
   result = NULL;
   while (q) {
      result = addToStringArray(result, q);
      q = strtok(NULL,delim);
   }
   
   free(p);
   return result;
}


/*
 * ncstrcmp -- A case insensitive ("no-case") strcmp
 */

int ncstrcmp(char *a, char *b) {

   while (*a && *b) {
      if (*a == *b) {
         a++; b++;
         continue;
      }
      return ((int) *b - *a);
   }
   if (!*a && !*b) return 0;
   return ((int) *b - *a);
}

/*
 * ncstrncmp -- A case insensitive ("no-case") strncmp
 */

int ncstrncmp (char *a, char *b, unsigned int count) {
   unsigned int i=0;

   for (i=0; i<count; i++) {
      if (a[i] == b[i]) {
         if (!a[i]) break;
         else continue;
      }
      return ((int) b[i]-a[i]);
   }
   return 0;
}


/*
 * ncstrstr -- A case insensitive ("no-case") strstr
 *
 *             This is implemented using a convert-copy-to-single-case-
 *             then-strstr hack.
 *             It's speed could be increased by switching
 *             to a Boyer-Moore scan algorithm anytime strlen(substr) > 5,
 *             and using a straight-forward search for strlen(substr) <= 5.
 */

char *ncstrstr(char *str, char *substr) {

   char *strCopy, *substrCopy, *p;

   strCopy = Xstrdup(str,__LINE__,__FILE__);
   substrCopy = Xstrdup(substr,__LINE__,__FILE__);

   /* convert the strings */
   p = strCopy;
   while (*p) {
      *p = tolower(*p);
      p++;
   }
   p = substrCopy;
   while (*p) {
      *p = tolower(*p);
      p++;
   }

   p = strstr(strCopy,substrCopy);
   free(strCopy);
   free(substrCopy);
   return p;
}

/*
 * newerFile  -- Compares the "last modified" time of two paths.
 *               Returns:
 *                   the name of the newer file if both files exist
 *                   the second file if both exist and have the same mod time
 *                   the name of the existing file if one file doesn't exist
 *                   NULL and sets errno if neither file exists or if there
 *                      is an error.
 */

char *newerFile(char *path1, char *path2) {
   static struct stat record1, record2;
   int i,j;

   /*
    * see if both, only one, or neither files exist
    */

   i = access(path1, F_OK);
   j = access(path2, F_OK);
   if (i==-1 && j==-1) {
      errno = ENOENT;
      return NULL;
   } else if (i==-1) {
      return path2;
   } else if (j==-1) {
      return path1;
   }

   /*
    * both files exist; stat them
    */

   if (stat(path1,&record1) != 0) return NULL;
   if (stat(path2,&record2) != 0) return NULL;

   return (record1.st_mtime > record2.st_mtime) ? path1 : path2;
}


/*
 * getcharraw() - return the next character from stdin without waiting
 *                for a CR to be hit.  Returns '\0' and sets errno
 *                on an error.
 */

#define FAILED_CHAR '\0';

char getcharraw(void) {
   short oldmode;
   struct sgttyb s;
   int count;
   char c;

   /* obtain old terminal mode */
   if (gtty(STDIN_FILENO,&s) == -1) return FAILED_CHAR;
   oldmode = s.sg_flags;

   /* set terminal to CBREAK and obtain keystroke */
   s.sg_flags |= CBREAK;
   if (stty(STDIN_FILENO,&s) == -1) return FAILED_CHAR;
   count = read (STDIN_FILENO, &c, 1);

   /* reset old terminal mode */
   s.sg_flags = oldmode;
   if ((stty(STDIN_FILENO,&s) == -1) || (count == -1)) return FAILED_CHAR;
   return c;
}

/*
 * basename -- return a pointer to the base filename (all leading
 *             pathname components removed) of <path>.  <path> _must_
 *             point to a NULL-terminated string.
 */

char *basename (char *path) {
   char *p, dirsep;

   dirsep = (strchr(path,':')) ? ':' : '/';
   if ((p = strrchr(path,dirsep)) != NULL) {
      return p+1;
   } else {
      return path;
   }
}

/*
 * dirname -- return a pointer to a string consisting of the directory
 *            component of <path>.  This returns a pointer to an internal
 *            buffer, so the next call to dirname() will overwrite this
 *            buffer.  <path> must be a NULL-terminated string.
 */

char *dirname (const char *path) {
   static char buffer[FILENAME_MAX];
   char *p, dirsep;

   strcpy(buffer,path);
   
   dirsep = (strchr(buffer,':')) ? ':' : '/';
   if ((p = strrchr(buffer,dirsep)) != NULL) {
      *p = '\0';
   }
   return buffer;
}
                   
