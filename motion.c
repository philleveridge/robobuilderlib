//==============================================================================
//						Communication & Command Routines
//==============================================================================

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "global.h"
#include "main.h"
#include "Macro.h"
#include "motion.h"
#include "rprintf.h"
#include "HunoBasic.h"
#include "e-motion.h"    //extra-motions (gedit??)
#include "wck.h"
#include <util/delay.h>

unsigned char PROGMEM wCK_IDs[16]={
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
};


// Frame variables-----------------------------------------------------------------
volatile WORD	gFrameIdx=0;	    // Frame counter
WORD	TxInterval=0;				// Timer1 interval
float	gUnitD[MAX_wCK];			// interpolation values
volatile WORD	gSceneIndex = -1;	// current scene playing, or -1 if none
WORD	gNumOfFrame;

// Program Memory pointers ------------------------------------------------------------------------
// (All are unsigned char*, except where noted below)

PGM_P			gpT_Table;		// Pointer to flash torque table
PGM_P			gpE_Table;		// Pointer to flash Port table
PGM_P			gpPg_Table;		// Pointer to flash runtime P table
PGM_P			gpDg_Table;		// Pointer to flash runtime D table
PGM_P			gpIg_Table;		// Pointer to flash runtime I table
PGM_P			gpZero_Table;	// Pointer to flash zero table
PGM_P			gpFN_Table;		// Pointer to flash frames table (int*)
PGM_P			gpRT_Table;		// Pointer to flash transition time table (int*)
PGM_P			gpPos_Table;	// Pointer to flash position table
//--------------------------------------------------------------------------------------------------

// Data structure for the Motion Data-------------------------------------------------------------
//      - hierarchy is : wCK < Frame < Scene < Motion < Action
//      - data is sent to the wCK for each frame on the timer
//      - frames are created from scenes by interpolation
//      - a motion comprises a sequence of scenes
//      - an action can invoke motions

struct TwCK_in_Motion{      // Structure for each wCK in a motion
	BYTE	Exist;			// 1 if wCK exists
	BYTE	RPgain;			// Runtime P setting
	BYTE	RDgain;			// Runtime D setting
	BYTE	RIgain;			// Runtime I setting
	BYTE	PortEn;			// (0 = disable, 1 = enable)
	BYTE	InitPos;		// Initial wCK position (apparently ignored)
};

struct TwCK_in_Scene{		// Structure for each wCK in a scene
	BYTE	Exist;			// 1 if wCK exists
	BYTE	SPos;			// wCK starting position
	BYTE	DPos;			// wCK destination position
	BYTE	Torq;			// Torque
	BYTE	ExPortD;		// External Port Data(1~3)
};

struct TMotion{			    // Structure for a motion
	BYTE	PF;				//  ? (not used)
	BYTE	RIdx;			//  ? (not used)
	DWORD	AIdx;			//  ? (not used)
	WORD	NumOfScene;		// number of scenes in motion
	WORD	NumOfwCK;		// number of wCK included
	struct	TwCK_in_Motion  wCK[MAX_wCK];	// Motion data for each wCK
	WORD	FileSize;		// overall file size
}Motion;

struct TScene{			    // Structure for a scene
	WORD	Idx;			// index of scene (0~65535)
	WORD	NumOfFrame;		// number of frames in scene
	WORD	RTime;			// running time of scene[msec]
	struct	TwCK_in_Scene   wCK[MAX_wCK];	// scene data for each wCK
}Scene;

//------------------------------------------------------------------------------
//  Motion Buffer Layout (where C is number of wCK)
//  
//  Offset  Length  Description
//  ------  ------  -----------
//  0       1       number of scenes
//  1       1       number of wCK servos (C)
//  2       C       P gain for each wCK
//  C+2     C       D gain for each wCK
//  2*C+2   C       I gain for each wCK
//  3*C+2   3*C+4   data for first scene (scene 0)
//  6*C+6   3*C+4   data for scene 1
//  9*C+10  3*C+4   data for scene 2
//  (3+3*i)*C+(4*i)+2		start of data for scene i
//  (etc.)
//  
//  For each scene, the data has the following structure:
//  
//  Offset  Length  Description
//  ------  ------  -----------
//  0       2       scene transition (run) time, in milliseconds
//  2       2       desired number of interpolation frames (1-100)
//  4       C       destination position for each wCK
//  C+4     C       torque (0-4) for each wCK
//  2*C+4   C       external port data (LEDs?) for each wCK
//  3*C+4	(end)
//------------------------------------------------------------------------------



// internal method declarations
static void ClearMotionData(void);


//------------------------------------------------------------------------------
// Transmit char to UART when UART is ready
//------------------------------------------------------------------------------
void sciTx0Data(BYTE td)
{
	while(!(UCSR0A&(1<<UDRE))); 	// wait until data register is empty
	UDR0=td;
}

void sciTx1Data(BYTE td)
{
	while(!(UCSR1A&(1<<UDRE))); 	// wait until data register is empty
	UDR1=td;
}


//------------------------------------------------------------------------------
// Get character when received. or timeout
//------------------------------------------------------------------------------
BYTE sciRx0Ready(void)
{
	WORD	startT;
	startT = gMSEC;
	while(!(UCSR0A&(1<<RXC)) ){ 	// test for received character
        if(gMSEC<startT){
			// wait RX_T_OUT for a character
            if((1000 - startT + gMSEC)>RX_T_OUT) break;
        }
		else if((gMSEC-startT)>RX_T_OUT) break;
	}
	return UDR0;
}

BYTE sciRx1Ready(void)
{
	WORD	startT;
	startT = gMSEC;
	while(!(UCSR1A&(1<<RXC)) ){ 	// test for received character
        if(gMSEC<startT){
			// wait RX_T_OUT for a character
            if((1000 - startT + gMSEC)>RX_T_OUT) break;
        }
		else if((gMSEC-startT)>RX_T_OUT) break;
	}
	return UDR1;
}


//------------------------------------------------------------------------------
// Load a set command to wCK into our transmit buffer (gTx0Buf).
// Input	: ID, Data1, Data2, Data3
// Output	: None
//------------------------------------------------------------------------------
void SendSetCmd(BYTE ID, BYTE Data1, BYTE Data2, BYTE Data3)
{
	wckSendSetCommand((7<<5)|ID, Data1, Data2, Data3);
}


//------------------------------------------------------------------------------
// Send a Synchronized Position Send Command for the scene and frame defined
// by our Scene global, gFrameIdx, and gUnitD.
// (Actually, just stuff such a command into our transmit buffer, gTx0Buf;
// the actual sending of this buffer is done elsewhere.)
//------------------------------------------------------------------------------
void SyncPosSend(void) 
{
	int lwtmp;
	BYTE CheckSum; 
	BYTE i, tmp, Data;

	Data = (Scene.wCK[0].Torq<<5) | 31; // get the torque for the scene

	gTx0Buf[gTx0Cnt]=HEADER;
	gTx0Cnt++;		

	gTx0Buf[gTx0Cnt]=Data;
	gTx0Cnt++;		

	gTx0Buf[gTx0Cnt]=16;  // This is the (last ID - 1) why is it hardcoded ?
	gTx0Cnt++;		

	CheckSum = 0;
	for(i=0;i<MAX_wCK;i++){	// for all wCK 
		if(Scene.wCK[i].Exist){	// if wCK exists add the interpolation step
			lwtmp = (int)Scene.wCK[i].SPos + (int)((float)gFrameIdx*gUnitD[i]);
			if(lwtmp>254)		lwtmp = 254; // bound result 1 to 254
			else if(lwtmp<1)	lwtmp = 1;
			tmp = (BYTE)lwtmp;
			gTx0Buf[gTx0Cnt] = tmp;
			gTx0Cnt++;			// put into transmit buffer
			CheckSum = CheckSum^tmp;
		}
	}
	CheckSum = CheckSum & 0x7f;

	gTx0Buf[gTx0Cnt]=CheckSum;
	gTx0Cnt++;			// put into transmit buffer
} 

void set_break_mode()
{
	sciTx0Data(HEADER);
	sciTx0Data(0xDF);
	sciTx0Data(0x20);		
	sciTx0Data((0xDF^0x20)&0x7f);
	
	sciRx0Ready();
	sciRx0Ready();
	return;
}


void ClearMotionData(void)
{
	WORD i;
	for (i = 0; i < MAX_wCK; i++) {				// clear the wCK motion data
		Motion.wCK[i].Exist		= 0;
		Motion.wCK[i].RPgain	= 0;
		Motion.wCK[i].RDgain	= 0;
		Motion.wCK[i].RIgain	= 0;
		Motion.wCK[i].PortEn	= 0;
		Motion.wCK[i].InitPos	= 0;
	}
}


//------------------------------------------------------------------------------
// Fill the motion data structure from flash.  Uses:
//		gpPg_Table: pointer to array of P gain component for each servo in flash
//		gpDp_Table: pointer to array of D gain component for each servo in flash
//		gpIg_Table: pointer to array of I gain component for each servo in flash
//		gpZero_Table: pointer to servo initial positions (ultimately ignored)
//------------------------------------------------------------------------------
void GetMotionFromFlash(void)
{
	WORD i;

	ClearMotionData();
	
	for(i=0;i<Motion.NumOfwCK;i++){		// fill the wCK motion data
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].Exist	= 1;
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].RPgain	= pgm_read_byte(gpPg_Table+i);
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].RDgain	= pgm_read_byte(gpDg_Table+i);
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].RIgain	= pgm_read_byte(gpIg_Table+i);
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].PortEn	= 1;
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].InitPos = pgm_read_byte(gpZero_Table+i);
	}
}

//------------------------------------------------------------------------------
// Fill the motion data structure from a buffer in RAM.
//------------------------------------------------------------------------------
void GetMotionFromBuffer(unsigned char *motionBuf)
{
	WORD i;
	unsigned char *pGains;
	unsigned char *dGains;
	unsigned char *iGains;
	
	ClearMotionData();
	
	Motion.NumOfScene = motionBuf[0];  // (See "Motion Buffer Layout" at top of file)
	Motion.NumOfwCK = motionBuf[1];
	
	//rprintf("GMBF: %d, %d\n", Motion.NumOfScene, Motion.NumOfwCK );

	pGains = motionBuf + 2;
	dGains = pGains + Motion.NumOfwCK;
	iGains = dGains + Motion.NumOfwCK;
	
	for (i = 0; i < Motion.NumOfwCK; i++) {		// fill the wCK motion data
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].Exist	= 1;
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].RPgain	= pGains[i];
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].RDgain	= dGains[i];
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].RIgain	= iGains[i];
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].PortEn	= 1;
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].InitPos = pgm_read_byte(gpZero_Table+i);
	}
}


//------------------------------------------------------------------------------
// Set P,D,I parameters of each servo's PID loop from current Motion data.
// 	
//------------------------------------------------------------------------------
void SendTGain(void)
{
	char reply1, reply2;
	WORD TIME_OUT2 = 250;
	for (int i=0; i<MAX_wCK;i++) {
		if (Motion.wCK[i].Exist) {
			char servoID = i;
			wckSendSetCommand((7<<5)|servoID, 11, Motion.wCK[i].RPgain, Motion.wCK[i].RDgain);
			reply1 = wckGetByte(TIME_OUT2);		// should match the P gain we just set
			reply2 = wckGetByte(TIME_OUT2);		// should match the D gain we just set

			wckSendSetCommand((7<<5)|servoID, 24, Motion.wCK[i].RIgain, Motion.wCK[i].RIgain);
			reply1 = wckGetByte(TIME_OUT2);		// P gain again
			reply2 = wckGetByte(TIME_OUT2);		// D gain again
		}
	}
}


//------------------------------------------------------------------------------
// Send external data for a scene
// 		
//------------------------------------------------------------------------------
void SendExPortD(void)
{
	WORD i;
	WORD TIME_OUT2 = 250;

/*	UCSR0B &= 0x7F;   		// UART0 Rx Interrupt disable
	UCSR0B |= 0x40;   		// UART0 Tx Interrupt enable

	while(gTx0Cnt);			// wait till buffer empty
*/
	for(i=0;i<MAX_wCK;i++){					// external data set from Motion structure
		if(Scene.wCK[i].Exist) {			// set external data if wCK exists
			SendSetCmd(i, 100, Scene.wCK[i].ExPortD, Scene.wCK[i].ExPortD);
			wckGetByte(TIME_OUT2);
			wckGetByte(TIME_OUT2);			
		}
	}
/*	gTx0BufIdx++;
	sciTx0Data(gTx0Buf[gTx0BufIdx-1]);		// Initiate the transmit
*/
}


//------------------------------------------------------------------------------
// Fill the scene data structure with the scene data from flash pointed to by gSceneIndex
// Uses:
//		gSceneIndex;			// current scene index
//		gpT_Table;		// Pointer to flash torque table
//		gpE_Table;		// Pointer to flash Port table
//		gpFN_Table;		// Pointer to flash frames table (int*)
//		gpRT_Table;		// Pointer to flash transition time table (int*)
//		gpPos_Table;	// Pointer to flash position table
//------------------------------------------------------------------------------
void GetSceneFromFlash(void)
{
	WORD i;
	
	Scene.NumOfFrame = pgm_read_word(gpFN_Table+(gSceneIndex * 2));	// get the number of frames in scene
	gNumOfFrame = Scene.NumOfFrame;
	Scene.RTime = pgm_read_word(gpRT_Table+(gSceneIndex * 2));		// get the run time of scene[msec]
	for(i=0;i<Motion.NumOfwCK;i++){			// clear the data for the wCK in scene
		Scene.wCK[i].Exist		= 0;
		Scene.wCK[i].SPos		= 0;
		Scene.wCK[i].DPos		= 0;
		Scene.wCK[i].Torq		= 0;
		Scene.wCK[i].ExPortD	= 0;
	}
	for(i=0;i<Motion.NumOfwCK;i++){			// get the flash data for the wCK
		Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].Exist		= 1;
		Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].SPos		= pgm_read_byte(gpPos_Table+(Motion.NumOfwCK*gSceneIndex+i));
		Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].DPos		= pgm_read_byte(gpPos_Table+(Motion.NumOfwCK*(gSceneIndex+1)+i));
		Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].Torq		= pgm_read_byte(gpT_Table+(Motion.NumOfwCK*gSceneIndex+i));
		Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].ExPortD		= pgm_read_byte(gpE_Table+(Motion.NumOfwCK*gSceneIndex+i));
	}
	UCSR0B &= 0x7F;   		// UART0 RxInterrupt disable
	UCSR0B |= 0x40;   		// UART0 TxInterrupt enable
	
	

	_delay_us(1300);
}

//------------------------------------------------------------------------------
// Fill the scene data structure with scene data from the given motion buffer.
// Uses:
//		Motion			current motion header data
//		gSceneIndex			current scene index
//------------------------------------------------------------------------------
void GetSceneFromBuffer(unsigned char *motionBuffer)
{
	WORD i, NumOfwCK;
	unsigned char *sceneBuffer;
	unsigned char *prevScene;

	NumOfwCK = Motion.NumOfwCK;
	sceneBuffer = motionBuffer + (3 + 3 * gSceneIndex) * NumOfwCK + (4 * gSceneIndex) + 2;
		
	Scene.NumOfFrame = *((WORD*)(sceneBuffer+2));	// get the number of frames in scene
	gNumOfFrame = Scene.NumOfFrame;
			
	Scene.RTime = *((WORD*)(sceneBuffer+0));		// get the run time of scene[msec]
	
	// Get the starting position for each wCK -- this will be the previous
	// scene destination, unless we're on scene 0, in which case we'll
	// assume the starting positions have been set by LoadMotionFromBuffer.
	if (gSceneIndex > 0) {
		prevScene = motionBuffer + (3 + 3 * (gSceneIndex-1)) * NumOfwCK + (4 * (gSceneIndex-1)) + 2;
		for (i = 0; i < Motion.NumOfwCK; i++) {						
			Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].SPos = prevScene[ 4 + i ];
		}		
	}

	// Now get rest of the scene data for each wCK, including the destination position.
	for (i = 0; i < NumOfwCK; i++) {						
		Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].Exist	= 1;
		Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].DPos	= sceneBuffer[ 4 + i ];
		Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].Torq	= sceneBuffer[ NumOfwCK + 4 + i ];
		Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].ExPortD	= sceneBuffer[ 2 * NumOfwCK + 4 + i ];

	//rprintf("Servo %d: %d to %d\r\n", i, Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].SPos,
	//	Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].DPos);
	}
	
	// Serial port preparations (?).
	UCSR0B &= 0x7F;   		// UART0 RxInterrupt disable
	UCSR0B |= 0x40;   		// UART0 TxInterrupt enable
	
	_delay_us(1300);
}



//------------------------------------------------------------------------------
// Set the Timer1 interrupt based on the number of frames and the run time of the scene
// 		
//------------------------------------------------------------------------------
void CalcFrameInterval(void)
{
	float tmp;
	if((Scene.RTime / Scene.NumOfFrame)<20){ // is each scene < 20 ms ?
		return;
	}
	tmp = (float)Scene.RTime * 14.4;		// Timer  is clocked at 14.4KHz
	tmp = tmp  / (float)Scene.NumOfFrame;
	TxInterval = 65535 - (WORD)tmp - 43;

	RUN_LED1_ON;
	F_PLAYING=1;		// set flag to say we are busy playing frames
	TCCR1B=0x05;		// clock on div 1024

	if(TxInterval<=65509)	
		TCNT1=TxInterval+26;
	else
		TCNT1=65535;

	TIFR |= 0x04;		// Clear the overflow flag
	TIMSK |= 0x04;		// Timer1 Overflow Interrupt Enable
}


//------------------------------------------------------------------------------
// Calculate the interpolation steps
// gUnitD[] is in range -254 to +254
//------------------------------------------------------------------------------
void CalcUnitMove(void)
{
	WORD i;

	for(i=0;i<MAX_wCK;i++){
		if(Scene.wCK[i].Exist){	// if the wCK exists
			if(Scene.wCK[i].SPos!=Scene.wCK[i].DPos){
				// if any movement is required
				gUnitD[i] = (float)((int)Scene.wCK[i].DPos-(int)Scene.wCK[i].SPos);
				gUnitD[i] = (float)(gUnitD[i]/Scene.NumOfFrame);
				if(gUnitD[i]>253)	gUnitD[i]=254;
				else if(gUnitD[i]<-253)	gUnitD[i]=-254;
			}
			else
				gUnitD[i] = 0;
		}
	}
	gFrameIdx=0;				// reset frame to start of scene
}


//------------------------------------------------------------------------------
// Build a frame to send: that is, stuff the next frame into our transmit
// buffer.
//------------------------------------------------------------------------------
void MakeFrame(void)
{
	while(gTx0Cnt);			// wait until the transmit buffer is empty
	gFrameIdx++;			// next frame
	SyncPosSend();			// build new frame
}


//------------------------------------------------------------------------------
// Start sending the frame (or whatever else happens to be in our transmit
// buffer, gTx0Buf).
//------------------------------------------------------------------------------
void SendFrame(void)
{
	if(gTx0Cnt==0)	return;	// return if no frame to send
	gTx0BufIdx++;
	sciTx0Data(gTx0Buf[gTx0BufIdx-1]);		// send first byte to start frame send
}


//------------------------------------------------------------------------------
// Load and play a motion that's already partially loaded into our Motion
// global data (Motion.NumOfScene and Motion.NumOfwCK), and partially defined
// by a bunch of global pointers into flash data (gpPos_Table, gpT_Table, etc.).
// This method blocks until the whole motion is done playing.
//------------------------------------------------------------------------------
void M_PlayFlash(void)
{
	WORD i;

	GetMotionFromFlash();		// Get the motion data from flash
	SendTGain();				// set the runtime P,D and I from motion structure
	for(i=0;i<Motion.NumOfScene;i++){
		gSceneIndex = i;
		GetSceneFromFlash();	// Get the scene data from flash
		SendExPortD();			// Set external port data
		CalcFrameInterval();	// Set the interrupt for the frames
		CalcUnitMove();			// Calculate the interpolation steps
		MakeFrame();			// build a frame to send
		SendFrame();			// start sending frame
		while(F_PLAYING);		// wait till scene interpolation complete
	}
}

//------------------------------------------------------------------------------
// Load a motion from a buffer in RAM, and return immediately.
//------------------------------------------------------------------------------
void LoadMotionFromBuffer(unsigned char *motionBuf)
{
	GetMotionFromBuffer( motionBuf );	// Load motion data into our Motion global
	
	// Initialize the starting positions of the first scene
	// to the current servo positions
	for (int i = 0; i < Motion.NumOfwCK; i++) {
		BYTE id = pgm_read_byte(&(wCK_IDs[i]));
		Scene.wCK[id].SPos = wckPosRead(id);
		
		//rprintf("SPos %d = %d\r\n", id, Scene.wCK[id].SPos);
	}
	
 	SendTGain();						// set the runtime PID gain from motion structure
}

//------------------------------------------------------------------------------
// Begin playing one scene of the motion previously loaded with
// LoadMotionFromBuffer.  Return immediately, without waiting for it to finish.
//------------------------------------------------------------------------------
void PlaySceneFromBuffer(unsigned char *motionBuf, WORD sceneIndex)
{
	gSceneIndex = sceneIndex;
	GetSceneFromBuffer( motionBuf );	// Load scene into global structure
//	SendExPortD();						// Set external port data (commented out for now -- not needed)
	CalcFrameInterval();				// Set the interrupt for the frames
	CalcUnitMove();						// Calculate the interpolation steps
	MakeFrame();						// build a frame to send
	SendFrame();						// start sending frame
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
void BasicPose()
{
	PF1_LED1_ON;
	
	UCSR0B &= 0x7F;   		// UART0 Rx Interrupt disable
	UCSR0B |= 0x40;   		// UART0 Tx Interrupt enable

	
	BYTE CheckSum; 
	BYTE i, tmp, Data;

	Data = (3<<5) | 31; // get the torque for the scene

	gTx0Buf[gTx0Cnt]=HEADER;
	gTx0Cnt++;		

	gTx0Buf[gTx0Cnt]=Data;
	gTx0Cnt++;		

	gTx0Buf[gTx0Cnt]=16;  // This is the (last ID - 1) why is it hardcoded ?
	gTx0Cnt++;		

	CheckSum = 0;
	for(i=0;i<16;i++){				// for all wCK 		
		tmp = pgm_read_byte((PGM_P) HunoBasicPose +i);
		
		gTx0Buf[gTx0Cnt] = tmp;
		gTx0Cnt++;						// put into transmit buffer
		CheckSum = CheckSum^tmp;
		
		rprintf (" %d %x,", i, tmp);
	}
	CheckSum = CheckSum & 0x7f;

	gTx0Buf[gTx0Cnt]=CheckSum;
	gTx0Cnt++;								// put into transmit buffer

	gTx0BufIdx++;
	sciTx0Data(gTx0Buf[gTx0BufIdx-1]);		// send first byte to start frame send
	
	PF1_LED1_OFF;
	rprintf ("\r\n");
}



//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------

struct FlashMotionData {
	PGM_P TT;
	PGM_P ET;
	PGM_P PT;
	PGM_P DT;
	PGM_P IT;
	PGM_P FT;
	PGM_P RT;
	PGM_P PoT;
	int NoS;
	int Now;
};

struct FlashMotionData mlist[] = 
{
	{ // 0. PunchLeft
		(PGM_P) HunoBasic_PunchLeft_Torque,
		(PGM_P) HunoBasic_PunchLeft_Port,
		(PGM_P) HunoBasic_PunchLeft_PGain,
		(PGM_P) HunoBasic_PunchLeft_DGain,
		(PGM_P) HunoBasic_PunchLeft_IGain,
		(PGM_P) HunoBasic_PunchLeft_Frames,
		(PGM_P) HunoBasic_PunchLeft_TrTime,
		(PGM_P) HunoBasic_PunchLeft_Position,
		HUNOBASIC_PUNCHLEFT_NUM_SCENES,
		HUNOBASIC_PUNCHLEFT_NUM_MOTORS
	},	
	{ // 1. PunchRight
		(PGM_P) HunoBasic_PunchRight_Torque,
		(PGM_P) HunoBasic_PunchRight_Port,
		(PGM_P) HunoBasic_PunchRight_PGain,
		(PGM_P) HunoBasic_PunchRight_DGain,
		(PGM_P) HunoBasic_PunchRight_IGain,
		(PGM_P) HunoBasic_PunchRight_Frames,
		(PGM_P) HunoBasic_PunchRight_TrTime,
		(PGM_P) HunoBasic_PunchRight_Position,
		HUNOBASIC_PUNCHRIGHT_NUM_SCENES,
		HUNOBASIC_PUNCHRIGHT_NUM_MOTORS
	},
	// 2. SidewalkLeft
	{
		(PGM_P) HunoBasic_SidewalkLeft_Torque,
		(PGM_P) HunoBasic_SidewalkLeft_Port,
		(PGM_P) HunoBasic_SidewalkLeft_PGain,
		(PGM_P) HunoBasic_SidewalkLeft_DGain,
		(PGM_P) HunoBasic_SidewalkLeft_IGain,
		(PGM_P) HunoBasic_SidewalkLeft_Frames,
		(PGM_P) HunoBasic_SidewalkLeft_TrTime,
		(PGM_P) HunoBasic_SidewalkLeft_Position,
		HUNOBASIC_SIDEWALKLEFT_NUM_SCENES,
		HUNOBASIC_SIDEWALKLEFT_NUM_MOTORS
	},
	// 3. SidewalkRight
	{
		(PGM_P) HunoBasic_SidewalkRight_Torque,
		(PGM_P) HunoBasic_SidewalkRight_Port,
		(PGM_P) HunoBasic_SidewalkRight_PGain,
		(PGM_P) HunoBasic_SidewalkRight_DGain,
		(PGM_P) HunoBasic_SidewalkRight_IGain,
		(PGM_P) HunoBasic_SidewalkRight_Frames,
		(PGM_P) HunoBasic_SidewalkRight_TrTime,
		(PGM_P) HunoBasic_SidewalkRight_Position,
		HUNOBASIC_SIDEWALKRIGHT_NUM_SCENES,
		HUNOBASIC_SIDEWALKRIGHT_NUM_MOTORS
	},
	// 4. TurnLeft
	{
		(PGM_P) HunoBasic_TurnLeft_Torque,
		(PGM_P) HunoBasic_TurnLeft_Port,
		(PGM_P) HunoBasic_TurnLeft_PGain,
		(PGM_P) HunoBasic_TurnLeft_DGain,
		(PGM_P) HunoBasic_TurnLeft_IGain,
		(PGM_P) HunoBasic_TurnLeft_Frames,
		(PGM_P) HunoBasic_TurnLeft_TrTime,
		(PGM_P) HunoBasic_TurnLeft_Position,
		HUNOBASIC_TURNLEFT_NUM_SCENES,
		HUNOBASIC_TURNLEFT_NUM_MOTORS
	},	
	// 5. TurnRight
	{
		(PGM_P) HunoBasic_TurnRight_Torque,
		(PGM_P) HunoBasic_TurnRight_Port,
		(PGM_P) HunoBasic_TurnRight_PGain,
		(PGM_P) HunoBasic_TurnRight_DGain,
		(PGM_P) HunoBasic_TurnRight_IGain,
		(PGM_P) HunoBasic_TurnRight_Frames,
		(PGM_P) HunoBasic_TurnRight_TrTime,
		(PGM_P) HunoBasic_TurnRight_Position,
		HUNOBASIC_TURNRIGHT_NUM_SCENES,
		HUNOBASIC_TURNRIGHT_NUM_MOTORS
	},
	// 6. GetupBack
	{
		(PGM_P) HunoBasic_GetupBack_Torque,
		(PGM_P) HunoBasic_GetupBack_Port,
		(PGM_P) HunoBasic_GetupBack_PGain,
		(PGM_P) HunoBasic_GetupBack_DGain,
		(PGM_P) HunoBasic_GetupBack_IGain,
		(PGM_P) HunoBasic_GetupBack_Frames,
		(PGM_P) HunoBasic_GetupBack_TrTime,
		(PGM_P) HunoBasic_GetupBack_Position,
		HUNOBASIC_GETUPBACK_NUM_SCENES,
		HUNOBASIC_GETUPBACK_NUM_MOTORS
	},
	// 7. GetupFront
	{
		(PGM_P) HunoBasic_GetupFront_Torque,
		(PGM_P) HunoBasic_GetupFront_Port,
		(PGM_P) HunoBasic_GetupFront_PGain,
		(PGM_P) HunoBasic_GetupFront_DGain,
		(PGM_P) HunoBasic_GetupFront_IGain,
		(PGM_P) HunoBasic_GetupFront_Frames,
		(PGM_P) HunoBasic_GetupFront_TrTime,
		(PGM_P) HunoBasic_GetupFront_Position,
		HUNOBASIC_GETUPFRONT_NUM_SCENES,
		HUNOBASIC_GETUPFRONT_NUM_MOTORS
	},
	// 8. WalkForward
		{
		(PGM_P) HunoBasic_WalkForward_Torque,
		(PGM_P) HunoBasic_WalkForward_Port,
		(PGM_P) HunoBasic_WalkForward_PGain,
		(PGM_P) HunoBasic_WalkForward_DGain,
		(PGM_P) HunoBasic_WalkForward_IGain,
		(PGM_P) HunoBasic_WalkForward_Frames,
		(PGM_P) HunoBasic_WalkForward_TrTime,
		(PGM_P) HunoBasic_WalkForward_Position,
		HUNOBASIC_WALKFORWARD_NUM_SCENES,
		HUNOBASIC_WALKFORWARD_NUM_MOTORS
	},
	// 9. WalkBackward
	{
		(PGM_P) HunoBasic_WalkBackward_Torque,
		(PGM_P) HunoBasic_WalkBackward_Port,
		(PGM_P) HunoBasic_WalkBackward_PGain,
		(PGM_P) HunoBasic_WalkBackward_DGain,
		(PGM_P) HunoBasic_WalkBackward_IGain,
		(PGM_P) HunoBasic_WalkBackward_Frames,
		(PGM_P) HunoBasic_WalkBackward_TrTime,
		(PGM_P) HunoBasic_WalkBackward_Position,
		HUNOBASIC_WALKBACKWARD_NUM_SCENES,
		HUNOBASIC_WALKBACKWARD_NUM_MOTORS
	},
	
	
	// 10. E-motion LSHOOT
	
	{
		(PGM_P) LSHOOT_Torque,
		(PGM_P) LSHOOT_Port,
		(PGM_P) LSHOOT_RuntimePGain,
		(PGM_P) LSHOOT_RuntimeDGain,
		(PGM_P) LSHOOT_RuntimeIGain,
		(PGM_P) LSHOOT_Frames,
		(PGM_P) LSHOOT_TrTime,
		(PGM_P) LSHOOT_Position,
		LSHOOT_NUM_OF_SCENES,
		LSHOOT_NUM_OF_WCKS
	},	
	
	// 10. E-motion RSHOOT
	{
		(PGM_P) RSHOOT_Torque,
		(PGM_P) RSHOOT_Port,
		(PGM_P) RSHOOT_RuntimePGain,
		(PGM_P) RSHOOT_RuntimeDGain,
		(PGM_P) RSHOOT_RuntimeIGain,
		(PGM_P) RSHOOT_Frames,
		(PGM_P) RSHOOT_TrTime,
		(PGM_P) RSHOOT_Position,
		RSHOOT_NUM_OF_SCENES,
		RSHOOT_NUM_OF_WCKS
	},	
	
	// 11. E-motion RSIDEWALK
	{
		(PGM_P) RSIDEWALK_Torque,
		(PGM_P) RSIDEWALK_Port,
		(PGM_P) RSIDEWALK_RuntimePGain,
		(PGM_P) RSIDEWALK_RuntimeDGain,
		(PGM_P) RSIDEWALK_RuntimeIGain,
		(PGM_P) RSIDEWALK_Frames,
		(PGM_P) RSIDEWALK_TrTime,
		(PGM_P) RSIDEWALK_Position,
		RSIDEWALK_NUM_OF_SCENES,
		RSIDEWALK_NUM_OF_WCKS
	},	
	
	// 12. E-motion LSIDEWALK
	{
		(PGM_P) LSIDEWALK_Torque,
		(PGM_P) LSIDEWALK_Port,
		(PGM_P) LSIDEWALK_RuntimePGain,
		(PGM_P) LSIDEWALK_RuntimeDGain,
		(PGM_P) LSIDEWALK_RuntimeIGain,
		(PGM_P) LSIDEWALK_Frames,
		(PGM_P) LSIDEWALK_TrTime,
		(PGM_P) LSIDEWALK_Position,
		LSIDEWALK_NUM_OF_SCENES,
		LSIDEWALK_NUM_OF_WCKS
	},	
	
	// 13. E-motion STANDUPR
	{
		(PGM_P) STANDUPR_Torque,
		(PGM_P) STANDUPR_Port,
		(PGM_P) STANDUPR_RuntimePGain,
		(PGM_P) STANDUPR_RuntimeDGain,
		(PGM_P) STANDUPR_RuntimeIGain,
		(PGM_P) STANDUPR_Frames,
		(PGM_P) STANDUPR_TrTime,
		(PGM_P) STANDUPR_Position,
		STANDUPR_NUM_OF_SCENES,
		STANDUPR_NUM_OF_WCKS
	},	
	
	// 14. E-motion STANDUPF
	{
		(PGM_P) STANDUPF_Torque,
		(PGM_P) STANDUPF_Port,
		(PGM_P) STANDUPF_RuntimePGain,
		(PGM_P) STANDUPF_RuntimeDGain,
		(PGM_P) STANDUPF_RuntimeIGain,
		(PGM_P) STANDUPF_Frames,
		(PGM_P) STANDUPF_TrTime,
		(PGM_P) STANDUPF_Position,
		STANDUPF_NUM_OF_SCENES,
		STANDUPF_NUM_OF_WCKS
	},	
	
	// 15. E-motion HUNODEMO_SITDOWN
	{
		(PGM_P) HUNODEMO_SITDOWN_Torque,
		(PGM_P) HUNODEMO_SITDOWN_Port,
		(PGM_P) HUNODEMO_SITDOWN_RuntimePGain,
		(PGM_P) HUNODEMO_SITDOWN_RuntimeDGain,
		(PGM_P) HUNODEMO_SITDOWN_RuntimeIGain,
		(PGM_P) HUNODEMO_SITDOWN_Frames,
		(PGM_P) HUNODEMO_SITDOWN_TrTime,
		(PGM_P) HUNODEMO_SITDOWN_Position,
		HUNODEMO_SITDOWN_NUM_OF_SCENES,
		HUNODEMO_SITDOWN_NUM_OF_WCKS
	},	
	
	// 16. E-motion HUNODEMO_HI
	{
		(PGM_P) HUNODEMO_HI_Torque,
		(PGM_P) HUNODEMO_HI_Port,
		(PGM_P) HUNODEMO_HI_RuntimePGain,
		(PGM_P) HUNODEMO_HI_RuntimeDGain,
		(PGM_P) HUNODEMO_HI_RuntimeIGain,
		(PGM_P) HUNODEMO_HI_Frames,
		(PGM_P) HUNODEMO_HI_TrTime,
		(PGM_P) HUNODEMO_HI_Position,
		HUNODEMO_HI_NUM_OF_SCENES,
		HUNODEMO_HI_NUM_OF_WCKS
	},	
	
	// 17. E-motion HUNODEMO_KICKLEFTFRONTTURN
	{
		(PGM_P) HUNODEMO_KICKLEFTFRONTTURN_Torque,
		(PGM_P) HUNODEMO_KICKLEFTFRONTTURN_Port,
		(PGM_P) HUNODEMO_KICKLEFTFRONTTURN_RuntimePGain,
		(PGM_P) HUNODEMO_KICKLEFTFRONTTURN_RuntimeDGain,
		(PGM_P) HUNODEMO_KICKLEFTFRONTTURN_RuntimeIGain,
		(PGM_P) HUNODEMO_KICKLEFTFRONTTURN_Frames,
		(PGM_P) HUNODEMO_KICKLEFTFRONTTURN_TrTime,
		(PGM_P) HUNODEMO_KICKLEFTFRONTTURN_Position,
		HUNODEMO_KICKLEFTFRONTTURN_NUM_OF_SCENES,
		HUNODEMO_KICKLEFTFRONTTURN_NUM_OF_WCKS
	},	
	
	// 18. E-motion HANDSTANDS1
	{
		(PGM_P) HANDSTANDS1_Torque,
		(PGM_P) HANDSTANDS1_Port,
		(PGM_P) HANDSTANDS1_RuntimePGain,
		(PGM_P) HANDSTANDS1_RuntimeDGain,
		(PGM_P) HANDSTANDS1_RuntimeIGain,
		(PGM_P) HANDSTANDS1_Frames,
		(PGM_P) HANDSTANDS1_TrTime,
		(PGM_P) HANDSTANDS1_Position,
		HANDSTANDS1_NUM_OF_SCENES,
		HANDSTANDS1_NUM_OF_WCKS
	}				
	
	};



void SampleMotion(int sm)	// Perform SampleMotion(s)
{
	PF2_LED_ON;
	gpT_Table			= mlist[sm].TT;
	gpE_Table			= mlist[sm].ET;
	gpPg_Table 			= mlist[sm].PT;
	gpDg_Table 			= mlist[sm].DT;
	gpIg_Table 			= mlist[sm].IT;
	gpFN_Table			= mlist[sm].FT;
	gpRT_Table			= mlist[sm].RT;
	gpPos_Table			= mlist[sm].PoT;
	gpZero_Table		= (PGM_P) MotionZeroPos;
	Motion.NumOfScene 	= mlist[sm].NoS;
	Motion.NumOfwCK 	= mlist[sm].Now;
	
	M_PlayFlash();
}
