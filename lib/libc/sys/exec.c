/*
 * exec(2) library calls.  Written as part of lenviron by Devin Reade
 * for GNO v2.0.3.  Incorporated into libc as of GNO v2.0.6.
 *
 * $Id: exec.c,v 1.1 1997/02/28 05:12:50 gdr Exp $
 *
 * This file is formatted with tabs every 3 columns.  The remainder of
 * the comments in this section are from the lenviron v1.1.3 implementation.
 * Of course, the GNO distribution headers now reflect these changes.
 *
 *************************************************************************
 *
 * These calls will only work with the GNO kernel!  They were tested with
 * GNO v2.0.3 and later, but _might_ work with other versions.
 *
 * These have been implemented using the standard Unix declarations.  In
 * particular, the prototype for execve(2) does _not_ match that of
 * Procyon's distribution of GNO v2.0.5 and earlier.
 *
 * If you wish to make use of the execve call as provided with the GNO
 * v2.0.5 (and earlier) distribution, that call is still available as the
 * function
 *
 *    int _execve(const char *path, const char *params);
 *
 * Note that execle(2) is not currently implemented.  exect(2) probably
 * never will be so implemented.
 *
 * Note that the current version of gsh parses $PATH backwards for some
 * reason.  For consistency, execvp() and execlp() will do the same for now.
 * If gsh gets fixed or if you switch to a shell that parses $PATH front-to-
 * back, just undefine BACKWARDS within this file an recompile.  The
 * appropriate code has already been tested.
 */

#ifdef __ORCAC__
segment "libc_sys__";
#endif

#pragma debug 0
#pragma memorymodel 0

/*
 * Use bits 0, 1, 2, 6 (== decimal 71) for optimization.  In Orca/C v2.1.0,
 * bits 4 and 5 are still reported to be buggy.
 *
 * Around variadic routines, we also add in optimization bit 3 (== 79).
 */

/* pragma optimize 71 */

#include <sys/types.h>
#include <types.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <orca.h>
#include <gno/gno.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

#define PARMSGUESS 10    /* initial length of argv array in execl, execlp */

#define BACKWARDS        /* undefine this for shells that parse $PATH     */
                         /*  front to back.  gsh as of Dec 93 needs this  */
                         /*  defined.                                     */

pascal int  Kexecve(char *pathname, char *cmdline, int *errno)
   inline(0x1D03,0xE10008);


/*
 * int isRootPath(const char *name);
 *
 * Pre:  <name> is a file name
 *
 * Post: Return TRUE if the name is the full specification of a path name
 *       to a file starting at the root of the file system, otherwise
 *       return FALSE.  Note that no test for existence is carried out;
 *       this routine only judges whether or not the given name is a valid
 *       one file starting at the root of the file system.
 *
 * Caveat:  Unlike the Unix filesystem, "/" is not recognised as a complete
 *       file (or directory) name.  Therefore, it will return FALSE if the
 *       the name is one character long and isn't "*" or "@" or a digit.
 */

			/* allowable start of full file/device/prefix names */
static char* PfxBrkStr = "/:0123456789.*@";

int
isRootPath(const char *name) {
	char *p;

   p = strchr(PfxBrkStr, *name);
   if (p == NULL) return FALSE;

   switch (*p) {
   case '/':	/* FALLTHROUGH */
   case ':':
     	/* "zero" length volume/prefix names not allowed */
	   if (*(p+1) == '\0') return FALSE;
		else return TRUE;
      /* NOTREACHED */
   case '.':
     	/* "zero" length volume/prefix names not allowed. "./" and ".:" fail */
      switch (*(p+1)) {
	   case '\0':	/* FALLTHROUGH */
      case '/':	/* FALLTHROUGH */
      case ':':
			return FALSE;
      default:
	      return TRUE;
      }
      /* NOTREACHED */
   case '*':	/* FALLTHROUGH */
   case '@':
	   switch (*(p+1)) {
	   case '\0':	/* FALLTHROUGH */
      case '/':	/* FALLTHROUGH */
      case ':':
	   	return TRUE;
      default:
	   	return FALSE;
      }
     	/* NOTREACHED */
   default:
	   /* it must be starting with a digit */
		return TRUE;
   }
}


/*
 * char *buildCmd (char *const *argv);
 *
 * Pre:  argv is a pointer to an array of strings, ending with a NULL pointer
 *
 * Post: Returns a pointer to a string consisting of all of the elements of
 *       argv, delimited by single spaces.  Returns NULL if memory for the
 *       string cannot be allocated.  If argv[0] == NULL, buildCmd() will
 *       return a pointer to a zero-length string.  If it returns NULL,
 *       errno is also set to ENOMEM.
 *
 *       Any GS/OS prefix (numerical or otherwise) on argv[0] will be stripped.
 */

char *
buildCmd (char *const *argv) {

   char *comdbuf;             /* pointer to the command line buffer */
   size_t comdsize;           /* length of the command line buffer  */
   int i;
   char *s;                   /* a temporary pointer */
   char delim;                /* delimiter for pathnames (':' or '/') */

   /* allocate memory for command line */
   comdsize = 1;
   i = 0;
   while (argv[i] != NULL) {
      comdsize = comdsize + strlen(argv[i]) + 1;
      i++;
   }
   if ((comdbuf = malloc (comdsize)) == NULL) return NULL;

   /* build command line from argv */
   i = 0;
   if (argv[i] == NULL) {
      comdbuf[0] = (char) NULL;
   } else {
	   /* find delimiter */
   	delim = (strchr(argv[0],':') == NULL) ? '/' : ':';

	   /* drop leading prefix */
	   s = argv[i];
      while (*s) s++;
      while ((s>argv[i]) && (*s != delim)) s--;
      if (*s==delim) s++;                        

      strcpy(comdbuf,s);
      i++;
      while (argv[i] != NULL) {
         strcat(comdbuf, " ");
         strcat(comdbuf, argv[i]);
         i++;
      }
   } 

   return comdbuf;
}


/* int _fileExists (const char *file, const char *path)
 *
 * Pre:  file is a file name, path is a full legal directory name
 *
 * Post: Returns 1 if file is in path, returns 0 and errno set on failure.
 *       Possible errno values on return are 0, ENOMEM, and ENOENT.
 *
 * Notes:  If <path> is a zero-length string (not NULL!), then existence
 *         of <file> will be tested based on <file> being a partial pathname.
 *
 *         _fileExists() will also fail if file is either a full path name or
 *         a zero-length string.
 */

static int
_fileExists (const char *file, const char *path) {

	static char *buffer = NULL;
   static size_t buffersize = 0;
   size_t pathlen, length;
   char delim;
   char *tp;
        	
   delim = (strchr(path,':') == NULL) ? '/' : ':'; /* find delimiter */

   /* <file> is a full pathname or empty string? Fail! */
   if ((*file == '\0') || (isRootPath(file))) {
	   errno = ENOENT;
      return 0;
   }

   /* calculate length of path prefix and full pathname */
   pathlen = strlen(path);
   length = strlen(file) + pathlen + 2;

   /*
    * Allocate more mem for buffer, if necessary.  This fragment's behavior
    *	is dependant on the implementation of realloc and is not necessarily
    * portable.  It assumes that realloc() called with a NULL pointer behaves
    * the same as malloc().
    */

   if (length > buffersize) {
   	tp = (char *) realloc (buffer, length);
   	if (tp == NULL) {
         if (buffer) free (buffer);
         buffer = NULL;
         buffersize = 0;
         errno = ENOMEM;
      	return 0;
      }
      buffer = tp;
   }

   strcpy(buffer,path);                /* make copy of path,      */
   if (pathlen) {
   	if (buffer[pathlen-1] != delim) { 	/* delimiter and terminate */
	   	buffer[pathlen] = delim;
      	buffer[pathlen+1] = '\0';
   	}
   } else {
	   *buffer = '\0';
   }

   strcat(buffer,file);

   if (access(buffer, F_OK) == 0) { 		/* file found */
	   errno = 0;
      return 1;
   } /* else ... */
   errno = ENOENT;
   return 0;
}


/*                                 
 * char *buildPath (const char *filename);
 *
 * Pre:  <filename> is the name of the file which we wish to locate.
 *
 * Post: If <filename> resides within $PATH, then buildPath will return a
 *       malloc'd pointer to the full pathname of the file.
 *       If <filename> cannot be found with $PATH, buildPath the default
 *       which is to search "/bin", then "/usr/bin".  On error, NULL is
 *       returned and errno is set either to ENOENT or ENOMEM, as appropriate.
 *
 *       If <filename> is in fact a full pathname, then a pointer to a malloc'd
 *       copy of the filename is returned; in this case, no test for existence
 *       is done <filename> is considered to be a full pathname if it
 *       begins with any of '/', ':', '.', '@', '*', or a digit.
 *       If any of '/', ':', or '.' is the first character, the
 *       filename must have a length of at least two characters.
 *
 * Caveat:  There is a conditional compilation in this function;  gsh currently
 *       parses the $PATH variable backwards, so for compatibility buildPath
 *       will also do a backwards parse if BACKWARDS is defined.  Otherwise,
 *       $PATH will be parsed front-to-back.
 */

char *
buildPath (const char *filename) {
   char *pathptr;             /* pointer to the PATH shell variable    */
   char *path;                /* pointer to the current PATH token     */
   char *buffptr;             /* where we will store the full pathname */
   size_t pathlen;            /* for calculating space for malloc() */
   char delim[2];             /* delimiter for pathnames.           */
                              /*   Assumption:  if a path doesn't   */
                              /*   contain any '/' chars, the       */
                              /*   delimiter is ':'.                */
#ifdef BACKWARDS
	 static char *default_path = "/usr/bin /bin";
   char *path_copy;
#else
	 static char *default_path = "/bin /usr/bin";
#endif

   /*
    * if for some weird and wonderful reason, filename is a full pathname,
    * then just return a pointer to a copy of it.  In this case no test
    * for existence is done.
    */
   if (isRootPath(filename)) {
	   buffptr = (char *) malloc (strlen(filename) + 1);
      if (buffptr == NULL) return NULL;
      strcpy (buffptr,filename);
      return buffptr;
   }

   /* get the value of the PATH shell variable */
   pathptr = getenv ("PATH");
   if (pathptr == NULL || *pathptr == '\0') {
      /* PATH doesn't exist -- use default */
      pathptr = default_path;
   }

   /* define the pathname delimiter */
   (strstr(pathptr,"/") == NULL) ? strcpy (delim, ":") : strcpy (delim, "/");

   /*
    * search paths for filename -- backwards or forwards as appropriate
    */

#ifdef BACKWARDS
   /* make a copy of the path */
   pathlen = strlen(pathptr) + 1;
   if ((path_copy = (char *) malloc((size_t) strlen(pathptr) + 1))==NULL) {
      errno = ENOMEM;
      return NULL;
   }
   strcpy(path_copy,pathptr);
   path = path_copy + pathlen;

   /* look for the file */
   while (path>=path_copy) {
      while ((path>path_copy) && (*path != ' ')) path--;
      if (path>path_copy) {     /* not done parsing $PATH */
         if (_fileExists(filename, path+1)) {    /* found it! */
            path++;
            break;
         } else {               /* not found; terminate string and try again */
            *path = '\0';
         }
      } else {
	      /*
          * at this point, path points to either first listed directory or
          * whitespace; check it
          */
         if (isprint(*path) && _fileExists(filename, path)) {    /* found it! */
            break;
         } else {
         	free(path_copy);		/* not in $PATH; parse failed */
         	errno = ENOENT;
         	return NULL;
         }
      }
   }
#else
   path = strtok (pathptr, " ");
   while (path != NULL) {
      if (_fileExists(filename, path)) break;
      path = strtok (NULL, NULL);
   }

   /* filename not within listed paths */
   if (path == NULL) {
      errno = ENOENT;
      return NULL;
   }
#endif  /* BACKWARDS */


   /* allocate the buffer */
   pathlen = strlen(filename) + strlen(path) + 2;
   if ((buffptr = malloc (pathlen)) == NULL) {
#ifdef BACKWARDS
      free(path_copy);
#endif
      return NULL;
   }

   /* build the full pathname */
   strcpy (buffptr,path);
   strcat (buffptr,delim);
   strcat (buffptr,filename);

#ifdef BACKWARDS
   free(path_copy);
#endif
                  
   return buffptr;
}

/*
 * int buildEnv (char *const *envp);
 *
 * Pre:  envp is a pointer to an array of strings of the format NAME=VALUE.
 *
 * Post: On success, the strings in envp are added to the environment via
 *       putenv(), and 0 is returned.  Returns -1 and sets errno=ENOMEM
 *       on failure.
 */
                              
int
buildEnv (char *const *envp) {
   while (*envp != NULL) {
      if (putenv(*envp) == -1) return -1;
      envp++;
   }
   return 0;
}


/*
 * This function provides the functionality of the execve() routine
 * provided with the GNO distribution.  Unfortunately, GNO's version
 * uses a non-standard prototype.  (That's why execve is given a
 * different prototype in this library.
 */

#if 0	/* it resides in trap.asm */
int _execve(const char *path, const char *params) {
   return(Kexecve(path, params, &errno));
}
#endif


/*
 * exec -- this function has been obsoleted, but is provided for
 *			backward compatibility
 */

#pragma databank 1

static void
_exec_child (const char *filename, const char *cmdline) {
	_execve(filename, cmdline);
  _exit(-1);
}

#pragma databank 0

int
exec (const char *filename, const char *cmdline) {
	return fork2 (_exec_child, 1024, 0, "forked child of exec(2)", 4,
                filename, cmdline);
}

/*
 * The next functions (execv, execvp, execve, execl, execlp) are as per
 * their man page descriptions.  On error, errno could be set to any of
 * ENOMEM, EIO, ENOENT.  Note that there is no execle; Orca/C pukes on
 * its declaration.
 */

int
execv(const char *path, char * const *argv) {
   char *comd;

   /* BUG BUG BUG BUG!   -- this should be using <path> ! */

   /* build the command line */
   if ((comd = buildCmd (argv)) == NULL) return -1;

   /* execute it */
   return (Kexecve(path, comd, &errno));
}


int
execvp(const char *file, char * const *argv) {
   char *comd;
   char *path;

   /* build the path name, if necessary */
   path = buildPath (file);

   /* build the command line */
   if ((comd = buildCmd (argv)) == NULL) return -1;

   /* execute it */
   return(Kexecve(path, comd, &errno));
}

int
execve (const char *path, char * const *argv, char * const *envp) {
   char *comd;

   /* build the command line */
   if ((comd = buildCmd (argv)) == NULL) return -1;

   /* build the environment */
   if (buildEnv(envp) == -1) return -1;

   /* execute it */
   return(Kexecve(path, comd, &errno));
}   

/* no stack repair code on variadic function definitions */
/* pragma optimize 79 */
#pragma optimize 8
#pragma debug 0

int
execl(const char *path, const char *arg, ...) {
   va_list list;
   char **argv;
   char *p;
   char **q;
   int i=0;
   size_t vect_length;
   int result;

   /* allocate memory for initial guess of number of parameters */
   argv = (char **) malloc (sizeof(char *) * PARMSGUESS);
   if (argv==NULL) return -1;
   vect_length = PARMSGUESS;

   /* build the array */
   p = arg;
   va_start (list, arg);
   while (p && *p) {
      argv[i] = p; i++;
      p = va_arg(list, char* );

      /* reallocate memory if necessary */
      if (i>=vect_length) {
         vect_length += PARMSGUESS;
         q = (char **) realloc(argv,vect_length);
         if (p == NULL) {
            free(argv);
            errno = ENOMEM;
            return -1;
         }
         argv = q;
      }
   }
   argv[i] = (char *) NULL;

   va_end(list);
   result = execv(path,argv);

   /* execvp failed; free argv */
   free(argv);
   return result;
}               

int
execlp(const char *file, const char *arg, ...) {
   va_list list;
   char **argv;
   char *p;
   char **q;
   int i=0;
   size_t vect_length;
   int result;

   /* allocate memory for initial guess of number of parameters */
   argv = (char **) malloc (sizeof(char *) * PARMSGUESS);
   if (argv==NULL) return -1;
   vect_length = PARMSGUESS;

   /* build the array */
   p = arg;
   va_start (list, arg);
   while (p && *p) {
      argv[i] = p; i++;
      p = va_arg(list, char* );

      /* reallocate memory if necessary */
      if (i>=vect_length) {
         vect_length += PARMSGUESS;
         q = (char **) realloc(argv,vect_length);
         if (p == NULL) {
            free(argv);
            errno = ENOMEM;
            return -1;
         }
         argv = q;
      }
   }
   argv[i] = (char *) NULL;

   va_end(list);
   result = execvp(file,argv);

   /* execvp failed; free argv */
   free(argv);
   return result;
}
