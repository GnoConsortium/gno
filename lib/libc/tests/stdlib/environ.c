/*
 * Tests implemented by Devin Reade
 *
 * $Id: environ.c,v 1.1 1997/02/28 05:12:57 gdr Exp $
 *
 * This file has been formatted with tab stops every 8 columns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <types.h>
#include <shell.h>
#include <errno.h>

#define NONE_SET 0x00
#define UNIX_SET 0x01
#define ORCA_SET 0x02
#define BOTH_SET 0x03

extern char **environ;

static char *string1="testname";
static char *string2="testvalue";
static char *string3="testname=testvalue";
int use_environ;

/*
 * void print_Unix_environ (void);
 *
 * Pre:  none.
 *
 * Post: the environment residing in the environ structure is printed
 *       to stdout.
 */

void
print_Unix_environ (void) {
	char **e, *s;
	const char *fname = "print_Unix_environ";

	if (!environ) {
		printf("%s: environ is NULL\n", fname);
		return;
	}

	e = environ;
	if (*e==NULL) {
		printf("%s: environment is empty\n", fname);
	}
	while (*e != NULL) {
		s = *e;
		if (*s == (char)NULL) {
			printf("%s: entry of zero length\n", fname);
		} else {
			printf("%s\n",s);
		}
		e++;
	}
	return;
}

/*
 * void print_Orca_environ (void);
 *
 * Pre:  none.
 *
 * Post: the environment residing internal to the shell is printed to stderr.
 *       It's not terribly efficient, but then it doesn't have to be ;)
 */

void
print_Orca_environ (void) {

	static ReadIndexedGSPB  parmBuffer;
	static ResultBuf255     nameBuffer, valueBuffer;
	static char *fname = "print_Orca_environ";
	char *entry;
	unsigned int nameLength, valueLength;

	/*
	 * initialize the parameter block
	 */

	parmBuffer.pCount = 4;
	parmBuffer.index = 1;
	nameBuffer.bufSize  = sizeof (GSString255);
	valueBuffer.bufSize = sizeof (GSString255);
	parmBuffer.name  = &nameBuffer;
	parmBuffer.value = &valueBuffer;

	/* loop until we've got them all */
	ReadIndexedGS (&parmBuffer);
	nameLength = nameBuffer.bufString.length;
	while (nameLength != 0) {
		valueLength = valueBuffer.bufString.length;
      
		/* allocate the new environ entry */
		entry = malloc (nameLength + valueLength + 2);
		if (entry == NULL) {
			printf("malloc failed in %s: %s\n", fname,
				strerror(errno));
			exit(1);
		}

		/* copy the name and value */
		strncpy (entry, nameBuffer.bufString.text, nameLength);
		entry[nameLength] = (char) NULL;
		strcat (entry, "=");
		strncat (entry, valueBuffer.bufString.text, valueLength);
		entry[nameLength + valueLength + 1] = (char) NULL;

		printf("%s\n",entry);
		free(entry);

		/* get the next shell variable and continue ... */
		parmBuffer.index++;
		ReadIndexedGS (&parmBuffer);
		nameLength = nameBuffer.bufString.length;
	}                
	return;        
}

unsigned int
test_environ (void) {

	unsigned int unix_set = 0;
	unsigned int orca_set = 0;
	char *s;
	char **e;

	/* test unix version */
	if (use_environ) {

#ifdef DEBUG
		printf("\n\nUnix environment:\n\n");
		print_Unix_environ ();
		printf("\n\n");
#endif
   
		e = environ;
		while (e && *e) {
			if (strncmp(*e,string3,strlen(string3)) == 0) {
				unix_set = UNIX_SET;
			}
			e++;
		}
	}

	/* test Orca version */
	if (getenv(string1) != NULL) {
		orca_set = ORCA_SET;
	}

	return (unix_set | orca_set);
}


int
main (int argc, char **argv) {

	int i=1; /* the number of the test */
	unsigned int result;

	if (argc > 1) {
		use_environ = 1;
	} else {
		use_environ = 0;
		printf("NOT ");
	}
	printf("using environ global variable\n");

	/*
	 * initial         Test 1:1
	 */

	result = test_environ();
	if (result != NONE_SET) {
		printf("Please unset the variable \"%s\" for this test.\n",
			string1);
		return -1;
	}
	printf("Test %d passed.\n",i);
	i++;

#define FAIL(fmt) { printf(fmt, i, string1); exit(1); }

	/*
	 * environInit();	Test  :2
	 */

	if (use_environ) {
		environInit();
		result = test_environ();
		switch (result) {
		case BOTH_SET:	FAIL("Test %d failed.  %s prematurely set\n");
		case ORCA_SET:	FAIL("Test %d failed.  %s set internally\n");
		case UNIX_SET:	FAIL("Test %d failed.  %s set in environ\n");
		case NONE_SET:
			printf("Test %d passed.\n",i);
			i++;
			break;
		default:
			printf("Test internal error:  test %d returned %ud\n",
				i, result);
			return -1;
		}
	}  

	/*
	 * setenv();       Test 2:3
	 */

	if (setenv(string1,string2,1) != 0) {
		printf("Test %d:  setenv() failed\n",i);
		return -1;
	}
	result = test_environ();
	if (use_environ) {
		switch (result) {
		case BOTH_SET:
			printf("Test %d passed.\n",i);
			i++;
			break;
		case ORCA_SET:
			FAIL("Test %d failed.  %s not set in environ\n");
		case UNIX_SET:
			FAIL("Test %d failed.  %s not set internally\n");
		case NONE_SET:
			FAIL("Test %d failed.  %s not set\n");
		default:
			printf("Test internal error:  test %d returned %ud\n",
				i, result);
			return -1;
		}
	} else {
		switch (result) {
		case BOTH_SET:
			FAIL("Test %d failed.  %s set in environ\n");
		case ORCA_SET:
			printf("Test %d passed.\n", i);
			i++;
			break;
		case UNIX_SET:
			FAIL("Test %d failed.  %s set in environ, not set internally\n");
		case NONE_SET:
			FAIL("Test %d failed.  %s not set\n");
		default:
			printf("Test internal error:  test %d returned %ud\n",
				i, result);
			return -1;
		}
	}

	/*
	 * unsetenv()	Test 3:4
	 */

	unsetenv(string1);
	result = test_environ();
	if (use_environ) {
		switch (result) {
		case BOTH_SET:
			FAIL("Test %d failed.  %s set\n");
		case ORCA_SET:
			FAIL("Test %d failed.  %s set internally\n");
		case UNIX_SET:
			FAIL("Test %d failed.  %s set in in environ\n");
		case NONE_SET:
			printf("Test %d passed.\n",i);
			i++;
			break;
		default:
			printf("Test internal error:  test %d returned %ud\n",
				i, result);
			return -1;
		}
	} else {
		switch (result) {
		case BOTH_SET:
			FAIL("Test %d failed.  %s set\n");
		case ORCA_SET:
			FAIL("Test %d failed.  %s set internally\n");
		case UNIX_SET:
			FAIL("Test %d failed.  %s set in environ\n");
		case NONE_SET:
			printf("Test %d passed.\n",i);
			i++;
			break;
		default:
			printf("Test internal error:  test %d returned %ud\n",
				i, result);
			return -1;
		}
	}

	/*
	 * environPush()	Test 4:5 bork
	 */
	
	if (setenv(string1,string2,1) != 0) {
		printf("Test %d:  setenv() failed\n",i);
		return -1;
	}

	if (environPush() != 0) { 		
		printf("Test %d:  environPush() failed\n",i);
		return -1;
	}

	result = test_environ();    
	if (use_environ) {
		switch (result) {
		case BOTH_SET:
			printf("Test %d passed.\n",i);
			i++;
			break;
		case ORCA_SET:
			FAIL("Test %d failed.  %s not set in environ\n");
		case UNIX_SET:
			FAIL("Test %d failed.  %s not set internally\n");
		case NONE_SET:
			FAIL("Test %d failed.  %s not set\n");
		default:
			assert(0);
		}
	} else {
		switch (result) {
		case BOTH_SET:
			FAIL("Test %d failed.  %s set in environ\n");
		case ORCA_SET:
			printf("Test %d passed.\n",i);
			i++;
			break;
		case UNIX_SET:
			FAIL("Test %d failed.  %s set in environ, not set internally\n");
		case NONE_SET:
			FAIL("Test %d failed.  %s not set\n");
		default:
			assert(0);
		}
	}


	/*
	 * environPop()		  Test 5:6
	 */

	unsetenv(string1);
	environPop();
	result = test_environ();
	if (use_environ) {
		switch (result) {
		case BOTH_SET:
			printf("Test %d passed.\n",i);
			i++;
			break;
		case ORCA_SET:
			FAIL("Test %d failed.  %s not set in environ\n");
		case UNIX_SET:
			FAIL("Test %d failed.  %s not set internally\n");
		case NONE_SET:
			FAIL("Test %d failed.  %s not set\n");
		default:
			assert(0);
		}
	} else {
		switch (result) {
		case BOTH_SET:
			FAIL("Test %d failed.  %s set in environ\n");
		case ORCA_SET:
			printf("Test %d passed.\n",i);
			i++;
			break;
		case UNIX_SET:
			FAIL("Test %d failed.  %s set in environ, not set internally\n");
		case NONE_SET:
			FAIL("Test %d failed.  %s not set\n");
		default:
			assert(0);
		}
	}


	/*
	 * putenv()	  Test 6:7
	 */

	unsetenv(string1);
	if (putenv(string3) != 0) {				  
		printf("Test %d:  putenv() failed\n",i);
		return -1;
	}
	result = test_environ();
	if (use_environ) {
		switch (result) {
		case BOTH_SET:
			printf("Test %d passed.\n",i);
			i++;
			break;
		case ORCA_SET:
			FAIL("Test %d failed.  %s not set in environ\n");
		case UNIX_SET:
			FAIL("Test %d failed.  %s not set internally\n");
		case NONE_SET:
			FAIL("Test %d failed.  %s not set\n");
		default:
			assert(0);
		}
	} else {
		switch (result) {
		case BOTH_SET:
			FAIL("Test %d failed.  %s set in environ\n");
		case ORCA_SET:
			printf("Test %d passed.\n",i);
			i++;
			break;
		case UNIX_SET:
			FAIL("Test %d failed.  %s set in environ, not set internally\n");
		case NONE_SET:
			FAIL("Test %d failed.  %s not set\n");
		default:
			assert(0);
		}
	}

	unsetenv(string1);
	printf("Tests done.\n");
	return 0;
}
