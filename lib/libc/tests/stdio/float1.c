#include <stdio.h>

int main(void) {
	double pi = 3.141596;

	printf("(f) pi is %f\n", pi);
	printf("(e) pi is %e\n", pi);
	printf("(g) pi is %g\n", pi);
	printf("(.2g) pi is %.2g\n", pi);
	printf("(.2g) pi is %.2g\n", pi / 1024. );
	return 0;
}
