/*
 * Test the _isnan and _isinf functions.
 * Requires lsaneglue to link and compile
 */

#include <stdio.h>
#include <sane.h> /* Modified sane.h from lsaneglue */

int _isnan (extended);
int _isinf (extended);

int main (void)
{
  int i;

  if (_isinf(s_inf()) && _isinf(-s_inf()))
    printf("_isinf() test successful.\n");
  else
    printf("_isinf() test failed.\n");

  for (i = 0; i < 256; ++i)
  {
    if (_isnan(s_nan(i)) != 1)
    {
      printf("_isnan() test failed.\n");
      return 1;
    }
  }
  printf("_isnan() test successful.\n");
  return 0;
}
