#include <stdio.h>

int main(void)
{
  int s = findfpcp();

  if (s == -1)
    printf("FPE not found\n");
  else
    printf("FPE found in slot %d\n", s);

    return (0);
}
