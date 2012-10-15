#include <strings.h>
#include <stdio.h>

#define TEST(fx, input, expected) {\
	int result = fx(input); \
	if (result != expected) { \
		fprintf(stderr, "%s(0x%04x): %d != %d\n",  #fx, input, result, expected);\
		errors++;\
	} \
}

#define TESTL(fx, input, expected) {\
	int result = fx(input); \
	if (result != expected) { \
		fprintf(stderr, "%s(0x%08lx): %d != %d\n", #fx, (long)input, result, expected);\
		errors++;\
	} \
}

int main(int argc, char **argv)
{
	int errors = 0;
	TEST(ffs, 0x0000, 0);
	TEST(ffs, 0x0001, 1);
	TEST(ffs, 0xff00, 9);
	TEST(ffs, 0x8000, 16);

	TESTL(ffsl, 0x00000000, 0);
	TESTL(ffsl, 0x00000001, 1);
	TESTL(ffsl, 0x00008000, 16);
	TESTL(ffsl, 0x00010000, 17);
	TESTL(ffsl, 0x80000000, 32);

	TEST(fls, 0x0000, 0);
	TEST(fls, 0x0001, 1);
	TEST(fls, 0xff00, 16);
	TEST(fls, 0x8000, 16);

	TESTL(flsl, 0x00000000, 0);
	TESTL(flsl, 0x00000001, 1);
	TESTL(flsl, 0x00008000, 16);
	TESTL(flsl, 0xffffffff, 32);
	TESTL(flsl, 0x80000000, 32);

	fprintf(stdout, "%d errors\n", errors);
	return 0;
}

