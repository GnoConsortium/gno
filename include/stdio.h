/****************************************************************
*
*  stdio.h - input/output facilities
*
*  February 1989
*  Mike Westerfield
*
*  Copyright 1989, 1993
*  Byte Works, Inc.
*
****************************************************************/

#ifndef __stdio__
#define __stdio__

#ifndef _SYS_CDEFS_H_
#include <sys/cdefs.h>
#endif

#ifndef _SYS_SYSLIMITS_H_
#include <sys/syslimits.h>
#endif

/*
 *  Misc.
 */

#ifndef __va_list__
#define __va_list__
typedef char *__va_list[2];
#endif

#ifndef EOF
#define EOF             (-1)
#endif

#ifndef NULL
#define NULL  (void *) 0L
#endif

#ifndef __size_t__
#define __size_t__ 1
typedef unsigned long size_t;
#endif

/* seek codes */

#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2

/*
 *  Error handling
 */

#if !defined(__KeepNamespacePure__) && !defined(_ANSI_SOURCE) && !defined(_POSIX_SOURCE)
   extern const int sys_nerr;               /* largest index for sys_errlist */
   extern const char * const sys_errlist[]; /* error messages */
#endif

/*
 *  files
 */

typedef struct __file {
   struct __file        *next;          /* next file in linked list */
   unsigned char        *_ptr,          /* next location to write to */
                        *_base,         /* first byte of the buffer */
                        *_end;          /* end of the file buffer */
   unsigned long        _size,          /* size of the file buffer */
                        _cnt;           /* # chars that can be read/writen to buffer */
#if defined(__ORCAC_VERSION) && (__ORCAC_VERSION > 210)
   int                  _pbk[2];        /* put back character; changed in v2.1.1b2 */
#else
#error bad stuff batman
   int                  _pbk;           /* put back character */
#endif                        
   unsigned int         _flag,          /* buffer flags */
                        _file;          /* GS/OS file ID */
   } FILE;

#define BUFSIZ          1024            /* default buffer size */
#define _LBUFSIZ        255             /* line buffer size */

#define _IOFBF          0x0001          /* full buffering */
#define _IONBF          0x0002          /* no buffering */
#define _IOLBF          0x0004          /* flush when a \n is written */
#define _IOREAD         0x0008          /* currently reading */
#define _IOWRT          0x0010          /* currently writing */
#define _IORW           0x0020          /* read/write enabled */
#define _IOMYBUF        0x0040          /* buffer was allocated by stdio */
#define _IOEOF          0x0080          /* has an EOF been found? */
#define _IOERR          0x0100          /* has an error occurred? */
#define _IOTEXT         0x0200          /* is this file a text file? */
#define _IOTEMPFILE     0x0400          /* was this file created by tmpfile()? */

extern FILE *stderr;                    /* standard I/O files */
extern FILE *stdin;
extern FILE *stdout;

#define L_tmpnam        26              /* size of a temp name */
#define TMP_MAX         10000           /* # of unique temp names */
#ifdef __GNO__
#define	__max_open	OPEN_MAX	/* <= OPEN_MAX <sys/syslimits.h> */
#else
#define __max_open	32767		/* Orca Shell: no practical limit */
#endif
#ifndef __KeepNamespacePure__
   #define SYS_OPEN	__max_open      /* max # open files */
#endif
#define FOPEN_MAX       __max_open      /* max # open files */
#undef	__max_open
#define FILENAME_MAX    1024            /* recommended file name length */

/*
 *  Other types
 */

typedef long fpos_t;

/*
 *  Functions declared as macros
 */

#ifdef __ORCAC_VERSION		/* Orca/C v2.1.0 or later */

#define setbuf(stream,buf)      ((buf==NULL) ? (void) __setvbuf(stream,NULL,_IONBF,0l) : (void) __setvbuf(stream,buf,_IOFBF,(size_t) BUFSIZ))
#define setlinebuf(stream)	(__setvbuf((stream),NULL,_IOLBF,_LBUFSIZ))
#define rewind(stream)          (__fseek((stream),0L,SEEK_SET))
int		__fseek(FILE *, long, int);
int		__setvbuf(FILE *, char *, int, size_t);
int             getc(FILE *);
int             putc(int, FILE *);

#else	/* __ORCAC_VERSION */

#define getc(p)                 (fgetc(p))
#define putc(x, p)              fputc(x, p)
#define setbuf(stream,buf)      ((buf==NULL) ? (void) setvbuf(stream,NULL,_IONBF,0l) : (void) setvbuf(stream,buf,_IOFBF,(size_t) BUFSIZ))
#define setlinebuf(stream)	(setvbuf((stream),NULL,_IOLBF,_LBUFSIZ))
#define rewind(stream)          (fseek((stream),0L,SEEK_SET))

#endif	/* __ORCAC_VERSION */

/*
 *  Function declarations
 */

void            clearerr(FILE *);
int             fclose(FILE *);
int             feof(FILE *);
int             ferror(FILE *);
int             fflush(FILE *);
int             fgetc(FILE *);
int             fgetpos(FILE *, fpos_t *);
char           *fgets(char *, int, FILE *);
FILE           *fopen(const char *, const char *);
int             fprintf(FILE *, const char *, ...);
int             fputc(int, FILE *);
int             fputs(const char *, FILE *);
size_t          fread(void *, size_t, size_t, FILE *);
FILE           *freopen(const char *, const char *, FILE *);
int             fscanf(FILE *, const char *, ...);
int             fseek(FILE *, long, int);
int             fsetpos(FILE *, const fpos_t *);
long int        ftell(FILE *);
size_t          fwrite(const void *, size_t, size_t, FILE *);
int             getchar(void);
char           *gets(char *);
void            perror(const char *);
int             printf(const char *, ...);
int             putchar(int);
int             puts(const char *);
int             remove(const char *);
int             rename(const char *, const char *);
int             scanf(const char *, ...);
int             setvbuf(FILE *, char *, int, size_t);
int             sprintf(char *, const char *, ...);
int             sscanf(const char *, const char *, ...);
FILE           *tmpfile(void);
char           *tmpnam(char *);
int             ungetc(int c, FILE *);
int             vfprintf(FILE *, const char *, __va_list);
int             vprintf(const char *, __va_list);
int             vsprintf(char *, const char *, __va_list);

/*
 * The remaining functions and macros in this file are GNO-specific
 */

/* System V/ANSI C; this is the wrong way to do this, do *not* use these. */
#ifndef _ANSI_SOURCE
#define	P_tmpdir	"/tmp/"
#endif

/*
 * Functions defined in POSIX 1003.1.
 */
#ifndef _ANSI_SOURCE
#define	L_cuserid	9	/* size for cuserid(); UT_NAMESIZE + 1 */
#define	L_ctermid	1024	/* size for ctermid(); PATH_MAX */
#define	fileno(f)	(f->_file)

__BEGIN_DECLS
#ifndef _POSIX_SOURCE
char	*cuserid __P((char *));				/* non-BSD */
#endif
char	*ctermid __P((char *));
FILE	*fdopen __P((int, const char *));
int	 fileno __P((FILE *));
__END_DECLS
#endif /* not ANSI */

/*
 * Routines that are purely local.
 */
#if !defined (_ANSI_SOURCE) && !defined(_POSIX_SOURCE)
__BEGIN_DECLS
char	*fgetln __P((FILE *, size_t *));
int	 fpurge __P((FILE *));
int	 getw __P((FILE *));
int	 pclose __P((FILE *));
FILE	*popen __P((const char *, const char *));
int	 putw __P((int, FILE *));
void	 setbuffer __P((FILE *, char *, int));
char	*tempnam __P((const char *, const char *));
int	 snprintf __P((char *, size_t, const char *, ...));
int	 vfscanf __P((FILE *, const char *, __va_list));
int	 vsnprintf __P((char *, size_t, const char *, __va_list));
int	 vscanf __P((const char *, __va_list));
int	 vsscanf __P((const char *, const char *, __va_list));
FILE	*zopen __P((const char *, const char *, int));
__END_DECLS

#endif	/* !_ANSI_SOURCE && !_POSIX_SOURCE */

#endif	/* __stdio__ */
