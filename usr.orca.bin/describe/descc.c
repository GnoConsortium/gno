#ifndef _ORCAC_
#define   _ORCAC_
#endif

#ifdef    _ORCAC_
 #pragma optimize  -1 
 #pragma stacksize 512
#endif /* _ORCAC_ */

/*                                                                        */
/* descc - compile info file into describe database file                  */
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
/*         7 variable-length Null-terminated strings.                     */
/*                                                                        */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma lint -1

/* prototypes */

void usage(char *callname);
void puke(int error,int lines);
int  mygets(char *buffer, int *lines, FILE *FInPtr);

/* defines */

typedef struct nameEntry_tag

   {
   char     name[34];
   long int offset; 
   }nameEntry;

#define _VERSION_ "v1.0.0"
#define QUOTE_CHAR '#'
#define OUTFILE   "/usr/local/lib/describe"

#define FIELD_LEN 9

#define NAME    "Name:    "
#define VERSION "Version: "
#define AUTHOR  "Author:  "
#define CONTACT "Contact: "
#define WHERE   "Where:   "
#define FTP     "FTP:     "

/*                             */
/* usage - you know what to do */
/*                             */

void usage (char *callname)

   {
   fprintf(stderr,"%s %s\n",callname,_VERSION_);
   fprintf(stderr,"usage: %s <describe sourcefile>\n",callname);
   exit(0);
   }

/*                      */
/* puke - stdlib errors */
/*                      */

void puke (int error,int lines)

   {
   fprintf(stderr,"\nError $%x in line %d of script\n",error,lines);
   fflush(stdout);
   exit(error);
   }

/*                                                                         */
/* mygets - get a line (skipping commented lines) and increment line count */
/*                                                                         */

int mygets (char *buffer, int *lines, FILE *FInPtr)

   {
   int i;

   do

      {
      if (fgets(buffer,80,FInPtr)==NULL) 
         return(-1);
      buffer[strlen(buffer)-1] = '\0';        /* remove trailing \n */
      lines++;
      }

   while(buffer[0] == QUOTE_CHAR || buffer[0] == '\n'); 
   return(0);
   }

/*              */
/*   Mainline   */
/*              */

int main (int argc, char **argv)

   {
   FILE *FInPtr, *FOutPtr;
   long int *record_locs, currLoc, endOfFile;
   char *tmpPtr, *buffer;
   int lines, namecount, c, i, j;
   nameEntry nameStruct;

   if (argc != 2)
      usage(argv[0]);

      /* open input and output files */

   buffer = (char *) malloc (81);
   if ((FInPtr = fopen(argv[1],"r")) == NULL)
      
      {
      fprintf(stderr,"Error opening %s; exiting.\n",argv[1]);
      free(buffer);
      exit(1);
      }

   if ((FOutPtr = fopen(OUTFILE,"w+")) == NULL)

      {
      fprintf(stderr,"Error opening output file %s; exiting.\n",OUTFILE);
      free(buffer);
      exit(1);
      }

      /* Compile array of names */

   lines = 0;
   namecount = 0;
   fseek(FOutPtr,2,SEEK_CUR);               /* space for # of array entries */

   while(mygets(buffer,&lines,FInPtr) != -1)

      {
      if (!strncmp(buffer,NAME,FIELD_LEN))       /* found a match */
 
         {
         tmpPtr = &buffer[FIELD_LEN];
         strcpy(nameStruct.name,tmpPtr);
         fwrite(&nameStruct,sizeof(nameStruct),1,FOutPtr);
         namecount++;
         }
            
      }

   record_locs = (long int *) malloc (namecount*sizeof(long int));
   rewind(FInPtr);
   buffer[0] = '\0';
   lines = 0;
   fprintf(FOutPtr,"\t");
    
      /* Increment to first field */

   while (strncmp(buffer,NAME,FIELD_LEN))    /* found a match! */
      mygets(buffer,&lines,FInPtr);

   for (i = 0; i < namecount; i++)

      {
      record_locs[i] = ftell(FOutPtr);
      for (j = 0; j < 5; j++)              /* parse additional info */

         {
         mygets(buffer,&lines,FInPtr);
         tmpPtr = &buffer[FIELD_LEN]; 
         fprintf(FOutPtr,"%s\n",tmpPtr);   /* print out fields */
         }
 
             /* handle comment field */ 

     while (1)

         {
         if (mygets(buffer,&lines,FInPtr) == -1)
            break;
         if (!strncmp(buffer,NAME,FIELD_LEN))    /* until next field */
            break;
         fprintf(FOutPtr,"%s ",buffer);
         }

      fprintf(FOutPtr,"\n");
      }

   endOfFile = ftell(FOutPtr);
   rewind(FOutPtr);
   fwrite(&namecount,2,1,FOutPtr);

      /* time to go through the record_locs array and backpatch in */
      /* all the record locations.  A little slower than necessary */
      /* perhaps, but it gets the job done.                        */
   
   for (i = 0; i < namecount; i++)

      {
      currLoc = ftell(FOutPtr);
      fread(&nameStruct,sizeof(nameStruct),1,FOutPtr);
      fseek(FOutPtr,-(sizeof(nameEntry)),SEEK_CUR);
      nameStruct.offset = record_locs[i];
      fwrite(&nameStruct,sizeof(nameStruct),1,FOutPtr);
      }

   fseek(FOutPtr,endOfFile,SEEK_SET);
   fclose(FOutPtr);
   free(record_locs);
   free(buffer);
   }
