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

// Communications protocol constant: the base of our set of
// commands for sending motion header and scenes
#define kMotionCmdBase 199

//------------------------------------------------------------------------------
// Receive the header (i.e. everything up to the first scene) of a motion
// buffer.  Send an acknowledgement, which triggers the host to send the
// first scene.
//------------------------------------------------------------------------------
void handle_motion_header()
{
	int i;
	int NumOfwCK;
	
	// Get the first two bytes, which are the number of scenes and 
	// the number of wCK servos.
	while (!uartReceiveByte(motionBuf));
	while (!uartReceiveByte(motionBuf+1));
	NumOfwCK = motionBuf[1];
	
	// Get the P gains, D gains, and I gains for the servos
	for (i = 0; i < NumOfwCK*3; i++) {
		while (!uartReceiveByte(motionBuf+2+i));
	}
	
	// Send an acknowledgement that we got the header data.
	uartSendByte('M');
	
	// Load the motion header into our motion-control code.
	LoadMotionFromBuffer(motionBuf);
}

//------------------------------------------------------------------------------
// Receive one scene of a motion buffer.  Send an acknowledgement, which
// triggers the host to send the next scene (if any).  If this is scene 0,
// then start the motion (trusting that we'll get the rest of the scenes
// before they're needed).
//------------------------------------------------------------------------------
void handle_scene(int sceneNum)
{
	// Calculate where this scene fits into our motion buffer
	int NumOfwCK = motionBuf[1];
	int sceneSize = 3 * NumOfwCK + 4;
	int startPos = 3 * NumOfwCK + 2 + sceneNum * sceneSize;
	int i;
	
	// Read the required number of bytes
	for (i = 0; i < sceneSize; i++) {
		while (!uartReceiveByte(motionBuf + startPos + i));
	}
	
	// Send an acknowledgement that we got the scene.
	uartSendByte('0' + sceneNum);
	
	// If this is scene 0, then start the motion.
	PlaySceneFromBuffer(motionBuf, 0);

/*	// Dump our buffer for debugging
	rprintf("buffer: ");
	const unsigned char *hexDigits = "0123456789ABCDEF";
	for (i = 0; i < startPos + sceneSize; i++) {
		unsigned char b = motionBuf[i];
		uartSendByte( hexDigits[ b >> 4 ] );
		uartSendByte( hexDigits[ b & 0x0F ] );
		uartSendByte( ' ' );
	}
*/

}


void handle_direct_ctl(BOOL showResponse)
{
	// PC control mode
	// 2 hex digits = length
	
	#define MAX_INP_BUF 32

	int c;
	int l=getHex(2);
	int buff[MAX_INP_BUF];
	// foreach byte
	for (c=0; (c<l && c<MAX_INP_BUF); c++) {			
		// read each hex digit into buffer
		buff[c]=getHex(2);
	}
				
	// transmit  buffer
	for (c=0; (c<l && c<MAX_INP_BUF); c++) {	
		sciTx0Data(buff[c]&0xFF);
	}

	if (showResponse) {
		// get response (or timeout)					
		BYTE b1 = sciRx0Ready();
		BYTE b2 = sciRx0Ready();
		// echo response
		rprintf ("=%x%x\r\n", b1,b2);
	} else {
		rprintf (" ok\r\n");
	}
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

	if (ch == kMotionCmdBase) {
		handle_motion_header();
		return;
	} else if (ch > kMotionCmdBase && ch <= kMotionCmdBase+8) {
		handle_scene(ch - kMotionCmdBase - 1);
		return;
	}
	
	switch (ch) {
	case 'q':
	case 'Q':
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
		break;
	
	case '?':
		rprintf("Serial Slave mode\r\n");
		break;
	
	case 'p':
	case 'P':
		rprintf("Basic pose\r\n");
		BasicPose();
		break;
	
	case 27:  // Esc
		rprintf("Exit SlaveSerial Mode\r\n");
		gNextMode = kIdleMode;
		break;

	case 'x':
	case 'X':
		handle_direct_ctl(ch=='x');
		break;
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
