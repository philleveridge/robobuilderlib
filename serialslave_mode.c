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

extern int getHex(int d);

// Buffer for handling Motion commands
#define kMaxMotionBufSize 466  // enough for 8 scenes
static unsigned char motionBuf[kMaxMotionBufSize];
static volatile int nextRxIndex;	// index of next byte to receive
static WORD scenesReceived;		// number of scenes received
static BOOL inMotion = FALSE;	// TRUE when we're playing a motion

//------------------------------------------------------------------------------
// ReceiveIntoMotionBuf: uart Rx routine that stuffs data into motionBuf.
//------------------------------------------------------------------------------
void ReceiveIntoMotionBuf(unsigned char c)
{
	motionBuf[nextRxIndex++] = c;
}

//------------------------------------------------------------------------------
// print_motionBuf: debugging routine to dump motionBuf to the uart.
//------------------------------------------------------------------------------
void print_motionBuf(int bytes)
{
	// Print the bytes, for debugging purposes.
	rprintf("motionBuf: ");
	const unsigned char *hexDigits = "0123456789ABCDEF";
	for (int i = 0; i < bytes; i++) {
		unsigned char b = motionBuf[i];
		uartSendByte( hexDigits[ b >> 4 ] );
		uartSendByte( hexDigits[ b & 0x0F ] );
		uartSendByte( ' ' );
	}
}

//------------------------------------------------------------------------------
// handle_load_motion: the host has sent a very small command indicating the
// desire to send a motion block; the robot will prepare the buffer and uart,
// respond that it is ready, and then receive the entire motion block at once,
// directly into the motion buffer.
//------------------------------------------------------------------------------
void handle_load_motion()
{
	unsigned char b0, b1;
	WORD startTicks;
	WORD profileTicks1, profileTicks2, profileTicks3;
	
	// Let's start by getting the data size to expect, as a 2-byte LE integer.	
	while (!uartReceiveByte(&b0));
	while (!uartReceiveByte(&b1));
	int bytes = ((int)b1 << 8) | b0;
	
	if (bytes > kMaxMotionBufSize) {
		rprintf("Error: requested buffer size (%d) exceeds max (%d)\r\n", 
				bytes, kMaxMotionBufSize);
		return;
	}
	
	// Initialize our buffer.
	for (int i=0; i < kMaxMotionBufSize; i++) motionBuf[i] = 0xFF;
	nextRxIndex = 0;
	scenesReceived = 0;
	
	// Point the uart at our custom receive function.
	uartSetRxHandler( ReceiveIntoMotionBuf );	// receive directly into motionBuf
	
	// Let the host know we're ready.
	rprintf("Ready to receive %d bytes\r\n", bytes );
	uartSendByte('^');
	
	// Now, we should be receiving data directly into motion buf via
	// the interrupt handler.  So all we have to do is sit here and
	// watch nextRxIndex, until we've gotten all the data we need, or
	// we appear to have timed out.
	startTicks = gTicks;
	while (nextRxIndex < bytes) {
		if (gTicks - startTicks > 50) {
			// Timed out
			uartSetRxHandler( NULL );
			rprintf("Timed out after receiving %d bytes\r\n", nextRxIndex );
			return;
		}
	}
	profileTicks1 = gTicks - startTicks;
	
	// Reset the UART Rx handler, and acknowledge that we got the data.
	uartSetRxHandler( NULL );
	rprintf("Received %d bytes in %d ticks\r\n", nextRxIndex, profileTicks1 );

	// Start the motion.
	scenesReceived = motionBuf[0];  // To-do: calculate this from nextRxIndex
	startTicks = gTicks;
	LoadMotionFromBuffer( motionBuf );

	profileTicks2 = gTicks - startTicks;
	inMotion = TRUE;
	startTicks = gTicks;
	PlaySceneFromBuffer( motionBuf, 0 );
	profileTicks3 = gTicks - startTicks;
	
	rprintf("Loaded in %d ticks; started in %d\r\n", profileTicks2, profileTicks3);
}


//------------------------------------------------------------------------------
// continue_motion: keep our motion going by noticing when the current scene
// is done playing, and starting the next one (if any).
//------------------------------------------------------------------------------
void continue_motion()
{
	static BOOL noRepeatLatch = FALSE;
	
	if (!inMotion) return;		// not playing a motion
	if (F_PLAYING) return;		// in the middle of a scene
	
	if (gSceneIndex+1 >= motionBuf[0]) {
		// all done with the motion
		rprintf("Done with %d-scene motion\r\n", motionBuf[0]);
		gSceneIndex = -1;
		inMotion = FALSE;
		noRepeatLatch = FALSE;
		return;
	}

	// Start the next scene (if we've received it)
	if (gSceneIndex+1 < scenesReceived) {
		rprintf("Starting scene %d of %d\r\n", gSceneIndex+1, motionBuf[0]);
		PlaySceneFromBuffer(motionBuf, gSceneIndex+1);
		noRepeatLatch = FALSE;
	} else if (!noRepeatLatch) {
		rprintf("still waiting for scene %d of %d\r\n", gSceneIndex+1, motionBuf[0]);
		noRepeatLatch = TRUE;
	}
}

//------------------------------------------------------------------------------
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

	rprintf(" %c [%d]\r\n", ch, ch);	
	
	switch (ch) {
	case 'm':
		handle_load_motion();
		break;

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
	
	case 'd':
	case 'D':
		// Query position, decimal mode
		rprintf("Position: ");
		for (BYTE id=0; id<16; id++) {
			ptmpA = PosRead(id);
			rprintf("%d", ptmpA);
			if (id < 15) uartSendByte(','); else rprintf("\r\n");
		}
		break;
	
	case '?':
		rprintf("Serial Slave mode: inMotion=%d, F_PLAYING=%d\r\n", inMotion, F_PLAYING);
		break;
	
	case 'p':
	case 'P':
		rprintf("Basic pose\r\n");
		BasicPose();
		break;
	
	case '.':
		rprintf(" sciRx0Ready:%d\r\n", sciRx0Ready());
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


//------------------------------------------------------------------------------
void serialslave_mainloop()
{
	//WORD lMSEC;
	rprintf ("Serial Slave mode\r\n");
	while (kSerialSlaveMode == gNextMode) {
		//lMSEC = gMSEC;
		
		Do_Serial();
		continue_motion();
		//_delay_ms(10);  // why should we delay here?
	}
}
