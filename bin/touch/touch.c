/* Touch - GS version of UNIX touch.  If file exists, sets mod. date to
   current date.  If file does not exist, creates a text file with that name.

   Copyright 1993 by Leslie M. Barstow III.
   Placed in the Public Domain with the following condition:
      Please maintain original Copyright in source, program, and manpage.
*/

#pragma debug 0
#pragma optimize -1
#pragma stacksize 1024

#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <orca.h>

typedef struct GSString {
  Word			length;
  char			string[1];
} GSString;

typedef struct FileInfoRecGS {
	Word			pCount;
	GSString		*pathname;
	Word			access;
	Word			fileType;
	LongWord		auxType;
	Word			storageType;	/* must be 0 for SetFileInfo */
	TimeRec			createDateTime;
	TimeRec			modDateTime;
} FileInfoRecGS;

typedef struct CreateRecGS {
   Word pCount;
   GSString *pathname;
   Word access;
   Word fileType;
   } CreateRecGS;

#ifndef NULL
#define NULL  0L
#endif

extern TimeRec ReadTimeHex();

#ifndef stackEntry
	#define stackEntry	0xE100B0
#endif

#ifndef PDosInt
extern pascal void PDosInt(); 
#endif

#define GetFileInfoGS(pBlockPtr)		PDosInt(0x2006,pBlockPtr)
#define SetFileInfoGS(pBlockPtr)	   PDosInt(0x2005,pBlockPtr)
#define CreateGS(pBlockPtr)           PDosInt(0x2001,pBlockPtr)

int main(int argc, char **argv)
{int i,j;
 GSString *filnam=(GSString *)NULL;
 CreateRecGS crtspace;
 FileInfoRecGS infospace;

 infospace.pCount = 7;
 crtspace.pCount = 3;
 crtspace.access = 0x00e3;
 crtspace.fileType = 0x0004;

 if (argc == 1)
   {fputs("Usage: touch file1 [file2 ...]\n", stdout);
    fputs("GS touch: Copyright 1993 by Leslie M. Barstow III\n",stdout);
    exit(-1);
   }

 for(i = 1; i < argc; i++)
	{j=strlen(argv[i]);
   if ((filnam=(GSString *)malloc(3+j)) == (GSString *)NULL)
     {fputs("touch - Error allocating string memory\n", stdout);
      return(-1);
     }
   filnam->length = j;
   strcpy(filnam->string,argv[i]);
   infospace.pathname=filnam;
   GetFileInfoGS(&infospace);
   if (j = toolerror())
	   {switch (j)
	      {case 0x46:  crtspace.pathname = filnam;
                     CreateGS(&crtspace);
                     if (j = toolerror())
                       {fputs("touch - unable to create file: ", stdout);
                        fputs(argv[i], stdout);
                        fputs(".\n", stdout);
                        exit(-1);
                       }
                     break;
         case 0x40:
         case 0x44:
         case 0x45:
         case 0x52:
         case 0x58:  fputs("touch - Invalid filename:", stdout);
                     fputs(argv[i], stdout);
                     fputs(".\n", stdout);
                     exit(-1);
                     break;
         case 0x27:  fputs("touch - Disk I/O Error for file: ", stdout);
                     fputs(argv[i], stdout);
                     fputs(".\n", stdout);
                     exit(-1);
                     break;
         default:    fputs("touch - internal error.\n", stdout);
                     exit(-1);
                     break;
        }
     }
   else
     {infospace.modDateTime = ReadTimeHex();
      infospace.storageType = 0;
      infospace.access |=0x20;
      infospace.modDateTime.extra = (Byte)0;
      SetFileInfoGS(&infospace);
     }
   free(filnam);
  }
}
