#include <sys/types.h>
#include <stdio.h>

#ifdef DEFFUNC
#  define EXTERN
#else
#  define EXTERN extern
#endif

#define ALN2I              1.442695022   /* 1 / ln(2)           */
#define TINY               1.0e-5        /* "zero" for heapsort */
#define BUFFERSIZE         4096          /* a generic buffer for I/O */
#define DEFAULT_LINECOUNT  1000          /* number of lines to memory sort */
#define DEFAULT_LINELENGTH  512          /* max length of line recognised */
#define DELIM              0x03          /* ETX */

#ifdef __ORCAC__
#  define NEWLINE    '\r'
#else
#  define NEWLINE    '\n'
#  define BROKEN_REALLOC
#endif

#ifdef __GNUC__
  int printf(char *format, ...);
  int fprintf(FILE *stream, char *format, ...);
  void perror(char *s);
  int close(int fd);
  int fclose(FILE *stream);
  int rename(char *, char *);
  void rewind(FILE *);
#endif

#ifdef DEBUG
#  define STATUS(string) fprintf(stderr,"%s\n",string)
   extern void begin_stack_check(void);
   extern int  end_stack_check(void);
#else
#  define STATUS(string) {;}
#endif

unsigned long int linecount (char *filename, size_t *maxlinelen);
char **loadarray (unsigned long n, char *filename, size_t maxlinelen);
void sortarray(char *array[], unsigned long n);
int disksort (char *filename, size_t linecount, size_t linelength);
int initdisksort(void);
int mergeone(FILE *fpA, FILE *fpB, FILE *fpC, char strA[], char strB[],
	     size_t linelength);

EXTERN short  v_flag;
EXTERN FILE   *out_fp;

#ifdef DSORT
EXTERN FILE   *fp1, *fp2, *fp3, *fp4;
EXTERN char   *file1, *file2, *file3, *file4;
EXTERN char   *tpath1, *tpath2, *tpath3, *tpath4;
#endif
