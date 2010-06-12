#include <stdio.h>

#define WORD		int
#define BYTE		unsigned char

volatile WORD   gticks=0;
volatile WORD   g10MSEC=0;
volatile WORD   g10Mtimer=0;
volatile BYTE   gSEC=0;
volatile BYTE   gMIN=0;

void delay_ms(int ms)
{
}

void putByte (BYTE b)
{
	putchar(b);
}

int  getByte ()
{
	return getchar();
}

int irGetByte()
{
	return -1;
}
