/*
    print.c

    the code that formats each individual paragraph is here.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <texttool.h>

#include "awgs.h"

extern saveArray *docSaveArray;
extern Ruler *docRulers;
extern textBlock **docTextBlocks; /* an array of textBlockPtrs */
extern word docSACount, numRulers, numBlocks;

/*
          1          2
01234567890123456 7890
This is a bloody ^car
z = 16;

This is a bloody
^car

col=4     (width - z - 1)
printcol=3
*/

/* 100 lines should be more than enough */
char paraBuf[100][80];


/* given a paragraph, and a pointer to it's ruler, format the
   paragraph according to the ruler. */
void printPara(RulerPtr ruler, pgraphPtr pptr)
{
char *txt;
int width,z;
int curLine, col, printcol, numctrl;
int i,left,style;

    curLine = col = printcol = 0;
    txt = ((char *)pptr) + sizeof(pgraph);

calcLine:
    /* width determines how long this line is in characters; thus, where
       the break for word wrap will occur */
    if (curLine == 0) width = (ruler->rightMargin - ruler->indentMargin)/8;
    else width = (ruler->rightMargin - ruler->leftMargin)/8;
    
    while (*txt != 0x0d) {
      switch (*txt) {
          case 1: txt+=3; break;
          case 2:
                  style = *(++txt);
                  if (noboldflag) { ++txt; break; }/* turn off boldfacing */
                  if (style & 3) paraBuf[curLine][col++] = 15;
                  else paraBuf[curLine][col++] = 14;
                  txt++;
                  break;
          case 3: txt+=2; break;
          case 4: txt+=2; break;
          case 5:
          case 6:
          case 7: break;

          default:
            if (printcol == width) {
              numctrl = 0;
              for (z = col - 1; z > 0; z--) {
                  if (paraBuf[curLine][z] == ' ') {
                      if (z != col - 1)
                          memcpy(&paraBuf[curLine+1][0],&paraBuf[curLine][z+1],
                             (size_t) (col - z - 1));
                      paraBuf[curLine][z] = 0;
                      curLine++; printcol -= (z + 1 + numctrl);
                      col -= (z + 1);
                      goto calcLine;
                  }
                  else if (paraBuf[curLine][z] < ' ') numctrl++;
              }
              curLine++; col = printcol = 0;
              /* one big word... don't break line */
              goto calcLine;
            }
            paraBuf[curLine][col] = *(txt++);
            printcol++; col++;
      }
    }
    paraBuf[curLine][col] = 0;
    for (z = 0; z <= curLine; z++) {
      if (z == 0) {
          width = (ruler->rightMargin - ruler->indentMargin)/8;
          left = (ruler->indentMargin)/8;
      }
      else {
          width = (ruler->rightMargin - ruler->leftMargin)/8;
          left = (ruler->leftMargin)/8;
      }
      for (i = 0; i < left; i++) putchar(' ');
      printf("%s\n",paraBuf[z]);
    }
}

/* this is an obsolete routine that prints a paragraph with no
   formatting at all */
#ifdef NOTDEFINED
void printPara(RulerPtr ruler, pgraphPtr pptr)
{
char *txt;

        txt = ((char *)pptr) + sizeof(pgraph);
        while (*txt != 0x0D) {
            switch (*txt) {
                case 1: txt+=2; break;
                case 2: txt++; break;
                case 3: txt++; break;
                case 4: txt++; break;
                case 5:
                case 6:
                case 7: break;

                default: putchar(*txt);
            }
            txt++;
        }
        putchar('\n');
}
#endif


/* go through each textBlock, sending each paragraph in turn to printPara. */

void printAWGS(void)
{
int z;
pgraphPtr pptr;
char *txt;
char x;

    for (z = 0; z < docSACount; z++) {
        pptr = (pgraphPtr) (((byte *)docTextBlocks[docSaveArray[z].textBlock])
               + docSaveArray[z].offset);
    
#ifdef DEBUG
fprintf(stderr,"[%d] offset %d  paragraph : %08lX",z,docSaveArray[z].offset,
  pptr);
fprintf(stderr," textBlock: %08lX\n",docTextBlocks[docSaveArray[z].textBlock]);
#endif
        printPara(&docRulers[docSaveArray[z].rulerNum],pptr);
    }
}
