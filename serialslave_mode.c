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
// Get a byte of data from the message in the uart (unescaping as necessary).
static u08 GetMsgByte(void)
{
	u08 ch;
	while (!uartReceiveByte(&ch));		// To-Do: add a timeout here
	if ('\\' == ch) {
		while (!uartReceiveByte(&ch));		// To-Do: and here
	}
	return ch;
}

//------------------------------------------------------------------------------
// Get a WORD of data from the message in the uart (unescaping as necessary).
static WORD GetMsgWord(void)
{
	u08 b0 = GetMsgByte();
	u08 b1 = GetMsgByte();
	return ((WORD)b1 << 8) | b0;
}

//------------------------------------------------------------------------------
// Skip the rest of the current message, by reading from the uart until we get
// a null byte not escaped by a backslash.
static void SkipMsgData()
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
	WORD startTicks;
	WORD profileTicks1, profileTicks2, profileTicks3;
	
	// Let's start by getting the data size to expect.	
	int bytes = GetMsgWord();
	
	if (bytes > kMaxMotionBufSize) {
		StartMsg('E');
		rprintf("Error: requested buffer size (%d) exceeds max (%d)\n", 
				bytes, kMaxMotionBufSize);
		EndMsg();
		return;
	}
	
	// Initialize our buffer.
	for (int i=0; i < kMaxMotionBufSize; i++) motionBuf[i] = 0xFF;
	nextRxIndex = 0;
	scenesReceived = 0;
	
	// Point the uart at our custom receive function.
	uartSetRxHandler( ReceiveIntoMotionBuf );	// receive directly into motionBuf
	
	// Let the host know we're ready.
	StartMsg('^');
	EndMsg();
	
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
	SendAck('^');
//	rprintf("Received %d bytes in %d ticks\n", nextRxIndex, profileTicks1 );

	// Start the motion.
	scenesReceived = motionBuf[0];  // To-do: calculate this from nextRxIndex
	startTicks = gTicks;
	LoadMotionFromBuffer( motionBuf );

	profileTicks2 = gTicks - startTicks;
	inMotion = TRUE;
	startTicks = gTicks;
	PlaySceneFromBuffer( motionBuf, 0 );
	profileTicks3 = gTicks - startTicks;
	
//	rprintf("Loaded in %d ticks; started in %d\n", profileTicks2, profileTicks3);
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
		// all done with the motion!
		// Send a notification that we're done with this motion...
		// currently, the data is the number of scenes; we might want
		// to change this to some other unique identifier that was
		// sent when the motion was loaded.
		StartMsg('d');
		AddMsgByte(motionBuf[0]);  // number of scenes
		EndMsg();

		gSceneIndex = -1;
		inMotion = FALSE;
		noRepeatLatch = FALSE;
		return;
	}

	// Start the next scene (if we've received it)
	if (gSceneIndex+1 < scenesReceived) {
		// Notify host that we're beginning a new scene.
		StartMsg('b');
		AddMsgByte(gSceneIndex);		// scene number we're starting
		AddMsgByte(motionBuf[0]);		// number of scenes
		EndMsg();

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
	// Phil: this needs to be rewritten for the new protocol.  I'm thinking
	// that we should simply take the message data bytes (extracted with
	// GetMsgByte) and stuff them directly into sciTx0Data.  But we might
	// also want a way for the host to specify how many response bytes it
	// expects, since leaving stale data on the sciRx buffer leads to problems
	// for other commands.
	

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
	
	u08 servoID = GetMsgByte();
	u08 torque = GetMsgByte();
	u08 position = GetMsgByte();
	SkipMsgData();	// skip rest of message (if any)
	
	if (torque <= 4) {
		wckPosSend(servoID, torque, position);
	} else {
		wckSetPassive(servoID);
	}
	
	SendAck('g');
}

//------------------------------------------------------------------------------
// Report all the servo positions and other status info.
void handle_get_status(void)
{
	// Currently this command takes no parameters...
	// in the future, we may want to send a mask indicating which sort
	// of data we're interested in receiving.
	SkipMsgData();

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
			SkipMsgData();
			break;

		case 'q':		// Query status
			handle_get_status();
			break;

		case 'g':		// servo Go to position
			handle_goto_position();
			break;
		
		case 'p':		// assume the basic Pose
			SendAck('p');
			SkipMsgData();
			BasicPose();
			break;
		
		case '.':		// read a byte from the wCK bus (just a debugging hack really)
			SkipMsgData();
			StartMsg('.');
			AddMsgByte(sciRx0Ready());
			EndMsg();
			break;
		
		case 'x':		// send data directly to the wCK bus
			handle_direct_ctl(TRUE);
			break;

		case 'm':		// request to load Motion
			handle_load_motion();
			break;
			
		case 27:		// Escape (exit serial slave mode)
			SendAck(27);
			SkipMsgData();
			gNextMode = kIdleMode;
			break;

		default:
			SendErr("Unknown message type");
			SkipMsgData();
		}
	}
	lastCh = ch;
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
