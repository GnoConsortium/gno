#pragma optimize  15 
#pragma stacksize 512

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma lint -1

#ifdef STACK_CHECK
void begin_stack_check(void);
int  end_stack_check(void);
#endif

/* prototypes */

void usage(char *callname);
void print_entry(FILE *FInPtr, long int index);
void myprintf(char *string, int wordwrap_size);

/* defines */

typedef struct nameEntry_tag

   {
   char     name[34];
   long int offset;
   }nameEntry;

#ifndef FALSE
 #define FALSE 0
 #define TRUE  1
#endif

#define _VERSION_ "v1.0"
#define INFILE "/usr/local/lib/describe"

#define NAME    "Name:    "
#define VERSION "Version: "
#define AUTHOR  "Author:  "
#define CONTACT "Contact: "
#define WHERE   "Where:   "
#define FTP     "FTP:     "

void usage(char *callname)

   {
   fprintf(stderr,"Describe %s\n",_VERSION_);
   fprintf(stderr,"usage: %s -[v] <utilityname>\n",callname);
   exit(0);
   }

void myprintf(char *string, int wordwrap_size)

   {
   int length = 0;
   char *headString, *tailString;
 
   headString = tailString = string;
   printf("\n");
   while (1)

      {
      tailString++;  length++;
      if (*tailString == '\0')

         {
         printf("%s",headString);
         return;
         }

      else if (length == wordwrap_size)

         {
         while (*tailString != ' ')
            tailString--;
         *tailString = '\0';
         printf("%s\n",headString);
         headString = tailString+1; 
         length = 0;
         }

      }

   }

void print_entry(FILE *FInPtr, long int index)

   {
   char *buffer;

   buffer = (char *) malloc (1024);

   fseek(FInPtr,index,SEEK_SET);

   printf("%s",VERSION);
   fgets(buffer,80,FInPtr);
   printf("%s",buffer);

   printf("%s",AUTHOR);
   fgets(buffer,80,FInPtr);
   printf("%s",buffer);

   printf("%s",CONTACT);
   fgets(buffer,80,FInPtr);
   printf("%s",buffer);

   printf("%s",WHERE);
   fgets(buffer,80,FInPtr);
   printf("%s",buffer);

   printf("%s",FTP);
   fgets(buffer,80,FInPtr);
   printf("%s",buffer);

   fgets(buffer,1024,FInPtr);
   myprintf(buffer,75);

   free(buffer);
#ifdef STACK_CHECK
   printf("Stack: %d\n",end_stack_check());
#endif
   exit(0);
   }

int main (int argc, char **argv)

   {
   FILE *FInPtr;
   char searchName[34];
   long int index;
   int verbose, argind, numOfEntries, cmp, offset1, offset2, check, i;
   nameEntry nameStruct;

#ifdef STACK_CHECK
   begin_stack_check();
#endif
   
   verbose = FALSE;
   argind  = 1;

   if ((argc == 3) && (!strcmp(argv[1],"-v")))

      {
      verbose = TRUE;
      argind++;
      }

   else if (argc != 2)
      usage(argv[0]);

   FInPtr = fopen(INFILE,"r");
   fread(&numOfEntries,2,1,FInPtr);
   offset1 = 0;
   offset2 = numOfEntries-1;

   strcpy(searchName,argv[argind]); 
   i=0;
   while(searchName[i] = tolower(searchName[i++]));

   if (verbose)
      printf("Searching...\n");

   while (1)

      {
      check = ((offset2-offset1)/2) + offset1;
      fseek(FInPtr,2+(check*sizeof(nameEntry)),SEEK_SET);
      fread(&nameStruct,sizeof(nameEntry),1,FInPtr);

      cmp = strcmp(nameStruct.name,searchName);

      if (verbose)
         printf("  checked %s\n",nameStruct.name);

      if (cmp > 0)
         offset2 = check-1;

      else if (cmp < 0)
         offset1 = check+1;

      else

         {
         if (verbose)
   
            {
            printf("Found entry %s!\n",searchName);
            exit(0);
            }

         printf("%s%s\n",NAME,searchName);
         print_entry(FInPtr,nameStruct.offset);
         }

      if (offset1 > offset2)

         { 
         printf("Entry '%s' not found in describe database.\n",searchName);
         exit(1);
         }

      }

   }
