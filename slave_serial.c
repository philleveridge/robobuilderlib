// slave_serial.c
//
//	This is our "experimental" mode where we feel free to hack and try
//	things that may or may not work.  It consists mainly of a lot of little
//	routines that the user can access via the serial interface.

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "Main.h"
#include "Macro.h"
#include "comm.h"
#include "rprintf.h"
#include "adc.h"
#include "ir.h"
#include "uart.h"
#include "accelerometer.h"
#include "majormodes.h"

#include <util/delay.h>

extern WORD    gMSEC;				// 
extern BYTE    gSEC;				// 
extern BYTE    gMIN;				// 
extern BYTE    gHOUR;				// 

extern int getHex(int d);

//------------------------------------------------------------------------------
// Perform  Motion when event occurs
// Read_and_Go   (or is it Read_and_WeEP?)
//------------------------------------------------------------------------------


void Do_Serial(void)
{
	BYTE	lbtmp;
	WORD	ptmpA;
	
	// -------------------------------------------------------------------------------------------------------
	// terminal input get input and  send to cooms and display response
	// -------------------------------------------------------------------------------------------------------

	int ch = uartGetByte();
	if (ch >= 0)
	{
		rprintf("%c", ch);	
		
		if (ch=='q' || ch == 'Q' ) 
		{
			//query mode
			for (BYTE id=0; id<16; id++)
			{
					ptmpA = PosRead(id);
					rprintf("%x", ptmpA);	
			}
			lbtmp=adc_psd();
			rprintf("%x", lbtmp);	
			lbtmp=adc_mic();
			rprintf("%x", lbtmp);	
			ptmpA=adc_volt();
			rprintf("%x", ptmpA);
			tilt_read(0);
			rprintf("%x", x_value);
			rprintf("%x", y_value);
			rprintf("%x  ", z_value);
			rprintf("%d:%d:%d-%d ",gHOUR,gMIN,gSEC,gMSEC);
			rprintf("\r\n");			
		}

		if (ch == '?') {
			rprintf("SlaveSerial Mode\r\n");
		}
		
		if (ch == 'p' || ch == 27 ) {
			rprintf("Exit SlaveSerial Mode\r\n");
			gNextMode = kIdleMode;
		}
		
		if (ch =='x' || ch=='X')        //'x' or 'X' pressed
		{
			// PC control mode
			// 2 hex digits = length
			
			#define MAX_INP_BUF 32
		
			int c;
			int l=getHex(2);
			int buff[MAX_INP_BUF];
			// foreach byte
			for (c=0; (c<l && c<MAX_INP_BUF); c++)
			{			
			//  read each hex digit into buffer
				buff[c]=getHex(2);
			}
						
			// transmit  buffer
			for (c=0; (c<l && c<MAX_INP_BUF); c++)
			{	
				sciTx0Data(buff[c]&0xFF);
			}

			if (ch==0x78)
			{
				// get response (or timeout)					
				BYTE b1 = sciRx0Ready();
				BYTE b2 = sciRx0Ready();
				// echo response
				rprintf ("=%x%x\r\n", b1,b2);
			}
			else
			{
				rprintf (" ok\r\n");
			}
		
		}
	}
} 


void slave_mainloop()
{
	WORD lMSEC;
	rprintf ("SlaveSerialMode\r\n");
	while (kSerialSlaveMode == gNextMode) {
		lMSEC = gMSEC;
		
		Do_Serial();
			
		_delay_ms(10);
	}
}
