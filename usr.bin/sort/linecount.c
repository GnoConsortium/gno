#ifdef __CCFRONT__
#include <14:pragma.h>
#endif

#include "common.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * unsigned long int linecount (char *filename, size_t *maxlinelen);
 *
 * Pre:  <filename> is the name of the file for which we need to know
 *       the number of lines.  The file must be closed.
 *
 * Post: Returns the number of newline characters in the file.  On
 *       return, the file is again closed and *maxlinelen is the length
 *       of the longest line in <filename> (length is calculated to 
 *       include the newline character but not the null terminator.  
 *       Returns zero on failure or if there are no newlines.
 *
 * Uses Globals:
 *       v_flag
 */

unsigned long int linecount (char *filename, size_t *maxlinelen) {

  char *buff;                   /* the input buffer */
  unsigned long result;         /* the number of newlines */
  int count;                    /* the number of chars last read */
  int fd;                       /* file descriptor for <filename> */
  short done;
  int i;
  size_t linelen;               /* length of current line */
   
  /* init some variables */
  done = 0;
  result = 0;
  *maxlinelen = 0;
  linelen = 0;

  /* open <filename> for unbuffered I/O */
  if ((fd = open(filename,O_RDONLY)) == -1) {
    if (v_flag) perror("linecount: couldn't open input file");
    return 0lu;
  }

  /* get an input buffer */
  if ((buff = malloc(BUFFERSIZE)) == NULL) {
    if (v_flag) perror ("linecount: couldn't allocate buffer");
    close(fd);
    return 0lu;
  }

  /* repeatedly fill the buffer and increment the newline count */
  while (!done) {
    count = read (fd, buff, BUFFERSIZE);
    switch (count) {
    case -1:                            /* file error */
      if (v_flag) perror ("linecount");
      close(fd);
      free(buff);
      return 0lu;
      /* NOTREACHED */
      break;
      
    case 0:                             /* EOF */
      done = 1;
      break;
      
    default:          /* got some info in the buffer */
      for (i=0; i<count; i++) {
        linelen++;
        if (buff[i] == NEWLINE) {
          result++;
          if (linelen > *maxlinelen) *maxlinelen = linelen;
          linelen = 0;
        }
      }
      break;
    }
  }
  
  /* clean up and return */
  close(fd);
  free(buff);
  return result;
}
