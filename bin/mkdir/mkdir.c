#pragma optimize -1
#pragma stacksize 2048

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<memory.h>
#include<gsos.h>
#include<errno.h>

void usage(char *callname)

   {
   fprintf(stderr,"usage: %s -[s|V] <dir1> <dir2> ...\n",callname);
   exit(0);
   }

void main(int argc, char **argv)

   {
   FileInfoRecGS *InfoStruct;
   int i = 1, exitFlag=0, silent = FALSE;

   InfoStruct = (FileInfoRecGS *) malloc (sizeof(FileInfoRecGS));
   InfoStruct->pathname =  malloc (256);
   InfoStruct->access = 0xC3;
   InfoStruct->fileType = 0x0F;
   InfoStruct->pCount = 3;

   if (argc < 2)
      usage(argv[0]);

   else if (!strcmp(argv[1],"-s"))

      {
      silent = TRUE;
      i++;
      }

   else if (!strcmp(argv[1],"-V"))

      {
      printf("mkdir v1.1\n");
      exit(0); 
      }

   else if (argv[1][0] == '-')        /* invalid option */
      usage(argv[0]);

   while(argv[i] != NULL)      /* create subdirectories */

      {
      InfoStruct->pathname->length = (word) strlen(argv[i]);
      strcpy(InfoStruct->pathname->text,argv[i]);
      Create(InfoStruct);
      errno = toolerror();

      if (errno)

         {
         if (!silent)
            fprintf(stderr,"error $%x creating %s",errno,argv[i]);
         if (errno == 0x40)

            {
            if (!silent)
               fprintf(stderr,": invalid pathname syntax\n");
            exitFlag=1;
            }

         else if (errno == 0x44)

            {
            if (!silent) 
               fprintf(stderr,": subdirectory does not exist\n");
            exitFlag=1;
            }

         else if (errno == 0x47)

            {
            if (!silent)
               fprintf(stderr,": duplicate directory name\n");
            exitFlag=2;
            } 

         else if (errno == 0x2B)
 
            {
            if (!silent)
               fprintf(stderr,": volume write protected\n");
            exitFlag=1;
            }
           
         else

            {
            if (!silent)
               fprintf(stderr,"\n");
            exitFlag=1;
            }

         }

      i++;
      } 
   
   free(InfoStruct->pathname);
   free(InfoStruct);
   exit(exitFlag);
   }
