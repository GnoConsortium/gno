/*
 * This tests the '%b' format specifier for printf and scanf.
 *
 * $Id: bmod.c,v 1.1 1997/10/30 04:45:25 gdr Exp $
 */

int printf(const char *, ...);
int scanf(const char *, ...);
void strcpy(char *, const char *);
void sleep(int);

struct {
  char len;
  char txt[255];
} buf;

#define STRING "this is the end"

int
main (int argc, char **argv) {
  int i;
  
  buf.len = sizeof(STRING) - 1;
  strcpy(buf.txt, STRING);

  printf("The string is: %s\n", buf.txt);
  sleep(2);
  printf("The string is: %b\n", &buf);
  
  printf("please type in a new string:\n");
  i = scanf("%b", &buf);
  printf("scanf returned %d\nThe new string is: %b\n", i, &buf);
  return 0;
}
