#pragma optimize -1
#pragma stacksize 256

/*                                                                     */
/* newverify.c - a simple program to launch newuser with the -v option */
/*                                                                     */

#include<stdio.h>
#include<stdlib.h>
#include<gno/gno.h>

int main(int argc, char **argv)

   {
   execve("/usr/sbin/newuser","newuser -v");
   }
