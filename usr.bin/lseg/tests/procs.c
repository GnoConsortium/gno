/*
 * Test code for lseg
 *            
 *   Generate procedures using various amounts of stack space
 *   Not designed to be executed; just examined
 *            
 *   Dave Tribby  *  September 1997
 */

#pragma noroot
#pragma stacksize 256
#pragma debug 0

int main(int argc, char *argv[])
{
return 0;
}

/* Simple procedures no optimization or debug */
#pragma optimize 0
void proc0(void)	{}
void proc1(void)	{ char ch; }
void proc2(void)	{ int i; }
void proc3(void)	{ char ch; int i; }
void proc4096(void)	{ char ch[4096]; }


/* Change the databank to see its effect */
#pragma databank 1
void databank0(void)	{}
void databank1(void)	{ char ch; }
void databank2(void)	{ int i; }
void databank3(void)	{ char ch; int i; }
void databank4096(void)	{ char ch[4096]; }
#pragma databank 0


/* Turn on full optimization to see its effect */
#pragma optimize -1
void proc0o(void)	{}
void proc1o(void)	{ char ch; }
void proc2o(void)	{ int i; }
void proc3o(void)	{ char ch; int i; }
void proc4096o(void)	{ char ch[4096]; }
#pragma optimize 0


/* Turn on full debug to see its effect */
#pragma debug -1
void proc0db(void)	{}
void proc1db(void)	{ char ch; }
void proc2db(void)	{ int i; }
void proc3db(void)	{ char ch; int i; }
void proc4096db(void)	{ char ch[4096]; }
#pragma debug 0
