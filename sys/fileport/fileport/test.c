
#include <stdio.h>

void c_port_open(void);
void c_port_write(char *, int);
void c_port_close(void);

char buffer[] = {"This is a simple test...\n"};

int main(int argc, char * argv[])
{
    c_port_open();
    c_port_write(&buffer[0], (int) 25);
    c_port_close();
    exit(0);
}

