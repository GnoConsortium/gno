/*
 * lseg - list segments of an OMF file
 *            
 *   Version 1.0 written by Jawaid Bazyar for GNO/ME 1.0 (June 1992)
 *
 *   Version 1.1 [v1.1] updated by Dave Tribby (Sept. 1997)
 *     - A few changes for GNO/ME 2.0.6
 *     - Added display of allocated stack bytes for code segments
 *     - Reformatted output to use a table format and make room for new info
 *     - Sanity check to see whether file is an OMF file
 *     - Use first field as block count for OMF version 1 files
 *     - Continue processing files even if one of them cannot be opened
 *     - Use standard error reporting interfaces
 *     - Print values in hex unless -d (decimal) flag is set
 *
 * $Id: lseg.c,v 1.3 1997/09/28 16:41:13 gdr Exp $
 */

/* Update for 2.0.6: Move optimization and stack size to Makefile
/* #pragma optimize -1
/* #pragma stacksize 1024
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gsos.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>	/* GNO 2.0.6: read() and close() moved to here */
#include <err.h>	/* GNO 2.0.6: use standard error reporting */

/* [v1.1] Use library routine rather than macro, so that program  */
/*        code is less complicated and can be optimized by ORCA/C */
#undef putchar

typedef struct OMFhead {
    longword BYTECNT;
    longword RESSPC;
    longword LENGTH;
    byte undefined;
    byte LABLEN;
    byte NUMLEN;
    byte VERSION;
    longword BANKSIZE;
    word KIND;
    word undefined2;
    longword ORG;
    longword ALIGN;
    byte NUMSEX;
    byte undefined3;
    word SEGNUM;
    longword ENTRY;
    word DISPNAME;
    word DISPDATA;
    longword tempOrg;
} OMFhead;

char *segTypes[] = {   /* [v1.1] Omit "unknown" for undefined segment types */
"Code              ",
"Data              ",
"Jump-table        ",
"",
"Pathname          ",
"",
"",
"",
"Library Dictionary",
"",
"",
"",
"",
"",
"",
"",
"Initialization    ",
"",
"Direct-page/Stack "};


/* --- Start of new code [v1.1] --- */

/* Option for decimal rather than hex output */
int decimal_output=FALSE;

/* Snippit of code to be analyzed for allocated stack size */
static char code[8];

/* Read a segment-body record; return the following values:       */
/*   0 if stack allocation code not found, but expected to follow */
/*   -1 if recognized as leading to stack allocation              */
/*   >0 bytes of stack allocation                                 */
int readSegRec(int fd)
{
   char rec_type;
   char *cp=code;
   int num_bytes;
   /* Normal preamble code includes phd, which uses 2 bytes */
   int base_size=2;

   /* Read record type (opcode) */
   read(fd,&rec_type,1);

   /* Is it executable code? */
   if (rec_type > 0  && rec_type < 0xDF)   {
      /* Determine size to read: min of rec_type and size of code array */
      if (rec_type > sizeof(code))
         num_bytes = sizeof(code);
      else
         num_bytes = rec_type;
      read(fd,code,num_bytes);

      /* Is it the code for no local variables: pha ; tsc ; phd ; tcd ? */
      if (rec_type > 2) {
         /* NOTE: pha is included only when code is not optimized */
         if ( *(cp) == 0x48 )   {
            /* pha uses 2 additional bytes of stack */
            base_size = 4;
            cp++;
         }
         if ( *(cp++) == 0x3B && *(cp++) == 0x0B && *cp == 0x5B )
            return base_size;  /* Yes; return base size */
         /* No; reset pointer and base size before checking further */
         cp = code;
         base_size = 2;
      }                  

      /* Is this the short preamble code: pea $xxxx ; jsl ~CHECKSTACK ? */
      if (rec_type == 4) {
         if ( *(cp) == 0xF4 && code[3] == 0x22)
            return 0;  /* Expect allocation later */
         else
            return -1;  /* Give up */
      }      

      /* Does it start with tsc ; sec ; sbc #x ? */
      if (rec_type > 7)   {
         if ( *(cp++) == 0x3B && *(cp++) == 0x38 && *(cp++) == 0xE9 )
            /* This is the preamble we're looking for!    */
            /* Next two bytes are size of local variables */
            return  *(int*)cp + base_size;
         else
            return -1;  /* Give up */
      }
   }  /* End of code cases */

   /* Is it an LEXPR record? (expected as part of jsl ~CHECKSTACK) */
   if (rec_type == 0xF3)   {
      /* Read the replacement length, op type, string length */
      read(fd,code,3);
      if ( *(cp++) == 0x03 && *(cp++) == 0x83 )   {
         /* Position file beyond the expression string */
         num_bytes = *cp;
         lseek(fd, num_bytes, SEEK_CUR);
         /* Next character should be null byte following string */
         read(fd,code,1);
         if (*code == 0)
            return 0;  /* Expect allocation later */
      }
   }
   return -1;  /* Give up */
}


/* Check code segments for dynamic startup stack allocation */
void checkCodeStack(int fd)
{
   int value;

   while ( (value = readSegRec(fd)) == 0 );
   if (value > 0)
       if (decimal_output)
           printf("%6d ", value);
       else
           printf("0x%04X ", value);
   else
       printf("       ");
}

/* Is the file under consideration not an OMF file? */
int notOMF(OMFhead *headPnt)
{
   if (headPnt->NUMLEN != 4
    || headPnt->VERSION > 2
    || headPnt->BANKSIZE > 0x010000
    || headPnt->NUMSEX != 0
    || headPnt->DISPNAME > headPnt->DISPDATA)  return TRUE;
   return FALSE;
}

/* --- End of new routines [v1.1] --- */


void prntAscii(char *s, int l)
{
    putchar('"');
    while (l--) {
	*s &= 0x7F;
	if (*s < ' ') {
	    putchar('^');
            putchar(*s+'@');
	} else putchar(*s);
        s++;
    }	
    putchar('"');
}

char name[256];
/*
 *  The caller is responsible for opening the file, passing its refNum,
 *  and closing it after we're done
 */
void scanOMF(int fd, char *fname)
{
OMFhead header;
longword off = 0l;
PositionRecGS p;
int kind;
int i;

    p.pCount = 2;
    p.refNum = fd;
    GetEOF(&p);

    while (off < p.position) {
        lseek(fd, off, SEEK_SET);
        read(fd,&header,sizeof(header));
        /* First time through, check validity of OMF header [v1.1] */
        if (off == 0  &&  notOMF(&header))   {
           printf("Note: %s is not a recognized OMF file\n", fname);
           return;
        }
        lseek(fd, off+header.DISPNAME+10, SEEK_SET);
        if (header.LABLEN == 0) {
            read(fd, name, 1);
            read(fd, name+1, name[0]);
        } else {
            name[0] = header.LABLEN;
            read(fd, name+1, header.LABLEN);
        }
        printf("%s",fname);
        i = strlen(fname);
        while (i++ < 20) putchar(' ');

        kind = header.KIND & 0x1F;
        switch (kind)   {
            case 0x00:
                if (decimal_output)
                   printf(" %s %8ld ", segTypes[kind],header.LENGTH);
                else
                   printf(" %s 0x%06lX ", segTypes[kind],header.LENGTH);
                /* Check code segment for stack allocation [v1.1] */
                /* Position to beginning of data */
                lseek(fd, off+header.DISPDATA, SEEK_SET);
                /* Check the code */
                checkCodeStack(fd);
                break;
            case 0x01:
            case 0x02:
            case 0x04:
            case 0x08:
            case 0x10:
            case 0x12:
                if (decimal_output)
                   printf(" %s %8ld        ", segTypes[kind],header.LENGTH);
                else
                   printf(" %s 0x%06lX        ", segTypes[kind],header.LENGTH);
                break;
            default:
                printf(" unknown (0x%02X)     ", kind);
            }
        prntAscii(name+1,name[0]);
        putchar('\n');
        /* In OMF version 1, the first field is a block count */
        if (header.VERSION == 1)
           off += (header.BYTECNT * 512);
        else
           off += header.BYTECNT;
        /* Check for unusual case that causes infinite loop [v1.1] */
        if (header.BYTECNT == 0) break;
    }
}

void usage(void)
{
    fprintf(stderr,"usage: lseg [-d] filename...\n");
    exit(1);
}


/* [v1.1] Check on how much stack space a C program uses. */
#if defined(__STACK_CHECK__)
#ifndef _GNO_GNO_H_
#include <gno/gno.h>
#endif
static void report_stack(void)
{
	fprintf(stderr,"\n ==> %d stack bytes used <== \n", _endStackCheck());
}
#endif


int main(int argc, char *argv[])
{
int fd;
int exit_status=0;
int ch;

#if defined(__STACK_CHECK__)
    _beginStackCheck();
    atexit(report_stack);
#endif

    /* Get option, if present */
    while ((ch = getopt(argc, argv, "d")) != EOF) {
        switch(ch) {
            case 'd':
                decimal_output = TRUE;
                break;
            default:
                usage();
            }
        }
    argc -= optind;
    argv = argv + optind;

    /* Make sure that at least one filename was provided */
    if (argc < 1) usage();

    /* Print header [v1.1] */
printf(
"File                 Type               Size     Stack  Name\n");
printf(
"-------------------- ------------------ -------- ------ ----------------\n");
    
    while (argc-- > 0) {
	if ((fd = open(*argv, O_RDONLY)) < 0) {
            exit_status = 1;
	    warn("%s", *argv);
        } else {
            scanOMF(fd,*argv);
            close(fd);
        }
        ++argv;
    }       
return exit_status;
}
