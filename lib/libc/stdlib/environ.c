/*
 * These routines were written by Devin Reade for GNO 2.0.1.
 *
 * $Id: environ.c,v 1.1 1997/02/28 05:12:49 gdr Exp $
 *
 * This file is formatted with tab stops every 3 columns.
 */

#ifdef __ORCAC__
segment "libc_stdlb";
#endif

#pragma databank 1
#pragma optimize 0
#pragma debug 0
#pragma memorymodel 0

#include <stddef.h>
#include <types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <gsos.h>
#include <orca.h>
#include <shell.h>


typedef struct stackelm {
   struct stackelm *next;
   char **env;
} stack_elm;

char **environ = NULL;

static short __environ_initialized = 0;
static short __use_environ = 0;
static short __in_environInit = 0;
static stack_elm *env_stack = NULL;

static char *__findenv(const char *name, int *offset);
static char *getvar(const char *name);
static int  setvar(char *name, const char *value);
static void unsetvar (const char *name);

/*
 * int environPush(void);
 *
 * Pre:  none
 *
 * Post: The current state of the shell variable list is saved.  This 
 *       affects both the internal and the list pointed to by environ.
 *
 *       Returns 0 on success, -1 otherwise.
 */

int
environPush(void) {
   stack_elm *p;
   PushVariablesGSPB parmBlock;

   parmBlock.pCount = 0;
   PushVariablesGS(&parmBlock);

   /* if we're not using environ, then we're finished */
   if(!__use_environ) return 0;

   /* push environ onto the environment stack */
   if ((p = (stack_elm *) malloc (sizeof(stack_elm))) == NULL)
      return -1;
   p->next = env_stack;
   env_stack = p;
   env_stack->env = environ;

   /* zero the new environment and initialize */
   environ = NULL;
   __environ_initialized = 0;
   if (environInit() != 0) {   /* environInit failed; restore old environ */
      __environ_initialized = 1;
      environ = env_stack->env;
      p = env_stack;
      env_stack = env_stack->next;
      free(p);
      return -1;
   }
   return 0;
}


/*
 * void environPop(void);
 *
 * Pre:  none
 *
 * Post: The shell variable list is restored to the state that it was in
 *       when the most recent environPush() call was made.  This affects both
 *       the internal version, and the list pointed to by environ.
 */

void
environPop(void) {
   stack_elm *s;
   char **p, **q;
   PushVariablesGSPB parmBlock;

   parmBlock.pCount = 0;
   PopVariablesGS(&parmBlock);
   
   /* if we're not using environ, then we're finished */
   if(!__use_environ) return;

   if(!env_stack) return;  /* empty stack */

   /* restore environ to its previous value */
   p = environ;
   environ = (env_stack) ? env_stack->env : NULL;
   s = env_stack;
   env_stack = (env_stack) ? env_stack->next : NULL;

   /* free up each element in the discarded environment */
   q = p;
   while (q && *q) {
      free(*q);
      q++;
   }

   /* free the discarded environment */
   if (p) free(p);

   /* free the discarded environment stack element */
   if (s) free(s);

   return;
}
   

/*
 * static int setvar (char *name, const char *value);
 *
 * Purpose:  to set shell variable <name> to <value>.  This affects only
 *    the internal representation, not that of the environ variable.
 *
 * Pre:  <name> and <value> are null-terminated strings
 *
 * Post: <name> is set to <value>.  Returns 0 on success, -1 on failure
 *
 * Acknowledgements:  This routine is modified from code written by
 *    Dave Tribby [ GEnie: D.TRIBBY Internet: tribby@cup.hp.com ]
 *    for the "evaluate" shell utility for the Orca shell.  Used
 *    with permission.
 */

static int
setvar(char *name, const char *value) {
   int                  error;
   GSString255Ptr       var_value;     /* Holds variable sized string */
   SetGSPB              set_var_pb;
   static GSString255   var_name;


   /* Shell call requires three parameters */
   set_var_pb.pCount = 3;

   /* Create GSOS string that holds the name of the variable */
   /* Truncate value if > size of GSString255                */
   var_name.length = strlen(name);
   if (var_name.length > sizeof(var_name.text)) {
      var_name.length = sizeof(var_name.text);
      strncpy(var_name.text, name, sizeof(var_name.text));
   } else {
      strcpy(var_name.text, name);
   }
   set_var_pb.name = &var_name;

   /* Allocate a GS string large enough to hold the value */
   var_value = (GSString255Ptr) malloc(strlen(value)+sizeof(Word));
   if (var_value == NULL) return (-1);

   var_value->length = strlen(value);
   strcpy(var_value->text, value);

   set_var_pb.value = var_value;
   set_var_pb.export = 1;

   /* Make the shell call to set the variable */
   SetGS(&set_var_pb);
   error = toolerror();
   free (var_value);
   if (error) {
      return -1;
   } else {
      return 0;
   }
}   /* setvar */


/*
 * static void unsetvar (const char *name);
 *
 * Pre:  <name> points to the name of the shell variable to be deleted.  It
 *       may have a trailing '='.
 *
 * Post: The variable is deleted from the shell's internal environment.
 *       Any further references to it will return a NULL pointer.
 */

static void
unsetvar (const char *name) {
   UnsetVariableGSPB parmblock;
   GSString255       parmname;
                                   
   /*
   ** delete the internal version
   */

   /* set up the parameters */
   parmblock.pCount = 1;
   parmblock.name = &parmname;
   parmname.length = strlen(name);
   if (parmname.length > 254) parmname.length = 254;
   strncpy(parmname.text,name,parmname.length);
   if (parmname.text[parmname.length -1] == '=') {
      parmname.text[parmname.length -1] = (char) NULL;
   } else {
      parmname.text[parmname.length] = (char) NULL;
   }

   UnsetVariableGS(&parmblock);
   return;
}


/*
 * static char *getvar (const char *name);
 *
 * Purpose:  to get the value of shell variable <name>, using the internal
 *       (not environ) representation.
 *
 * Pre:  <name> is a null-terminated string
 *
 * Post: returns a pointer to the value of <name>.  If the variable has
 *       not been set or if the program is executing from an environment
 *       where shell variables do not exist, a NULL value is returned.
 *
 * Acknowledgements:  This routine is modified from code written by
 *       Dave Tribby [ GEnie: D.TRIBBY Internet: tribby@cup.hp.com ]
 *       for the "evaluate" shell utility for the Orca shell.  Used
 *       with permission.
 */

static char *getvar(const char *name) {
   
   ReadVariableGSPB        get_var_pb;
   static ResultBuf255     var_value;
   static GSString255      var_name;
   char                    *result;
   int                     length;

   /* Shell call requires three parameters */
   get_var_pb.pCount = 3;

   /* Create GSOS string that holds the name of the variable */
   /* Truncate value if > size of GSString255                */
   var_name.length = strlen(name);
   if (var_name.length > sizeof(var_name.text)) {
      var_name.length = sizeof(var_name.text);
      strncpy(var_name.text, name, sizeof(var_name.text));
   } else {
      strcpy(var_name.text, name);
   }
   get_var_pb.name = &var_name;

   /* initialize the result buffer */
   var_value.bufSize = sizeof (GSString255);
   get_var_pb.value = &var_value;

   /* Make the shell call to get the variable */
   ReadVariableGS(&get_var_pb);

   /* failed if tool error or not for export */
   if (toolerror() || (get_var_pb.export == 0)) return (NULL);

   /* get length of variable value */
   length = ((ResultBuf255Ptr) get_var_pb.value)->bufString.length; 

   /* failed if variable not defined (length zero) */
   if (length == 0) return (NULL);

   /* set the NULL terminator */
   result = (((ResultBuf255Ptr) get_var_pb.value)->bufString.text);
   result[length] = (char) NULL;

   return (result);
}   /* getvar */


/* int environInit (void)
 *
 * Purpose:  to initialize environ.  This need not be done if the calling
 *       program does not need to access environ directly [that is, if it
 *       restricts itself to using getenv(), setenv(), putenv(), and
 *       unsetenv() ]
 *
 * Pre:  none
 *
 * Post: the environment environ is initialized, and contains entries for
 *       all defined internal shell variables.  Returns 0 on success,
 *       non-zero otherwise.
 */

int environInit (void) {

   static ReadIndexedGSPB  parmBuffer;
   static ResultBuf255     nameBuffer, valueBuffer;
   unsigned int nameLength, valueLength;
   char *name;
   char *value;

   /* make sure we only do this once */
   if (__environ_initialized) return 0;
   __environ_initialized = 1;
   __use_environ = 1;
   __in_environInit = 1;

   /*
   ** initialize the parameter block
   */

   parmBuffer.pCount = 4;
   parmBuffer.index = 1;
   nameBuffer.bufSize  = sizeof (GSString255);
   valueBuffer.bufSize = sizeof (GSString255);
   parmBuffer.name  = &nameBuffer;
   parmBuffer.value = &valueBuffer;

   /* get space for our name and value buffers */
   name = (char *) malloc (255 * sizeof(char));
   if (!name) {
      __in_environInit = 0;
      return 1;
   }
   value = (char *) malloc (255 * sizeof(char));
   if (!value) {
      free(name);
      __in_environInit = 0;
      return 1;
   }
   
   /*
   ** add each variable into environ as they appear in the shell
   ** environment
   */

   ReadIndexedGS (&parmBuffer);
   nameLength = nameBuffer.bufString.length;
   while (nameLength != 0) {
      valueLength = valueBuffer.bufString.length;
      
      /* copy the name and value */
      strncpy (name, nameBuffer.bufString.text, nameLength);
      name[nameLength] = (char) NULL;
      strncpy (value, valueBuffer.bufString.text, valueLength);
      value[valueLength] = (char) NULL;

      /* try to place it in environ */
      if (setenv(name, value, 1) != 0) {
         free(name);
         free(value);
         __in_environInit = 0;
         return 1; 
      }

      /* get the next shell variable and continue ... */
      parmBuffer.index++;
      ReadIndexedGS (&parmBuffer);
      nameLength = nameBuffer.bufString.length;
   }

   free(name);
   free(value);
   __in_environInit = 0;
   return 0;        
}  /* environInit() */


/*
 * int putenv (const char *str)
 *
 * Purpose: Take a string of the form NAME=value and stick it into the
 *          environment.
 *
 * Pre:  <str> is a null-terminated string of the form NAME=value
 *
 * Post: returns zero if successful.  A non-zero value indicates that
 *       space to expand the environment pointer table could not be
 *       acquired; in this case, the string has not been added
 *
 * Warning:
 *       Certain naming restrictions may apply if the environment variable
 *       is referenced by shell programs
 */


int putenv (const char *str) {
   char *name, *value;
   size_t l_str;
   int result;

   /* get space for our buffer */
   l_str = strlen(str);
   name = (char *) malloc (l_str + 1);
   if (!name) return -1;

   strcpy(name,str);

   /* replace the '=' with a null and set value */
   for (value=name; (*value) && (*value != '='); value++);
   if (*value == '=') {    /* found the end of name */
      *value = (char) NULL;
      value++;
      result = (*value) ? setenv(name,value,1) : -1;
   } else {
      result = -1;
   }
   free(name);
   return result;
}


/*
 * char *getenv (const char *NAME)
 *
 * Purpose: search the environment for a string of the format NAME=VALUE
 *
 * Pre:  NAME is the name of the variable for which the value is to be
 *       retrieved.  It may end with an extra '=' which is not part of the
 *       name.
 *
 * Post: returns a pointer to the value of NAME.  If NAME is not defined, it
 *       returns NULL.  getenv() is case sensitive to NAME.
 */

char *getenv(const char *name) {
   char *result;
   size_t length;

   length = strlen(name);
   if (!length) return NULL;

   if(name[length-1] == '=') {
	   char *tmp_name;

      if ((tmp_name = malloc(length+1)) == NULL) return NULL;
      strcpy(tmp_name,name);
      tmp_name[length-1] = (char) NULL;
      result = getvar(tmp_name);
      free(tmp_name);
   } else {
      result = getvar(name);
   }
   return result;
}

/*
 * static char *__findenv(const char *name, int *offset);
 *
 * Pre: <name> is a null-terminated string, which may end with '='.
 *
 * Post: returns a pointer to the value associated with <name> in the
 *       environment, if any, else it returns NULL.  Sets <offset> to
 *       be the offset of the name/value combination in the environmental
 *       array (environ), for use by setenv(3) and unsetenv(3).  It
 *       explicitly removes '=' in argument <name>.
 *
 * Acknowledgements:  This is based on UCB code; see the above legalese.
 */

static char *__findenv(const char *name, int *offset) {
   unsigned int len;
   char **P, *C;

   if (environ==NULL) {
      return NULL;
   }

   for (C = name, len = 0; *C && (*C != '='); C++, len++);

   for (P = environ; *P; P++) {
      if (!strncmp(*P, name, len)) {
         C = *P + len;
         if (*C == '=') {
            *offset = P - environ;
            C++;
            return(C);
         }
      }
   }
   return(NULL);
}


/*
 * int setenv (const char *name, const char *value, int rewrite);
 *
 * Pre:  <name> is the name of the environment variable to be set with
 *       value <value>.  <rewrite> is either unset (zero) or set (non-
 *       zero).
 *
 * Post: If <name> does not previously exist, then it is added to the
 *       environment with value <value>.  If <name> has been previously
 *       set, then <rewrite> is tested:  If <rewrite> is non-zero then
 *       the old value is replaced, otherwise the call has no effect.
 *
 *       Returns zero on success or if <name> already exists and <rewrite>
 *       is not set.  Returns -1 and sets errno on failure.
 */

int setenv (const char *name, const char *value, int rewrite) {
   static int alloced;        /* if allocated space before */
   char *C;
   size_t l_value, l_name;
   int offset;
   char *tmp_name;
   char *tmp_str;

   if (*value == '=') value++;      /* ignore any prepended '=' in value */
   l_name = strlen(name);           /* get the string lengths */
   l_value = strlen(value);
   if(name[l_name-1] == '=') l_name--;    /* ignore any appended '=' in name */
   if ((l_name == 0) || (l_value == 0)) { /* bad args! */
	   errno = EINVAL;
      return -1;
   }

   /*
   ** make a copy of the name
   */

   tmp_name = (char *) malloc (l_name+1); /* allocate necessary memory */
   if (tmp_name == NULL) return -1;
   strncpy(tmp_name, name, l_name);       /* do the copy */
   tmp_name[l_name] = '\0';

   /*
   ** make a string of the form name=value, if necessary
   */

   if (__use_environ) {             /* are we using the environ structure? */
      tmp_str = (char *) malloc (l_name + l_value + 2);
      if (!tmp_str) {
         free(tmp_name);
         errno = ENOMEM;
         return -1;
      }
      strcpy (tmp_str,tmp_name);
      strcat (tmp_str,"=");
      strcat (tmp_str,value);
   }


   /*
   ** Change the internal version
   */
   if ((!__in_environInit) && ((rewrite) || (getenv(tmp_name) == NULL))) {
      if (setvar(tmp_name, value)) {
	      int tmp_err = errno;

         free(tmp_name);
         if (__use_environ) free(tmp_str);
         errno = tmp_err;
         return -1;
      }
   }
   if (__use_environ==0) {
      free(tmp_name);
   	return 0;
   }

   /*
   ** Change the external (environ) version
   */

   C = __findenv(tmp_name, &offset);  /* find if already exists */
   if (C!=NULL) {
      if (!rewrite) {
	      free(tmp_name);
         free(tmp_str);
         return 0;
      }
      if (strlen(C) >= l_value) {         /* old larger; copy over */
         while (*value) {
            *C = *value;
            C++;
            value++;
         }
         free(tmp_name);
         free(tmp_str);
         return 0;
      }
   } else {                               /* not found; create new slot */
      int   cnt;
      char  **P;

      cnt = 0;
      if (environ) for (P = environ; *P; P++) cnt++;

      if (alloced) {       /* done before; just increase size */

         P = (char **) realloc ((char *)environ,
            (size_t)(sizeof(char *) * (cnt + 2)));
         if (!P) {         /* realloc failed */
            unsetvar(tmp_name);
            free(tmp_name);
            free(tmp_str);
            errno = ENOMEM;
            return -1;
         } else {
            environ = P;
         }
      } else {             /* first time; get new space */
         alloced = 1;      /* copy old entries into it */
         P = (char **) malloc ((size_t) (sizeof(char *) * (cnt + 2)));
         if (!P) {
            unsetvar(tmp_name);
            free(tmp_name);
            free(tmp_str);
            errno = ENOMEM;
            return -1;
         }

         /* 
          * original was:
          *    bcopy(environ, P, cnt * sizeof(char *));
          * changed so that we could use the standard Orca libraries
          * for non-gno implementations.
          */
         if (environ) memcpy(P, environ, cnt * sizeof(char *));
         environ = P;
      }
      environ[cnt + 1] = NULL;
      offset = cnt;
   }
   
   /* we've got the new slot, now add it in */
   environ[offset] = tmp_str;
   free(tmp_name);
   return 0;
}


/*
 * void unsetenv (const char *name);
 *
 * Pre:  <name> points to the name of the shell variable to be deleted.  It
 *       may have a trailing '='.
 *
 * Post: The variable is deleted.  Any further references to it will return
 *       a NULL pointer.  This routine unsets both the internal and, if it's
 *       initialized, the environ representations.
 *
 * Acknowledgements:  Contains BSD code.  See the above legalese.
 */

void unsetenv (const char *name) {
   char **P;
   int offset;

   /*
   ** delete the internal version
   */

   unsetvar(name);
   if(!__use_environ) return;

   /*
   ** delete the environ version, if necessary.
   */

   while (__findenv(name, &offset)!=NULL) {  /* if set multiple times */

      free(environ[offset]);
      for (P = &environ[offset];; P++)
         if (!(*P = *(P + 1)))
            break;
   }

   return;
}
