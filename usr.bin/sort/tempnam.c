#ifdef __CCFRONT__
#include <14:pragma.h>
#endif
/*
 * #include <stdio.h>
 *
 * char *tempnam (const char *dir, const char *prefix);
 *
 * Generate a pathname for a temporary file.
 *
 * tempnam will select a directory for the temporary file by using the
 * following criteria:
 *
 *   If dir is not the NULL pointer, tempnam uses the pathname pointed to by
 *   dir as the directory,
 *
 *   otherwise, tmpdir uses the value of the TMPDIR environment variable if
 *   the variable is defined,
 *
 *   otherwise the directory defined by P_tmpdir in the stdio.h header file
 *   if that directory is writable by the caller,
 *
 *   otherwise, tempnam will use "/tmp" as a last resort.
 */

#ifdef __ORCAC__
#define __GNO__ 1
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define max(A,B) (((A)<(B))?(B):(A))

#if !defined(__GNO__)
extern char *mktemp();
extern int access();
#endif

static char  seed[4]="AAA";

#if (defined __GNO__)
static char  pbrk;
#else
# define pbrk '/';
#endif

/* BSD stdio.h doesn't define P_tmpdir, so let's do it here */
#ifndef P_tmpdir
static char *P_tmpdir = "/tmp";
#endif


static char *
cpdir(char *buf, char *str)
{
   char *p;
   char *path;

   if(str != NULL) {

#if defined(__GNO__)
     /* get the path delimiter */
     if (strchr(str,':')) pbrk = ':';
     else if (strchr(str,'/')) pbrk = '/';
     else {
       if ((path=getenv("PATH"))==NULL) pbrk = '/';
       else pbrk = (strchr(path,':')) ? ':' : '/';
     }
#endif

      (void) strcpy(buf, str);
      p = buf - 1 + strlen(buf);
      if(*p == pbrk) *p = '\0';
   }

   return(buf);
}


char *
tempnam (char *dir, char *prefix)
  /* dir    -- use this directory please (if non-NULL)   */
  /* prefix -- use this (if non-NULL) as filename prefix */
{
   register char *p, *q, *tmpdir;
   int            tl=0, dl=0, pl;

   /* create a buffer <p> that's as large as necessary */
   pl = strlen(P_tmpdir);
   if( (tmpdir = getenv("TMPDIR")) != NULL ) tl = strlen(tmpdir);
   if( dir != NULL ) dl = strlen(dir);
   if( (p = malloc((unsigned int)(max(max(dl,tl),pl)+16))) == NULL )
     return(NULL);
   *p = '\0';

#if defined (__GNO__)
   if( (dl == 0) || (access( cpdir(p, dir), W_OK) != 0) )
     if( (tl == 0) || (access( cpdir(p, tmpdir), W_OK) != 0) )
       if( access( cpdir(p, P_tmpdir),   W_OK) != 0 )
         if( access( cpdir(p, "/tmp"),  W_OK) != 0 )
           return(NULL);

#else  /* not __GNO__ */
   if( (dl == 0) || (access( cpdir(p, dir), 3) != 0) )
     if( (tl == 0) || (access( cpdir(p, tmpdir), 3) != 0) )
       if( access( cpdir(p, P_tmpdir),   3) != 0 )
         if( access( cpdir(p, "/tmp"),  3) != 0 ) 
           return(NULL);
#endif /* not __GNO__ */

   (void) strcat(p, "/");
   if(prefix)
   {
      *(p+strlen(p)+5) = '\0';
      (void)strncat(p, prefix, 5);
   }

   (void)strcat(p, seed);
   (void)strcat(p, "XXXXXX");

   q = seed;
   while(*q == 'Z') *q++ = 'A';
   ++*q;

   if(*mktemp(p) == '\0') return(NULL);
   return(p);
}
