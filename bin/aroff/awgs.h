/*  
     This file contains the data structures that are
     used in AWGS Word Processor files.

     Data structures gleaned from DTS File Type Note TN.50.8010
*/

/* #define DEBUG */

typedef struct pgraph {
    word      firstFont;
    byte      firstStyle;
    byte      firstSize;
    byte      firstColor;
    word      reserved;
} pgraph, *pgraphPtr;

typedef struct textBlock {
    word      blockSize;
    word      blockUsed;
    pgraphPtr pgraphs;
} textBlock, *textBlockPtr;

typedef struct tabRec {
    word      tabLocation;
    word      tabType;
} tabRec, *tabRecPtr;

#define rsFULL 0x80
#define rsRIGHT 0x40
#define rsCENTER 0x20
#define rsLEFT 0x10
#define rsNOBREAK 0x08
#define rsTRIPLE 0x04
#define rsDOUBLE 0x02
#define rsSINGLE 0x01

typedef struct Ruler {
    word      numParagraphs;
    word      statusBits;
    word      leftMargin;
    word      indentMargin;
    word      rightMargin;
    word      numTabs;
    tabRec    tabRecs[10];
} Ruler, *RulerPtr;

typedef struct SaveArrEntry {
    word      textBlock;          /* Text block number */
    word      offset;             /* offset + text block = paragraph */
    word      attributes;         /* 0 = normal text, 1 = page break paragrf */
    word      rulerNum;           /* #of ruler associated with this paragrf */
    word      pixelHeight;        /* height of paragraph in pixels */
    word      numLines;           /* # of lines in this paragraph */
} saveArray, *saveArrayPtr;

extern int noboldflag;
