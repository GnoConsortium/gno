/*
 *	ls
 *
 *	version 2.0
 *
 *	Rewritten for speed, memory usage, and reliability
 *
 *	now checks TERM var and displays MouseText folder only if set to gnocon
 *	does not apply name quicksort if files are on an HFS volume
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <gsos.h>
#include <unistd.h>
#include <misctool.h>

#pragma stacksize 1024
#pragma optimize -1
/*#pragma optimize 8*/

#define CALCULATE -1
#define ONLYONE -2

unsigned fl_all,fl_reverse;
unsigned fl_gnocon;
unsigned fl_sortname,fl_sortcreate,fl_sortmod;
unsigned firstflag;

struct directory {
    ResultBuf32Ptr dir_name;
    struct directory *parent;
    unsigned fake;
    word num_entries;
    longword num_bytes;
    word no_sort;
    unsigned width;
    DirEntryRecPtrGS *entry_ptrs;
    DirEntryRecPtrGS entries;
};

DirEntryRecGS dirinfo;
ResultBuf32 nameb;

lsexit(int x_code)
{
    SYSTEMQUITFLAGS(0x4000);
    SYSTEMQUITPATH(NULL);
    exit(x_code);
}
/*#define lsexit(x) exit(x)*/

#define qsort(a,b,c,d) nqsort(a,b,c,d)
extern void SortList(void *a, int c, int b, void *d);
void nqsort(void *,unsigned, unsigned,int (*cmp)(void *,void *));

void printGS(GSString255Ptr g)
{
    fwrite(g->text,g->length,1,stdout);
}

typedef struct listStruct *list;
const char month[12][4] =
  { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };

typedef struct FileTypeConv {
   word type;
   char rep[10];
} FileTypeConv;

FileTypeConv FTTable[] = {
        0x0f, "DIR ", /* directory */
        0xB0, "src ", /* apw source file */
        0xb1, "obj ", /* apw object file */
        0x04, "txt ", /* ascii text file */
        0xb3, "s16 ", /* gs/os program file */
        0xb5, "exe ", /* gs/os shell program file */
        0xb8, "nda ",
        0xb9, "cda ",
        0xba, "tol ",
        0x00, "non ", /* typeless file */
        0x01, "bad ", /* bad block file */
        0x06, "bin ", /* general binary file */
        0x08, "fot ", /* graphics screen file */
        0x19, "adb ", /* appleworks data base file */
        0x1a, "awp ", /* appleworks word processor file */
        0x1b, "asp ", /* appleworks spreadsheet */
        0xb2, "lib ", /* apw library file */
        0xb4, "rtl ", /* apw runtime library */
        0xef, "pas ", /* pascal partition */
        0xf0, "cmd ",
        0xfa, "int ", /* integer basic program */
        0xfb, "var ", /* int basic var file */
        0xfc, "bas ", /* applesloth basic file */
        0xfd, "var ", /* applesloth variable file */
        0xfe, "rel ", /* EDASM relocatable code */
        0xff, "sys ", /* prodos 8 system program file */
        -1, ""};

char conv[10];
char *getFileType(int type)
{
int i;

    i = 0;
    while (FTTable[i].type != -1)
    {
        if (FTTable[i].type == type)
        {
            return FTTable[i].rep;
        }
        else i++;
    }
    sprintf(conv,"$%2X ",type);
    return conv;
}

word curYear;

void *sortRoutine;
int columns;
int openDirectory, fl_longOutput, inK;
int dirOnly, idType, fl_recursive, fl_nosort;
int more,less;
unsigned whichTime;

void printDirName(struct directory *d, unsigned level)
{
    if (d == NULL) return;
    else {
        printDirName(d->parent,level+1);
        if (d->dir_name != NULL) {
            printGS((GSString255Ptr)&d->dir_name->bufString);
            if (level) putchar('/');
        }
    }
    if (!level) putchar(':');
}

void long_out(DirEntryRecGS *entry)
{
TimeRec *time;

    if (whichTime) time = &entry->createDateTime;
    else time = &entry->modDateTime;

    printf("%c%c%c%c%c%c%c %04lX %4s %8ld",
          (entry->fileType == 0x0f) ? 'd' :
            ((entry->flags & 0x8000) ? 'e' : '-'),
          (entry->access & 0x01) ? 'r' : '-',
          (entry->access & 0x02) ? 'w' : '-',
          ((entry->fileType == 0xff) || (entry->fileType == 0xb5) ||
            (entry->fileType == 0xb3) ||
            ((entry->fileType == 0xb0) && (entry->auxType == 6l)))
            ? 'x' : '-',
          (entry->access & 0x20) ? 'b' : '-',
          (entry->access & 0x40) ? 'r' : '-',
          (entry->access & 0x80) ? 'd' : '-',
          entry->auxType,
          getFileType(entry->fileType),
          entry->eof+entry->resourceEOF);
    if (time->year+1900 == curYear)
        printf(" %3s %2d %02d:%02d ",
          month[time->month],time->day+1,time->hour,time->minute);
    else printf(" %3s %2d  %4d ",month[time->month],time->day+1,
          time->year+1900);
    /*puts(entry->name->bufString.text);*/
}            

/* fake a directory read for command-line arguments */
struct directory *fakeDirect(int argc, char **argv,struct directory *par)
{
struct directory *new;
unsigned i,file_count = 0,maxWidth = 0;
unsigned err;
DirEntryRecPtrGS newents,nptr;
FileInfoRecGS fi;
ResultBuf255Ptr newn;
extern GSString255Ptr __C2GSMALLOC(char *);

    new = malloc(sizeof(struct directory));
    new->num_entries = argc;
    new->num_bytes = 0l;
    new->parent = par;
    new->dir_name = NULL;
    new->no_sort = 0;
    new->fake = 1;
    if ((argc * sizeof(DirEntryRecGS)) > 60000)
        printf("exceeded ORCA pointer math bug range\n");
    newents = new->entries = malloc(argc * sizeof(DirEntryRecGS));
    new->entry_ptrs = malloc(argc * sizeof(DirEntryRecGS *));
    fi.pCount = 12;
    fi.optionList = 0l;

    nptr = newents;
    for (i = 0; i < argc; i++) {
        new->entry_ptrs[file_count] = nptr;
        nptr->pCount = 17;
        nptr->optionList = NULL;

        fi.pathname = __C2GSMALLOC(argv[i]);
        GetFileInfoGS(&fi);
        if (err=_toolErr) {
	    fprintf(stderr,"ls: %s: %s\n",strerror(_mapErr(err)),argv[i]);
	    lsexit(1);
        }
        nptr->name = malloc(fi.pathname->length+5);
        nptr->name->bufSize = fi.pathname->length+5;
        memcpy(&nptr->name->bufString,fi.pathname, fi.pathname->length+2);
        nptr->name->bufString.text[fi.pathname->length] = 0;
	free(fi.pathname);

            if (fi.storageType == 0x05) /* extended file? */
            	nptr->flags = 0x8000;
	    else nptr->flags = 0;
            nptr->fileType = fi.fileType;
            /* this is used by the qsort routine to maintain
               command-line ordering for directories */
            if (fi.fileType == 0x0F) nptr->entryNum = i;
            nptr->auxType = fi.auxType;
            nptr->eof = fi.eof;
            nptr->blockCount = fi.blocksUsed;
            nptr->createDateTime = fi.createDateTime;
            nptr->modDateTime = fi.modDateTime;
            nptr->access = fi.access;
            nptr->resourceEOF = fi.resourceEOF;
            nptr->resourceBlocks = fi.resourceBlocks;

            file_count++;
            new->num_bytes += (nptr->eof + nptr->resourceEOF);
            if (nptr->name->bufString.length > maxWidth)
            	maxWidth = nptr->name->bufString.length;
            /* NOTHING AFTER THIS LINE, PLEASE */
            nptr = (DirEntryRecPtrGS) ((unsigned long) nptr + sizeof(DirEntryRecGS));
    }
    new->width = maxWidth;
    new->num_entries = file_count;
    return new;
}

struct directory *readDirect(int fd,struct directory *par, ResultBuf32Ptr n)
{
struct directory *new;
unsigned i,file_count = 0,maxWidth = 0;
DirEntryRecPtrGS newents,nptr;
ResultBuf255Ptr newn;

    nameb.bufSize = 36;

    dirinfo.pCount = 14;
    dirinfo.refNum = fd;
    dirinfo.base = dirinfo.displacement = 0;
    dirinfo.name = (ResultBuf255Ptr) &nameb;
    GetDirEntry(&dirinfo);
    new = malloc(sizeof(struct directory));
    new->num_entries = dirinfo.entryNum;
    new->num_bytes = 0l;
    new->parent = par;
    new->dir_name = n;
    new->fake = 0;
    if (dirinfo.fileSysID == hfsFSID) new->no_sort = 1;
    else new->no_sort = 0;
    if ((dirinfo.entryNum * sizeof(DirEntryRecGS)) > 60000)
        printf("exceeded ORCA pointer math bug range\n");
    newents = new->entries = malloc(dirinfo.entryNum * sizeof(DirEntryRecGS));
    new->entry_ptrs = malloc(dirinfo.entryNum * sizeof(DirEntryRecGS *));

    nptr = newents;
    for (i = 0; i < new->num_entries; i++) {
        new->entry_ptrs[file_count] = nptr;
        nptr->pCount = 17;
        nptr->refNum = fd;
        nptr->base = nptr->displacement = 1;
        nptr->name = (ResultBuf255Ptr) &nameb;
        nptr->optionList = NULL;
        GetDirEntry(nptr);
        if ((fl_all) || (!(nptr->access & fileInvisible))) {
            file_count++;
            newn = malloc(37);
            nameb.bufString.text[nameb.bufString.length] = 0;
            memcpy(newn,&nameb,36);
            nptr->name = newn;
            new->num_bytes += (nptr->eof + nptr->resourceEOF);
            if (nptr->name->bufString.length > maxWidth)
            	maxWidth = nptr->name->bufString.length;
            /* NOTHING AFTER THIS LINE, PLEASE */
            nptr = (DirEntryRecPtrGS) ((unsigned long) nptr + sizeof(DirEntryRecGS));
        }
    }
    new->width = maxWidth;
    new->num_entries = file_count;
    return new;
}

int sortByType(DirEntryRecGS **a,DirEntryRecGS **b)
{
unsigned ea,eb;

    if ((*a)->fileType == 0x0F) {
        if ((*b)->fileType == 0x0F) {
            /* if both are dirs, maintain command-line ordering */
            ea = (*a)->entryNum;
            eb = (*b)->entryNum;
            if (ea > eb) return 1;
            else if (ea == eb) return 0;
            else return -1;
        }
        else return 1;
    } else {
        if ((*b)->fileType == 0x0F) return -1;
    }
}

int sortByName(DirEntryRecGS **a,DirEntryRecGS **b)
{
    return strcmp((*a)->name->bufString.text,(*b)->name->bufString.text) * more;
}

int sortByCreate(DirEntryRecGS **a,DirEntryRecGS **b)
{
TimeRec *time1, *time2;

    time1 = &((*a)->createDateTime);
    time2 = &((*b)->createDateTime);
    if (time1->year > time2->year) return less;
    if (time1->year < time2->year) return more;
    if (time1->month > time2->month) return less;
    if (time1->month < time2->month) return more;
    if (time1->day > time2->day) return less;
    if (time1->day < time2->day) return more;
    if (time1->hour > time2->hour) return less;
    if (time1->hour < time2->hour) return more;
    if (time1->minute > time2->minute) return less;
    if (time1->minute < time2->minute) return more;
    if (time1->second > time2->second) return less;
    if (time1->second < time2->second) return more;
    return 0;
}

int sortByMod(DirEntryRecGS **a,DirEntryRecGS **b)
{
TimeRec *time1, *time2;

    time1 = &((*a)->modDateTime);
    time2 = &((*b)->modDateTime);
    if (time1->year > time2->year) return less;
    if (time1->year < time2->year) return more;
    if (time1->month > time2->month) return less;
    if (time1->month < time2->month) return more;
    if (time1->day > time2->day) return less;
    if (time1->day < time2->day) return more;
    if (time1->hour > time2->hour) return less;
    if (time1->hour < time2->hour) return more;
    if (time1->minute > time2->minute) return less;
    if (time1->minute < time2->minute) return more;
    if (time1->second > time2->second) return less;
    if (time1->second < time2->second) return more;
    return 0;
}

void listDir(struct directory *d)
{
unsigned i;
unsigned dirflag;
struct directory *dd;
int fd;
int Col,Row,Height;
int j, afile = FALSE, numColumns = 1;
unsigned rWidth,width;
unsigned fakeind;
unsigned num_entries;
char *cwd;
DirEntryRecGS *entry;

    num_entries = d->num_entries;
    if ((fl_longOutput) && (!d->fake))
    	printf("total %ldk\n",(d->num_bytes / 1024l));
    if (!num_entries) return;
    if (d->fake && openDirectory) {
        /* sort by type first */
        qsort(d->entry_ptrs, num_entries, sizeof(DirEntryRecGS *), sortByType);
        num_entries = 0;
        while ((num_entries < d->num_entries) &&
               (d->entry_ptrs[num_entries]->fileType != 0x0F))
               num_entries++;
        /* finally, sort the non-directory files the way they wanted */
        if (num_entries)
            qsort(d->entry_ptrs, num_entries, sizeof(DirEntryRecGS *), sortRoutine);
    } else
    if ((!fl_nosort) && (!(d->no_sort && fl_sortname)))
    	qsort(d->entry_ptrs, num_entries, sizeof(DirEntryRecGS *),sortRoutine);
    if (!fl_longOutput && columns == CALCULATE) {
        rWidth = d->width;
        width = rWidth;
        if (inK) width+=5;
        if (idType) width++;
        if (width < 15) {
            rWidth = 15 - (width - rWidth);
            width = 15;
        } /* for jawaid :-) */
        numColumns = 80 / (++width);
    }
    Height = (num_entries / numColumns);
    if (num_entries % numColumns)  Height++;
    for (Row=0; Row < Height; Row ++) {
    	for (Col = 0; Col < numColumns; Col ++) {
            i = Col * Height + Row;
            if (i >= num_entries) continue;

            entry = d->entry_ptrs[i];
            if (entry->fileType == 0x0F) dirflag = TRUE;
            /*if (mult && entry->fileType == 0x0f) continue;*/
            afile = TRUE;
            if (inK) printf("%4ld ",((entry->eof+entry->resourceEOF)/1024)+1);
            /* time = 0 means do mod date, = 1 means do create date */
            /*time = (sortRoutine == CompareCreate) ? 1 : 0;*/
            if (fl_longOutput) long_out(d->entry_ptrs[i]);
            printGS(&entry->name->bufString);
            if (idType) {
            	if (entry->fileType == 0x0f) putchar('/');
            	else if ((entry->fileType == 0xff) || (entry->fileType == 0xb5) ||
              	    (entry->fileType == 0xb3) ||
              	    ((entry->fileType == 0xb0) && (entry->auxType == 6l)))
              	    putchar('*');
            	else putchar(' ');
            }
            if (Col+1 < numColumns) {
                for (j=0;j<(rWidth-(int)(entry->name->bufString.length));j++)
                    putchar(' ');
                putchar(' ');
            }
    	}
        putchar('\n');
    }

    if ((dirflag && fl_recursive) || (d->fake && openDirectory)) {
    unsigned start_ent = (d->fake ? num_entries : 0);

        /* if there were regular files in a command-line list,
           put a blank line in there but don't count as first */
        if (d->fake && (num_entries != 0)) {
            putchar('\n');
        }
        for (i = start_ent; i < d->num_entries; i++) {
	    if (d->entry_ptrs[i]->fileType == 0x0F) {
            ResultBuf32Ptr p;

                p = (ResultBuf32Ptr) d->entry_ptrs[i]->name;
                /* let the danged kernel do some of the work! */
                if (fl_recursive) {
                    cwd = malloc(1024l);
                    getwd(cwd);
                    chdir(p->bufString.text);
                    fd = open(".",O_RDONLY);
                }
                else fd = open(p->bufString.text,O_RDONLY);
                if (fd <= 0) {
                    perror("could not open");
                    exit(1);
                }
                dd = readDirect(fd,d,p);
    		close(fd);
                /* print the directory name for recursive listings */
                if (!firstflag) firstflag = 1;
                else putchar('\n');
                printDirName(dd,0);
                putchar('\n');
    		listDir(dd);
                if (fl_recursive) {
                    chdir(cwd);
                    free(cwd);
                }
		/* DISPOSE of 'dd' at this point */
            }
        }
    }
}

int main(int argc, char **argv)
{
int fd;
struct directory *dd;
TimeRec curtime;
int ch;
extern int getopt_restart(void);

    getopt_restart();
    curtime = ReadTimeHex();
    curYear = curtime.year + 1900;

    if (isatty(STDOUT_FILENO) && (!strncmp(ttyname(STDOUT_FILENO),".ttyco",6)))
	strcpy(FTTable[0].rep,"\x1B\xFXY\xE\x18  ");
    else strcpy(FTTable[0].rep,"DIR "); /* we restart, remember? */

    /* initialize our restartable global variables */
    fl_nosort = 0; fl_reverse = 0;
    more = 1; less = -1;
    sortRoutine = sortByName;
    columns = CALCULATE;
    openDirectory = TRUE;
    fl_all = FALSE;
    fl_longOutput = FALSE;
    inK = FALSE;
    dirOnly = FALSE;
    idType = FALSE;
    fl_recursive = FALSE;
    whichTime = 0;
    firstflag = 0;

    while ((ch = getopt(argc, argv, "acdflnqrst1CFR")) != EOF) {
        switch(ch) {
            case 'n' :
                fl_nosort = TRUE;
                break;
            case 'r' :
                more = -1; less = 1;
                break;
            case 't' :
                sortRoutine = sortByMod;
                whichTime = 0;
                break;
            case 'c' :
                sortRoutine = sortByCreate;
                whichTime = 1;
                break;
            case '1' :
                columns = ONLYONE;
                break;
            case 'C' :
                columns = CALCULATE;
                break;
            case 'a' :
                fl_all = TRUE;;
                break;
            case 'l' :
                fl_longOutput = TRUE;
                break;
            case 's' :
                inK = TRUE;
                break;
            case 'f' :
                dirOnly = TRUE;
                fl_longOutput = FALSE;
                inK = FALSE;
                fl_all = TRUE;
                break;
            case 'q' :
                break;
            case 'F' :
                idType = TRUE;
                break;
            case 'R' :
                fl_recursive = TRUE;
                break;
            case 'd' :
                openDirectory = FALSE;
                fl_recursive = FALSE;
                break;
            default:
                (void)fprintf(stderr,
                  "usage: ls [-acdfilnqrstu1ACLFR] [name ...]\n");
                 lsexit(1);
        }
    }
    argv += optind;
    argc -= optind;

    if (!argc) {
    	fd = open(".",O_RDONLY);
        if (fd <= 0) {
            perror("ls:");
            exit(1);
        }
        dd = readDirect(fd,NULL,NULL);
        close(fd);
    	listDir(dd);
    } else { /* files as arguments */
        /* add the args to a special list one by one with GetFileInfo */
        dd = fakeDirect(argc, argv, NULL);
        if (dd->num_entries) listDir(dd);
    }
    lsexit(0);
}
