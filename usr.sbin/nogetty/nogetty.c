#include <stdio.h>
#include <orca.h>

int main(int argc, char *argv[])
{
    setenv("user","root");
    setenv("home",":user:root");
    execve("/bin/gsh","gsh");
}
