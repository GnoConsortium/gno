#include <stdio.h>
#include <texttool.h>

void dor(int r)
{
    if (r < 32) {
    	ErrWriteChar(' ');
    	ErrWriteChar('^');
    	ErrWriteChar(r+64);
    	ErrWriteChar(' ');
    }                   
    else {
    	ErrWriteChar(' ');
    	ErrWriteChar(r);
    	ErrWriteChar(' ');
	}
}         

int main(int argc, char *argv[])
{
int r;

    SetInputDevice(1,1l);
    SetOutputDevice(1,1l);
    InitTextDev(0);

    WriteChar(27);
    WriteChar('?');
    r = ReadChar(0);
    dor(r);
    r = ReadChar(0);
    dor(r);
    r = ReadChar(0);
    dor(r);
    r = ReadChar(0);
    dor(r);
    r = ReadChar(0);
    dor(r);
}          
