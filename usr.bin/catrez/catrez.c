/* ---------------------------------------------------------------------
 *
 * Concatenate resources multiple files into the resource fork of a file
 *            
 *    catrez [-v] [-a] -d destfile file1 [file2 ... ]
 * where the options mean:
 *    -v   verbose output
 *    -a   append resources to destination rather than overwriting
 *    -d destfile   the destination file name [required]
 *    file1 file2 ...  the source file(s) [at least 1 required]
 *                                                      
 * Written by Dave Tribby (tribby@cup.hp.com) beginning 5-3-96
 *
 * $Id: catrez.c,v 1.2 1997/09/26 06:32:43 gdr Exp $
 *
 * ---------------------------------------------------------------------
 */

char	*copyright    = "  Copyright 1996-1997 by David M. Tribby\n";
char    *prog_version = "  Version 1.0.2 (%s)\n";

/* NOTE: if you need to compile this without code specific to gno, */
/* #define __NO_GNO__                                              */

#include <Types.h>
#include <Memory.h>
#include <GSOS.h>
#include <Resources.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <orca.h>

#ifndef __NO_GNO__
#include <GNO/gno.h>
#include <signal.h>
#endif

#pragma lint -1
/* #pragma debug 25 */		/* Only for development */
#pragma optimize 95		/* Bit 5 off due to defect in ORCA/C 2.1.0 */


/* Scheduling parameters */
int		verboseflag = FALSE;
int		appendflag = FALSE;
GSString255Ptr	dest_filename = NULL;

Word		dest_file_id = 0;	/* GSOS file ID for destination */
int		status_return = 0;	/* Program return status */
int		user_break = FALSE;	/* TRUE if user hits break (GNO) */

/* Error message */
char		*rezopenerr = "Error $%04X opening resource fork of %s\n";


/* Resources.h left out this one: don't pre-load when opening file */
#define noPreload 0x8000


#ifndef __NO_GNO__
/* Running under GNO or ORCA? */
int		gnoactive;

/* ---------------------------------------------------------------------
 *
 * SigHandler
 *
 *    Signal handler routine (only used under GNO)
 *    Don't do anything fancy; just terminate quickly after these sigs
 *
 *----------------------------------------------------------------------
 */
#pragma databank 1
void SigHandler(int sig, int code)
   {
   switch (sig) {
      case SIGHUP:	/* Hang-up */
      case SIGINT:	/* User keyboard interrupt */
      case SIGPIPE:	/* Writing to a pipe with no reader */
      case SIGALRM:	/* Alarm timer expiration */
      case SIGTERM:	/* Kill */
         user_break = TRUE;
         status_return = 1;
         break;
      }
   }   /* SigHandler */
#pragma databank 0
#endif



/* ---------------------------------------------------------------------
 *
 *  Resource converter routine
 *
 *    This routine copies a resource in and out of memory without
 *    modification. It is used for resources with the "converter"
 *    attribute bit set. Since catrez doesn't want any conversion
 *    to take place, it makes no modifications.
 *                             
 *    See pp 45-21 to 45-26 in the Apple IIGS Toolbox Reference, Vol 3
 *                             
 * ---------------------------------------------------------------------
 */
#pragma databank 1
#pragma toolparms 1
pascal long NullConverter(
		Word         convertCommand, /* 0=read, 2=write, 4=size */
		IORecPtrGS   convertParam,   /* read/write control block */
                ResRefRecPtr resPointer)     /* resource reference ptr */
   {
   long		return_val = 0;		/* Function return value */
   int		command_code;		/* GS/OS command code    */

   if (convertCommand == 4)   {
      /* ReturnDiskSize */
      return_val = GetHandleSize((Handle)resPointer->resHandle);
      }
   else   {
      /* Do the Read or Write */
      if (convertCommand == 2) 
         command_code = 0x2013;		/* Write */
      else
         command_code = 0x2012;		/* Read */
      /* Since the asm code for calling GS/OS was provided in */
      /* the toolbox reference, insert it here.               */
      asm {
         pei convertParam+2	; Pointer to GS/OS
         pei convertParam	;   parameter block
         pei command_code	; GS/OS read or write
         jsl 0xE100B0		; Call GS/OS
         sta return_val		; Save error code
         };
      }
   return return_val;
   }   /* NullConverter */
#pragma toolparms 0
#pragma databank 0
                   



/* ---------------------------------------------------------------------
 *
 * AllDone
 *
 *    Function called at completion of program; set by "atexit()"
 *
 * ---------------------------------------------------------------------
 */
void AllDone(void)
   {
   /* Make sure destination file is compacted */
   if ( dest_file_id )   CompactResourceFile(0, dest_file_id);

   /* Shut down the resource manager (which closes open resource files) */
   ResourceShutDown();
   }   /* AllDone */



/* ---------------------------------------------------------------------
 *
 * CtoGS
 *
 * Turn C-string into a GS/OS input string
 *
 *
 * ---------------------------------------------------------------------
 */
GSString255Ptr CtoGS(char *c_str)
   {
   GSString255Ptr gs_str;

   /* Allocate memory for string, null char, & length word */
   if (gs_str = (GSString255Ptr) malloc(strlen(c_str)+5))   {
      /* Set length field and copy text */
      gs_str->length = strlen(c_str);
      strcpy(gs_str->text, c_str);
      }
   return gs_str;
   }   /* CtoGS */



/* ---------------------------------------------------------------------
 *
 * OpenDestination
 *
 *   Open or create the destination file
 *
 * ---------------------------------------------------------------------
 */
Word OpenDestination(void)
   {
   int 			    error;
   static OpenRecGS	    open_rec  = { 10, 0, NULL, readWriteEnable, 1};
   static SetPositionRecGS  eof_rec   = { 3, 0, 0, 0 };
   static RefNumRecGS	    close_rec = { 1 };

   /* If append flag isn't set, remove existing resources (if any) */
   if (!appendflag)   {
      /* Open resource fork of destination file */
      open_rec.pathname = dest_filename;
      OpenGS(&open_rec);
      if (!toolerror())   {
         /* Set EOF position at 0 */
         eof_rec.refNum = open_rec.refNum;
         SetEOFGS(&eof_rec);
         /* Close the file */
         close_rec.refNum = open_rec.refNum;
         CloseGS(&close_rec);
         }
      }

   /* Create destination file resource fork; no-op if file */
   /* already exists and has a resource fork.              */
   /* Note: "unknown" file type (0) is used.               */
   CreateResourceFile(0, 0, readWriteEnable+renameEnable+destroyEnable,
   		(Pointer)dest_filename);

   /* Open the destination file resource fork */
   dest_file_id = OpenResourceFile(noPreload+readWriteEnable,
                                           NULL, (Pointer)dest_filename);
   if (error = toolerror())   {
      printf(rezopenerr, error,dest_filename->text);
      exit(2);
      }
                              
   /* Only want to look at one file at a time */
   SetResourceFileDepth(1);
   }   /* OpenDestination */



/* ---------------------------------------------------------------------
 *
 * CopyResources
 *
 *    Copy resources from the named file to the destination file
 *
 * ---------------------------------------------------------------------
 */
void CopyResources(char *fname)
   {
   GSString255Ptr	src_filename;
   Word			file_id;
   int			error;
   Word			type_index;
   long			rez_index;
   Word			rez_type;
   long			rez_ID;
   Handle		rez_handle;
   Word			rez_attr;

   if (verboseflag)   printf("\nSource file: %s \n", fname);

   /* Turn C-string source file name into a GS/OS input string */
   src_filename = CtoGS(fname);

   /* Open the file's resource fork */
   file_id = OpenResourceFile(noPreload+readEnable,
                                           NULL, (Pointer)src_filename);
   error = toolerror();

   /* Done with the GS/OS string */
   free(src_filename);

   /* Cannot proceed if resource fork wasn't opened */
   if (error)   {
      printf(rezopenerr, error,fname);
      status_return = 1;
      return;
      }
                              
   /* Get resource information from the source file */
   SetCurResourceFile(file_id);

   /* Loop through resource types */
   for (type_index=1; !user_break; type_index++)   {

      /* Get the next type; done when error code is set */
      rez_type = GetIndType(type_index);
      if (toolerror())  break;

      if (verboseflag)   printf(" Resource type %04X:   ", rez_type);
      
      /* Loop through IDs for this type */
      for (rez_index=1; !user_break; rez_index++)   {

         /* Get the next ID for this type; done when error code is set */
         rez_ID = GetIndResource(rez_type, rez_index);
         if (toolerror())  break;           

         if (verboseflag)   printf(" %7lX", rez_ID);
                                              
         /* Special handling required if "converter" attribute is set */
         rez_attr = GetResourceAttr(rez_type, rez_ID);
         if (rez_attr & 0x0800)   {
            if (verboseflag) printf("\n");
            printf("NOTE: Converter required: type %04X, ID %lX, file %s\n",
              rez_type,rez_ID,fname);
            /* Use the "null" converter to preserve raw format */
            ResourceConverter((Pointer)&NullConverter,
                           	rez_type, resLogApp+resLogIn);
            }
                                
         /* Load the resource */
         rez_handle = LoadResource(rez_type, rez_ID);
         if (error = toolerror())   {
            printf("\nError %04X loading type %04X, ID %lX, file %s\n",
              error,rez_type,rez_ID,fname);
            status_return = 1;
            continue;
            }

         /* Detach resource from its file */
         DetachResource(rez_type, rez_ID);

         /* Temporarily reset to destination resource file */
         SetCurResourceFile(dest_file_id);

         /* Add resource to destination file, ignoring "protected" attribute */
         AddResource(rez_handle, rez_attr & ~resProtected, rez_type, rez_ID);
         if (error = toolerror())   {
            printf("\nError %04X adding type %04X, ID %lX\n",
              error,rez_type,rez_ID);
            status_return = 1;
            }            
         else   {
            /* Force it to disk */
            WriteResource(rez_type, rez_ID);
            if (error = toolerror())   {
               printf("\nError %04X writing type %04X, ID %lX\n",
                 error,rez_type,rez_ID);
               status_return = 1;
               }            

            /* Set "protected" attribute again if necessary */
            if (rez_attr & resProtected)   {
              SetResourceAttr(rez_attr, rez_type, rez_ID);
              if (error = toolerror())   {
                 printf("\nError %04X setting attributes for type %04X, ID %lX\n",
                   error,rez_type,rez_ID);
                 status_return = 1;
                 }
              }            
            }            
                                         
         /* Release the resource from memory */
         ReleaseResource(-1, rez_type, rez_ID);
         if (error = toolerror())   {
            printf("\nError %04X releasing type %04X, ID %X\n",
              error,rez_type,rez_ID);
            status_return = 1;
            }
                                               
         /* Resume getting resource information from the source file */
         SetCurResourceFile(file_id);
         }

      if (verboseflag)   printf("\n");
      }

   /* Close the source file */
   CloseResourceFile(file_id);
   }   /* CopyResources */

#if defined(__GNO__) && defined(__STACK_CHECK__)
#include <gno/gno.h>
static void report_stack(void)
{
	fprintf(stderr,"\n ==> %d stack bytes used <== \n", _endStackCheck());
}
#endif

/*----------------------------------------------------------------------*/
int main (int argc, char **argv)
/*----------------------------------------------------------------------*/
{
int     p_num;		/* Parameter number */
int	destfileprovided = FALSE;
char	*illegal_opt="Warning: Illegal option %s ignored\n";
char	*usage=
   "Usage:\n\tcatrez [-v] [-a] -d dest_file file1 [file2 ...]\n";

#if defined(__GNO__) && defined(__STACK_CHECK__)
_beginStackCheck();
atexit(report_stack);
#endif

#ifndef __NO_GNO__
/* Are we running under the GNO kernel? */
kernStatus();
gnoactive = (toolerror() == 0);
if ( gnoactive )   {
   /* Install the GNO signal handlers */
   signal(SIGHUP,  SigHandler);
   signal(SIGINT,  SigHandler);
   signal(SIGPIPE, SigHandler);
   signal(SIGALRM, SigHandler);
   signal(SIGTERM, SigHandler);
   /* Ignore user signals (so they don't kill the program) */
   signal(SIGUSR1, SIG_IGN);
   signal(SIGUSR2, SIG_IGN);
   }                           
#endif

/* If no parameters were provided, print the usage string */
if (argc < 2)   {
   printf(usage);
   printf(copyright);
   printf(prog_version, __DATE__);
   return 0;
   }

/* Parse the option parameters */
p_num = 1;
while ((p_num < argc) && (argv[p_num][0] == '-') )  {
   if (strlen(argv[p_num]) > 2)
      printf(illegal_opt, argv[p_num]);
   else switch ( argv[p_num][1] ) {
      case 'v':
         verboseflag = TRUE;
         printf("%s: ", argv[0]);
         printf(prog_version, __DATE__);
         break;             
      case 'a':
         appendflag = TRUE;
         break;
      case 'd':
         destfileprovided = TRUE;
         p_num++;
         if (p_num == argc)   {
            printf("Error: -d option requires a destination filename\n");
            printf(usage);
            return 1;
            }
         else   {
            dest_filename = CtoGS(argv[p_num]);
            }
         break;
      default:
         printf(illegal_opt, argv[p_num]);
      }
   p_num++;
   }

if (!destfileprovided) {
   printf("Error: No destination filename provided\n");
   printf(usage);
   return 1;
   }

if (p_num == argc) {
   printf("Error: No source filenames provided\n");
   printf(usage);
   return 1;
   }

/* Start the resource manager */
ResourceStartUp(userid());

/* Set the cleanup function */
atexit(AllDone);

/* Open (or create) the destination file */
OpenDestination();

/* Open and copy each of the source files */
while ( (p_num < argc) && !user_break ) {
   CopyResources(argv[p_num]);
   p_num++;
   }

/* Cleanup is done in atexit() */
return status_return;
}
