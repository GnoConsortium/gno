#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char **argv) {
	register int i, j;
	double d;
	char *p;

	while(1) {
		printf("enter an integer:\n");
		i = scanf("%d", &j);
		if (i<0) exit (1);
		if (i > 0) {
			printf("scanf returned %d.  j == %d\n", i, j);
		} else {
			printf("invalid input: enter an integer\n");
			continue;
		}
		d = 5.5;
		printf("enter a float.  5.5 = %g\t", d);
		p = (char *) &d;
		for (j = 0; j < sizeof(double); j++) {
			printf("%x ", *p++);
		}
		printf("\n");
		i = scanf("%lf", &d);
		if (i<0) exit (1);
		if (i > 0) {
			printf("i = %d, d = %g\n", i, d);
			p = (char *) &d;
			for (j = 0; j < sizeof(double); j++) {
				printf("%x ", *p++);
			}
			printf("\n");
		} else {
			printf("invalid input: enter a double\n");
		}
	}
	return 0;
}
