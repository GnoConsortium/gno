
#pragma optimize -1

#include <setjmp.h>
#include <signal.h>

jmp_buf	my_jump;

#pragma databank 1
void my_handler(int sig, int code)
{
	printf("WHEEE!\n");
	asm {brk 2}
	longjmp(my_jump, 1);
}
#pragma databank 0

int main(int argc, char *argv[])
{
	sigblock(sigmask(SIGPIPE)); /* something to make the mask interesting */
	signal(SIGTSTP, my_handler);
	asm {brk 0}
	setjmp(my_jump);
	asm {brk 1}
	for (;;);
}
