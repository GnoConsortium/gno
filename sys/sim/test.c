#include <stdio.h>
#include "sim.h"

#pragma optimize 9

asm void TestHandler() {
   sec
   rtl
}

int main(int argc, char *argv[])
{
int v,e;

    e = SIMVersion(&v);
    if (e) {
	printf("SIM Error Code: %d\n",e);
	exit(1);
    }
    printf("SIM Version Code: %04X\n",v);
    e = InstallIntVect(2,(unsigned long) TestHandler);
    if (e) {
	printf("(Install) SIM Error Code: %d\n",e);
	exit(1);
    }
    e = RemoveIntVect(2,(unsigned long) TestHandler);
    if (e) {
	printf("(Remove) SIM Error Code: %d\n",e);
	exit(1);
    }
}
