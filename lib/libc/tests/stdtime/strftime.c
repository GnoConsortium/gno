#include <stdio.h>
#include <time.h>

const char *formats[] = {
  "%A",
  "%a",
  "%B",
  "%b",
  "%C",
  "%c",
  "%D",
  "%d",
#if 1
  "%Ec",
  "%EC",
  "%Ex",
  "%Ey",
  "%EY",
#endif
  "%e",
  "%H",
  "%h",
  "%I",
  "%j",
  "%k",
  "%l",
  "%M",
  "%m",
  "%n",
#if 1
  "%Od",
  "%Oe",
  "%OH",
  "%OI",
  "%Om",
  "%OM",
  "%OS",
  "%Ou",
  "%OU",
  "%OV",
  "%Ow",
  "%OW",
  "%Oy",
#endif
  "%p",
  "%R",
  "%r",
  "%S",
  "%s",
  "%T",
  "%t",
  "%U",
  "%u",
  "%V",
  "%v",
  "%W",
  "%w",
  "%x",
  "%Y",
  "%y",
  "%Z",
  "%+",
  "%%",
  /* This is the format used in getty(1) */
  "%l:%M%p on %A, %d %B %Y",
  NULL
};

#define BUFFERSIZE 1024
static char buffer[BUFFERSIZE];

int main (int argc, char **argv) {
  static struct tm *tmptr;
  time_t timeval;
  int i;
  size_t result;

  time(&timeval);
  tmptr = localtime(&timeval);

  printf("SEED = %ld\n", timeval);

  for (i=0; formats[i] != NULL; i++) {
    result = strftime(buffer, BUFFERSIZE-1, formats[i], tmptr);
    printf("%s\t%ld\t%s\n", formats[i], result, buffer);
  }
  return 0;
}

