/*

    awgs.c

    Main loop driver code and awgs wordproc file read routines

*/

#pragma stacksize 2048
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <gsos.h>
#include <shell.h>

#include "awgs.h"

void fileError(char *s)
{
int err;
int cl[2];

   if (err = toolerror()) {
        fprintf(stderr,"%s\n",s);
        ERROR(&err);
        cl[0] = 1; cl[1] = 0; CloseGS(cl);
        exit(1);
   }
}

int noboldflag = 0;
saveArray *docSaveArray;
Ruler *docRulers;
textBlock **docTextBlocks; /* an array of textBlockPtrs */
word docSACount, numRulers, numBlocks;

void readAWGS(char *file)
{
int ref,err,z;          /* refnum, err temp, and loop variable  */
long recBlockSize;      /* size of the upcoming text block      */
int cl[2];              /* pBlock for GS/OS CloseGS call        */
GSString255 f;          /* converted cstring(file) -> GSString  */
OpenRecGS o;            /* pBlock for GS/OS Open call           */
SetPositionRecGS p;
IORecGS i;

    f.length = strlen(file);
    strcpy(f.text,file);
    o.pCount = 7;
    o.pathname = &f;
    o.requestAccess = readEnable;
    o.resourceNumber = 0;
    OpenGS(&o);
    if (err = toolerror()) {
        fprintf(stderr,"aroff: could not open AWGS file %s\n",file);
        ERROR(&err);
        exit(1);
    }
    ref = o.refNum;
    if ((o.fileType != 0x50) || (o.auxType != 0x8010l)) {
        cl[0] = 1; cl[1] = ref; CloseGS(cl);
        fprintf(stderr,"aroff: file (%s) is not an AWGS file\n",file);
        exit(1);
    }
    p.pCount = 3; p.refNum = ref; p.base = startPlus; p.displacement = 668l;
    SetMarkGS(&p); fileError("SetMarkGS");

    i.pCount = 4;
    i.refNum = ref;
    i.dataBuffer = (void *) &docSACount;
    i.requestCount = 2l;
    ReadGS(&i); fileError("ReadGS (docSACount)");

#ifdef DEBUG
fprintf(stderr,"Number of SaveArray entries: %d\n",docSACount);
#endif

    docSaveArray = calloc((size_t) docSACount, sizeof(saveArray));
    i.dataBuffer = (void *) docSaveArray;
    i.requestCount = sizeof(saveArray) * docSACount;
    ReadGS(&i); fileError("ReadGS (docSaveArray)");

#ifdef DEBUG
fprintf(stderr,"  saNum  textBlock  rulerNum\n");
fprintf(stderr,"  -----  ---------  --------\n");
for (z = 0; z < docSACount; z++) {
  fprintf(stderr,"  [%3d]      %5d     %5d\n",
    z+1, docSaveArray[z].textBlock,docSaveArray[z].rulerNum);
}
#endif

    numRulers = numBlocks = 0;
    for (z = 0; z < docSACount; z++) {
        if (docSaveArray[z].rulerNum+1 > numRulers)
            numRulers = docSaveArray[z].rulerNum+1;
        if (docSaveArray[z].textBlock+1 > numBlocks)
            numBlocks = docSaveArray[z].textBlock+1;
    }
#ifdef DEBUG
fprintf(stderr,"Number of Rulers: %d\n",numRulers);
fprintf(stderr,"Number of Blocks: %d\n",numBlocks);
#endif

    docRulers = calloc((size_t) numRulers, sizeof(Ruler));
    i.dataBuffer = (void *) docRulers;
    i.requestCount = sizeof(Ruler) * numRulers;
    ReadGS(&i); fileError("ReadGS (docRulers)");

    docTextBlocks = calloc((size_t) numBlocks, sizeof(textBlockPtr));
    for (z = 0; z < numBlocks; z++) {
        i.requestCount = 4l;
        i.dataBuffer = (void *) &recBlockSize;
        ReadGS(&i); fileError("ReadGS (recBlockSize)");

#ifdef DEBUG
fprintf(stderr,"block %d size %8ld : ",z,recBlockSize);
#endif
        docTextBlocks[z] = malloc(recBlockSize);
        i.requestCount = recBlockSize;
        i.dataBuffer = (void *) docTextBlocks[z];
        ReadGS(&i); fileError("ReadGS (textBlock)");
    }

    cl[0] = 1;
    cl[1] = ref;
    CloseGS(cl);
}

void usage(void)
{
    fprintf(stderr,"aroff [-b] file1 [file ...]\n"
    "-b  don't do any boldfacing (useful for GNO Ref. Manuals)\n");
    exit(1);
}

int main(int argc, char *argv[])
{
int i,z;
extern void printAWGS(void);
extern int _INITGNOSTDIO();

    _INITGNOSTDIO();
    
    if (argc == 1) usage();
    for (i = 1; i < argc; i++) {
        if (*argv[i] == '-') {
            if (argv[i][1] == 'b')
                { noboldflag = 1; continue; }
	    else usage();
        }
        readAWGS(argv[i]);
        printAWGS();
        free(docSaveArray);
        free(docRulers);
        for (z = 0; z < numBlocks; z++)
            free(docTextBlocks[z]);
        free(docTextBlocks);
    }
}
