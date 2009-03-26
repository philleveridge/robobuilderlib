// serialslave_mode.c
//
//	In this mode, the robot is under close control of the host app over
//	the serial connection.  This is the mode you'd probably be using if
//	you control your robot (perhaps with a gamepad) in competition.

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

// Buffer for handling Motion commands
#define kMaxMotionBufSize 466  // enough for 8 scenes
//#define kMaxMotionBufSize 102  // enough for 1 scene
static unsigned char motionBuf[kMaxMotionBufSize];

//------------------------------------------------------------------------------
// Handle a motion buffer command.
//------------------------------------------------------------------------------
void handle_motion_cmd(void)
{
	u08 b0, b1;
	WORD bufSize;
	int pos = 0, i;
	WORD startSec = gSEC;
	
	// next two bytes are the buffer size, in little-endian order
	while (!uartReceiveByte(&b0));
	while (!uartReceiveByte(&b1));
	bufSize = b0 + ((WORD)b1 << 8);
	
	// now, read that buffer (To-do: time out after a while)
	while (pos < bufSize) {
		while (!uartReceiveByte(motionBuf+pos));
		pos++;
		if (gSEC > startSec+1) break;
	}

	rprintf("Got %d bytes... ", pos);
//	for (i=0; i<pos; i++) rprintf("%x ", motionBuf[i]);
	rprintf("\r\n");

	// OK, now we have data in our buffer, we need to start the
	// motion via the Comm module.
	LoadMotionFromBuffer(motionBuf);
	PlaySceneFromBuffer(motionBuf, 0);
	
}

//------------------------------------------------------------------------------
// Do_Serial
//
//	Handle one command (if there is any) in serial slave mode.
//------------------------------------------------------------------------------
void Do_Serial(void)
{
	BYTE	lbtmp;
	WORD	ptmpA;
	

	int ch = uartGetByte();
	if (ch < 0) return;

	rprintf("%c [%d]\r\n", ch, ch);	
	
	if (ch=='q' || ch == 'Q' ) 
	{
		// Query mode
		
		// Servo positions
		for (BYTE id=0; id<16; id++)
		{
				ptmpA = PosRead(id);
				rprintf("%x", ptmpA);	
		}

		// PSD (distance) sensor value
		lbtmp = adc_psd();
		rprintf("%x", lbtmp);
		
		// Microphone value
		lbtmp = adc_mic();
		rprintf("%x", lbtmp);	
		
		// Battery voltage
		ptmpA = adc_volt();
		rprintf("%x", ptmpA);
		
		// accelerometer
		tilt_read(0);
		rprintf("%x", x_value);
		rprintf("%x", y_value);
		rprintf("%x", z_value);
		
		// clock
		rprintf("  %d:%d:%d-%d ",gHOUR,gMIN,gSEC,gMSEC);
		rprintf("\r\n");			
	}

	if (ch == '?') {
		rprintf("Serial Slave mode\r\n");
	}
	
	if (ch == 'p' || ch == 27 ) {
		rprintf("Exit SlaveSerial Mode\r\n");
		gNextMode = kIdleMode;
	}
	
	if (ch == 'M') {
		handle_motion_cmd();
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


void serialslave_mainloop()
{
	WORD lMSEC;
	rprintf ("Serial Slave mode\r\n");
	while (kSerialSlaveMode == gNextMode) {
		lMSEC = gMSEC;
		
		Do_Serial();
			
		_delay_ms(10);
	}
}
