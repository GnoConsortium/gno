#pragma optimize -1
#pragma stacksize 1024

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gsos.h>
#include <fcntl.h>
#include <errno.h>

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

char *segTypes[] = {
"Code",
"Data",
"Jump-table",
"Unknown",
"Pathname",
"Unknown",
"Unknown",
"Unknown",
"Library Dictionary",
"Unknown",
"Unknown",
"Unknown",
"Unknown",
"Unknown",
"Unknown",
"Unknown",
"Initialization",
"Unknown",
"Direct-page/Stack"};

void prntAscii(char *s, int l)
{
    while (l--) {
	*s &= 0x7F;
	if (*s < ' ') {
	    putchar('^');
            putchar(*s+'@');
	} else putchar(*s);
        s++;
    }	
}

char name[256];
/*
 *  returns either 0 (no stack segment found) or the size of the stack
 *  segment contained in the application
 *  The caller is responsible for opening the file, passing it's refNum,
 *  and closing it after we're done
 */
word scanOMF(int fd)
{
OMFhead header;
longword off = 0l;
PositionRecGS p;
int kind;
int stsize = 0;

    p.pCount = 2;
    p.refNum = fd;
    GetEOF(&p);

    while (off < p.position) {
        /* printf("offset: %ld\t\t",off); */
        putchar('\t');
        lseek(fd, off, 0);
        read(fd,&header,sizeof(header));
        lseek(fd, off+header.DISPNAME+10, 0);
        if (header.LABLEN == 0) {
            read(fd, name, 1);
            read(fd, name+1, name[0]);
        } else {
            name[0] = header.LABLEN;
            read(fd, name+1, header.LABLEN);
        }
        putchar('"');
        prntAscii(name+1,name[0]);
        putchar('"');
        kind = header.KIND & 0x1F;
        if (kind < 0x13)
            printf("\t%s segment (%02X) %08lX bytes\n",segTypes[kind],kind,
            header.LENGTH);
        if (kind == 0x12)   /* got a stack segment */
	    stsize = (word) header.LENGTH;
        off += header.BYTECNT;
    }
    return (stsize ? stsize : 4096); /* the default stack size for programs */
}

void usage(void)
{
    fprintf(stderr,"usage: lseg filename...\n");
    exit(1);
}

int main(int argc, char *argv[])
{
int fd;

    if (argc == 1) usage();
    argv++; argc--;
    while (argc-- > 0) {
        fd = open(*argv,O_RDONLY);
	if (fd < 0) {
	    fprintf(stderr,"lseg: %s: %s\n",*argv, strerror(errno));
            exit(1);
        }
        printf("Executable: %s\n",*(argv++));
        printf("\tStack size: %d\n",scanOMF(fd));
        close(fd);
    }
}
