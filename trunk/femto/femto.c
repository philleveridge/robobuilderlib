#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "win.h"
#endif

#ifdef LINUX
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "linux.h"
#endif

#ifdef AVR
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h> 
#include <avr/interrupt.h>

#include "rprintf.h"
#include "uart.h"
#include "main.h"
#include "macro.h"
#endif


#include "femto.h"

int dbg=0;

//wait for byte or IR press
int  GetByte()
{
	int b;
	
	RUN_LED1_OFF;
	while (1) 
	{
		if ((b=uartGetByte())>=0) break;
		if ((b=irGetByte())>0)   break;
	}
	RUN_LED1_ON;
	
	return b;
}


void readLine(char *s, int n)
{
	int ch=0;
	int ibcnt=0;

	rprintfProgStr(PSTR("> "));
	
	while (ibcnt<n) 
	{
		ch=GetByte();
		ibcnt++;

		if (ch == 13) 
		{
			rprintfChar(13); 
			rprintfChar(10);
		   	break;
		}
		if (ch==8 || ch==127)   //Bsapce ?
		{
			if (ibcnt>0) 
			{
				rprintfChar(8);
				rprintfChar(32);
				rprintfChar(8);
				ibcnt--;
				s--;
			}	
		}
		else
		{
			rprintfChar(ch);            //echo input
			*s++ = ch;
		}
	}		
	*s=0;
}

BYTE EEMEM FIRMWARE        	[64];  	// leave blank - used by Robobuilder OS
BYTE EEMEM BASIC_PROG_SPACE	[256];  // global env dictonary to be stored in EEPROM

void femto()
{
	rprintfProgStr(PSTR("Femto v0.1\n"));
	repl();
}

#ifndef AVR
main()
{
	femto();
}
#endif


