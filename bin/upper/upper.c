/* converts text to upper case, used to gsh pipe mechanism  */
/* program by Tim Meekins, Copyright (C) 1991 Procyon, Inc. */

#include <stdio.h>
#include <texttool.h>
#pragma optimize -1
#pragma stacksize 1024

main()
{
  char ch;

  while(1)
  {
    ch = ReadChar(0) & 0x7f;
    if (ch==0) return;
    if (ch >= 'a' && ch <= 'z')
      ch = ch - ('a'-'A');
    WriteChar(ch);
    if (ch==13) WriteChar(10);
  }
}
