#line 1 ":trenco4:gno.src:lib:libc:tests:gen:getcwd.c"
/*
 * Test written by Devin Reade.
 *
 * $Id: getcwd.c,v 1.1 1997/02/28 05:12:52 gdr Exp $
 */

#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#define BUFFERSIZE  PATH_MAX
#define BUFFERSIZE2 15

__const char *__const sys_siglist[] = { "dummy list", NULL };

int main (int argc, char **argv) {
	static char buffer[BUFFERSIZE];
	static char buffer2[BUFFERSIZE2];

	char *p, *buf;
  int i, len;

  for (i=0; i<3; i++) {
  	switch (i) {
     case 0:  buf = NULL;    len = 0;           break; /* should pass */
     case 1:  buf = buffer;  len = BUFFERSIZE;  break; /* should pass */
     case 2:  buf = buffer2; len = BUFFERSIZE2; break; /* should fail */
     default: assert(0);
     }
  	p = getcwd(buf, len);
  	if (p == NULL) {
	   	perror("getcwd failed");
  	} else {
  		printf("cwd is \"%s\"\n", p);
  	}
  }   
  return 0;                    
}
