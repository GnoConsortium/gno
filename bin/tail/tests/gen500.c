/* Generate file for tail test */

#include <stdio.h>

int
main(int argc, char *argv[])
{
	int i;

        for (i=1; i<=500; i++)
	        printf("This is line number%5d\n",i);
}
