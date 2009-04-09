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
#include "wck.h"

#include <util/delay.h>

extern int getHex(int d);
extern int getDec(void);

// Buffer for handling Motion commands
#define kMaxMotionBufSize 466  // enough for 8 scenes

//modified (removed static) to allow access from basic.c - 
unsigned char motionBuf[kMaxMotionBufSize];


static volatile int nextRxIndex;	// index of next byte to receive
static WORD scenesReceived;		// number of scenes received
static BOOL inMotion = FALSE;	// TRUE when we're playing a motion

//------------------------------------------------------------------------------
// Send an acknowledgement that we received and handled the last
// message of the given type.
static void SendAck(u08 msgType) 
{
	rprintf("\xBE\xAD%c%c%c", 'k', msgType, 0);
}

//------------------------------------------------------------------------------
// Send a message header for the given message type.
static void StartMsg(u08 msgType)
{
	uartSendByte(0xBE);
	uartSendByte(0xAD);
	uartSendByte(msgType);
}

//------------------------------------------------------------------------------
// Send the message ending delimiter.
static void EndMsg(void)
{
	uartSendByte(0);
}

//------------------------------------------------------------------------------
// Add a byte of data to the message (properly escaped as necessary).
static void AddMsgByte(u08 b)
{
	if (!b) uartSendByte('\\');
	uartSendByte(b);
}

//------------------------------------------------------------------------------
// Add a word of data to the message (in Little-Endian format).
static void AddMsgWord(WORD w)
{
	AddMsgByte(w & 0xFF);
	AddMsgByte(w >> 8);
}

//------------------------------------------------------------------------------
// Send an error message.
static void SendErr(const char *errMsg) 
{
	StartMsg('E');
	rprintfStr((char*)errMsg);
	EndMsg();
	
	// Note: this approach doesn't work...
	//	rprintf("\xBE\xAD%c%s%c", 'E', errMsg, 0);
	// ...apparently because rprintf doesn't actually implement %s properly.
}

//------------------------------------------------------------------------------
// Skip the rest of the current message, by reading from the uart until we get
// a null byte not escaped by a backslash.
static void SkipMessageData()
{
	// To-do: add a timeout mechanism here.
	u08 ch;
	while (1) {
		while (!uartReceiveByte(&ch)) ;
		if ('\\' == ch) {
			// backslash found; skip the next byte and continue
			while (!uartReceiveByte(&ch)) ;
			continue;
		}
		if (!ch) return;	// null found; end of message
	}
}

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
		rprintf("Error: requested buffer size (%d) exceeds max (%d)\n", 
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
	rprintf("Ready to receive %d bytes\n", bytes );
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
			rprintf("Timed out after receiving %d bytes\n", nextRxIndex );
			return;
		}
	}
	profileTicks1 = gTicks - startTicks;
	
	// Reset the UART Rx handler, and acknowledge that we got the data.
	uartSetRxHandler( NULL );
	rprintf("Received %d bytes in %d ticks\n", nextRxIndex, profileTicks1 );

	// Start the motion.
	scenesReceived = motionBuf[0];  // To-do: calculate this from nextRxIndex
	startTicks = gTicks;
	LoadMotionFromBuffer( motionBuf );

	profileTicks2 = gTicks - startTicks;
	inMotion = TRUE;
	startTicks = gTicks;
	PlaySceneFromBuffer( motionBuf, 0 );
	profileTicks3 = gTicks - startTicks;
	
	rprintf("Loaded in %d ticks; started in %d\n", profileTicks2, profileTicks3);
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
		rprintf("Done with %d-scene motion\n", motionBuf[0]);
		gSceneIndex = -1;
		inMotion = FALSE;
		noRepeatLatch = FALSE;
		return;
	}

	// Start the next scene (if we've received it)
	if (gSceneIndex+1 < scenesReceived) {
		rprintf("Starting scene %d of %d\n", gSceneIndex+1, motionBuf[0]);
		PlaySceneFromBuffer(motionBuf, gSceneIndex+1);
		noRepeatLatch = FALSE;
	} else if (!noRepeatLatch) {
		rprintf("still waiting for scene %d of %d\n", gSceneIndex+1, motionBuf[0]);
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
		rprintf ("=%x%x\n", b1,b2);
	} else {
		rprintf (" ok\n");
	}
}

//------------------------------------------------------------------------------
void handle_goto_position(void)
{
	// three binary parameters, one byte each:
	// Servo ID, torque (max. 0, min. 4), position (0-255)
	// Note: send 0xFF for torque to set the servo to passive mode
	
	u08 servoID, torque, position;
	while (!uartReceiveByte(&servoID)) ;
	while (!uartReceiveByte(&torque)) ;
	while (!uartReceiveByte(&position)) ;
	
	if (torque <= 4) {
		wckPosSend(servoID, torque, position);
	} else {
		wckSetPassive(servoID);
	}
}

//------------------------------------------------------------------------------
// Report all the servo positions and other status info.
void handle_get_status(void)
{
	// Currently this command takes no parameters...
	// in the future, we may want to send a mask indicating which sort
	// of data we're interested in receiving.
	SkipMessageData();

	StartMsg('q');

	// Start with servo positions.
	for (BYTE id=0; id<16; id++) {
		AddMsgByte( wckPosRead(id) );
	}

	// PSD (distance) sensor value
	AddMsgWord( adc_psd() );
	
	// Microphone value
	AddMsgWord( adc_mic() );
	
	// Battery voltage
	AddMsgWord( adc_volt() );
	
	// accelerometer
	tilt_read(0);
	AddMsgWord( x_value );
	AddMsgWord( y_value );
	AddMsgWord( z_value );
	
	// clock
	AddMsgByte( gHOUR );
	AddMsgByte( gMIN );
	AddMsgByte( gSEC );
	AddMsgWord( gMSEC );
	AddMsgWord( gTicks );

	EndMsg();

}

//------------------------------------------------------------------------------
// Do_Serial
//
//	Handle one command (if there is any) in serial slave mode.
//------------------------------------------------------------------------------
void Do_Serial(void)
{
//	BYTE	lbtmp;
//	WORD	ptmpA;
	
	u08 ch;
	if (!uartReceiveByte(&ch)) return;

	static u08 lastCh = 0;	
	if (lastCh == 0xBE && ch == 0xAD) {
		// Got magic word that indicates the start of a message.
		// Assume the rest of the message is coming very quickly now.
		// First we get the type code...
		u08 msgType;
		while (!uartReceiveByte(&msgType)) ;  // To-do: add a timeout here

		// Now, dispatch to a handler based on the type
		// (except for any trivial ones we can handle here).
		switch (msgType) {

		case '?':		// Are-you-there?
			SendAck('?');
			SkipMessageData();
			break;

		case 'q':		// Query status
			handle_get_status();
			break;

		default:
			SendErr("Unknown message type");
			SkipMessageData();
		}
	}
	lastCh = ch;
/*	
	int ch = uartGetByte();
	if (ch < 0) return;
	
	switch (ch) {
	case '?':
		rprintf("Serial Slave mode: inMotion=%d, F_PLAYING=%d\n", inMotion, F_PLAYING);
		rprintf("  m: load motion (binary)\n");
		rprintf("  q: query robot status\n");
		rprintf("  d: query servo positions in decimal form\n");
		rprintf("  g: Goto (set servo position and torque)\n");
		rprintf("  p: basic pose\n");
		rprintf("  .: read one byte from wCK bus\n");
		rprintf("  x: load data directly into wCK bus\n");
		rprintf(" <ESC>: exit Serial Slave mode\n");
		break;
	
	case 'm':
		handle_load_motion();
		break;

	case 'g':
		handle_goto_position();
		break;
		
	case 'q':
	case 'Q':
		// Query mode
		uartSendByte('Q');
		
		// Servo positions
		for (BYTE id=0; id<16; id++)
		{
				ptmpA = wckPosAndLoadRead(id);
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
		rprintf("\n");			
		break;
	
	case 'd':
	case 'D':
		// Query position, decimal mode
		rprintf("Position: ");
		for (BYTE id=0; id<16; id++) {
			ptmpA = wckPosRead(id);
			rprintf("%d", ptmpA);
			if (id < 15) uartSendByte(','); else rprintf("\n");
		}
		break;
	
	case 'p':
	case 'P':
		rprintf(" Basic pose\n");
		BasicPose();
		break;
	
	case '.':
		rprintf(" sciRx0Ready:%d\n", sciRx0Ready());
		break;
	
	case 27:  // Esc
		rprintf("Exit SlaveSerial Mode\n");
		gNextMode = kIdleMode;
		break;

	case 'x':
	case 'X':
		handle_direct_ctl(ch=='x');
		break;
	
	default:
		rprintf(" Unknown command '%c' [%d]\n", ch, ch);
	}
*/
} 


//------------------------------------------------------------------------------
void serialslave_mainloop()
{
	//WORD lMSEC;
	rprintf ("Serial Slave mode\n");
	while (kSerialSlaveMode == gNextMode) {
		//lMSEC = gMSEC;
		
		Do_Serial();
		continue_motion();
		//_delay_ms(10);  // why should we delay here?
	}
}
