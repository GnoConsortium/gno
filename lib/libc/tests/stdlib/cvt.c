#include <stdio.h>
#include <string.h>
#include <sane.h>

char *ecvt (double, size_t, int *, int *);
char *fcvt (double, size_t, int *, int *);

#define PI 3.141592653589793238512808959
#define BUFFERSIZE 20

char buffer[BUFFERSIZE];
               
int
main(int argc, char **argv) {
	double d;
	int ndigits, decpt, sign, buflen, i;

	buflen = BUFFERSIZE - 10;

	for (i=0; i<4; i++) {
		switch(i) {
		case 0:		d = PI;		break;
		case 1:		d = 0.0123;	break;
		case 2:		d = 166.75;	break;
     case 3:		d = 0.9876;	break;
		}
        	ndigits = 6 /*buflen*/;

		strcpy(buffer, ecvt(-d, ndigits, &decpt, &sign));
		printf("ecvt: decpt=%d\tsign=%d\tbuffer=%s\n",
			decpt, sign, buffer);

		strcpy(buffer, fcvt(-d, ndigits, &decpt, &sign));
		printf("fcvt: decpt=%d\tsign=%d\tbuffer=%s\n",
			decpt, sign, buffer);

		printf("\n");
        }
	return 0;
}
