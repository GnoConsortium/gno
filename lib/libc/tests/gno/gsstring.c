/*
 * Tests by Devin Reade
 *
 * $Id: gsstring.c,v 1.1 1997/02/28 05:12:55 gdr Exp $
 *
 * This file is formatted for tab stops every 8 columns.
 */

#include <types.h>
#include <gno/gno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FAIL() \
{ \
	printf("test failed at line %d of %s\n", __LINE__, __FILE__); \
	exit(EXIT_FAILURE); \
}

#if 1
#define NO_RESOURCE()	{ printf("resource failure\n"); exit(EXIT_FAILURE); }
#else
#define NO_RESOURCE() \
{ \
	perror("resource failure"); \
	exit(EXIT_FAILURE); \
}
#endif

const char *str1 = "this is a test string\n";
const char *str2 = "this is longer than the first\n";

int main(int argc, char **arv) {
	GSStringPtr ptr1, ptr2;
	ResultBufPtr ptr3, ptr4;
	size_t len1, len2;

	printf("testing GIinit\n");
	len1 = strlen(str1);
	if ((ptr1 = GIinit(len1, str1)) == NULL) NO_RESOURCE();
	if (len1 != ptr1->length)	FAIL();
	if (ptr1->text[len1] != '\0')	FAIL();
	if (strcmp(str1, ptr1->text))	FAIL();
	
	printf("testing GIchange\n");
	len2 = strlen(str2);
	if ((ptr2 = GIchange(ptr1, len2, NULL)) == NULL) NO_RESOURCE();
	if (len2 != ptr2->length)	FAIL();
	if (ptr2->text[len2] != '\0')	FAIL();
	if (ptr2->text[0] != '\0')	FAIL();

	printf("testing GIfree\n");
	GIfree(ptr2);

#define RESULT_LEN1 300
#define RESULT_LEN2 10

	printf("testing GOinit\n");
	if ((ptr3 = GOinit(RESULT_LEN1, str1)) == NULL)	NO_RESOURCE();
	if (ptr3->bufSize != RESULT_LEN1 + 2*sizeof(word))	FAIL();
	if (ptr3->bufString.length != len1)			FAIL();
	if (ptr3->bufString.text[len1] != '\0')			FAIL();
	if (strcmp(ptr3->bufString.text, str1))			FAIL();

	printf("testing GOchange\n");
	if (RESULT_LEN2 >= len1)				FAIL();
	if ((ptr4 = GOchange(ptr3, RESULT_LEN2, NULL)) == NULL)	FAIL();
	if (ptr4->bufSize != RESULT_LEN2 + 2*sizeof(word))	FAIL();
	if (ptr4->bufString.length != RESULT_LEN2)		FAIL();
	if (ptr4->bufString.text[RESULT_LEN2] != '\0')		FAIL();
	if (strncmp(ptr4->bufString.text, str1, RESULT_LEN2))	FAIL();

	printf("testing GOfree\n");
	GOfree(ptr4);

	printf("\nPASSED\n");
	return 0;
}
