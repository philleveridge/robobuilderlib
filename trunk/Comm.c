//==============================================================================
//						Communication & Command Routines
//==============================================================================

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "global.h"
#include "Main.h"
#include "Macro.h"
#include "Comm.h"
#include "rprintf.h"
#include "HunoBasic.h"
#include "e-motion.h"    //extra-motions (gedit??)
#include <util/delay.h>

unsigned char PROGMEM wCK_IDs[16]={
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
};


// Frame variables-----------------------------------------------------------------
volatile WORD	gFrameIdx=0;	    // Frame counter
WORD	TxInterval=0;		// Timer1 interval
float	gUnitD[MAX_wCK];	// interpolation values
volatile WORD	gSIdx;				// Scene counter(0~65535)
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
	BYTE	PF;				//  ?
	BYTE	RIdx;			//  ?
	DWORD	AIdx;			//  ?
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


// external variables

volatile extern WORD		gMSEC;
volatile extern BYTE		gTx0Buf[];		// UART0 transmit buffer
volatile extern BYTE		gTx0Cnt;		// UART0 transmit length
volatile extern BYTE		gTx0BufIdx;		// UART0 transmit pointer
volatile extern BYTE		gRx0Cnt;		// UART0 receive length
volatile extern BYTE		gRx0Buf[];		// UART0 receive buffer
volatile extern BYTE 		F_PLAYING;		// state: playing from Flash

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
	BYTE CheckSum; 
	ID=(BYTE)(7<<5)|ID; 
	CheckSum = (ID^Data1^Data2^Data3)&0x7f;

	gTx0Buf[gTx0Cnt]=HEADER;
	gTx0Cnt++;		

	gTx0Buf[gTx0Cnt]=ID;
	gTx0Cnt++;		

	gTx0Buf[gTx0Cnt]=Data1;
	gTx0Cnt++;		

	gTx0Buf[gTx0Cnt]=Data2;
	gTx0Cnt++;		

	gTx0Buf[gTx0Cnt]=Data3;
	gTx0Cnt++;		

	gTx0Buf[gTx0Cnt]=CheckSum;
	gTx0Cnt++;		
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


//------------------------------------------------------------------------------
// Position Move command. also reads the current position
// Input	: ID, SpeedLevel, Position
// Output	: Current Position ( 0~254) (444 if no response)
// 
//------------------------------------------------------------------------------
WORD PosRead(BYTE ID) 
{
/*	BYTE	Data1, Data2;
	BYTE	CheckSum; 
	WORD	startT;
	
	
	Data1 = (5<<5) | ID;
	Data2 = 0;
	gRx0Cnt = 0;			// clear Rx pointer
	CheckSum = (Data1^Data2)&0x7f;
	sciTx0Data(HEADER);
	sciTx0Data(Data1);
	sciTx0Data(Data2);
	sciTx0Data(CheckSum);
	
	startT = gMSEC;
	while(gRx0Cnt<2){
        if(gMSEC<startT){ 	// wait for response packet or timeout
            if((1000 - startT + gMSEC)>RX_T_OUT)
            	return 448;	// return 444 if timeout
        }
		else if((gMSEC-startT)>RX_T_OUT) return 446;
	}
	
	WORD res = (gRx0Buf[0] << 8) | (gRx0Buf[1]);
	return res;
	
	*/
	
	return PosMove(ID, 5, 0);
} 


//------------------------------------------------------------------------------
// Position Move command. also reads the current position
// Input	: ID, SpeedLevel, Position
// Output	: Current Position ( 0~254) (444 if no response)
// 
//------------------------------------------------------------------------------
WORD PosMove(BYTE ID, BYTE torq, BYTE target) 
{
	BYTE	Data1;
	BYTE	CheckSum; 
	BYTE 	c;
	WORD 	res;
		
	Data1 = (torq<<5) | ID;
	gRx0Cnt = 0;			// clear Rx pointer
	CheckSum = (Data1^target)&0x7f;
	sciTx0Data(HEADER);
	sciTx0Data(Data1);
	sciTx0Data(target);
	sciTx0Data(CheckSum);
		

	
	c= sciRx0Ready();
	
	res = c;
	
	c= sciRx0Ready();
	
	res <<= 8;
	
	res  |= c;
	
	return res;
	
	/*
	startT = gMSEC;
	while(gRx0Cnt<2){
        if(gMSEC<startT){ 	// wait for response packet or timeout
            if((1000 - startT + gMSEC)>RX_T_OUT)
            	return 448;	// return 444 if timeout
        }
		else if((gMSEC-startT)>RX_T_OUT) return 446;
	}
	//return gRx0Buf[RX0_BUF_SIZE-1];
	WORD res = (gRx0Buf[0] << 8) | (gRx0Buf[1]);
	return res;
	*/
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

	for(i=0;i<MAX_wCK;i++){				// clear the wCK motion data
		Motion.wCK[i].Exist		= 0;
		Motion.wCK[i].RPgain	= 0;
		Motion.wCK[i].RDgain	= 0;
		Motion.wCK[i].RIgain	= 0;
		Motion.wCK[i].PortEn	= 0;
		Motion.wCK[i].InitPos	= 0;
	}
	for(i=0;i<Motion.NumOfwCK;i++){		// fill the wCK motion data
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].Exist		= 1;
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].RPgain	= pgm_read_byte(gpPg_Table+i);
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].RDgain	= pgm_read_byte(gpDg_Table+i);
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].RIgain	= pgm_read_byte(gpIg_Table+i);
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].PortEn	= 1;
		Motion.wCK[pgm_read_byte(&(wCK_IDs[i]))].InitPos	= pgm_read_byte(gpZero_Table+i);
	}
}


//------------------------------------------------------------------------------
// Set P,D,I parameters of each servo's PID loop from current Motion data.
// 	
//------------------------------------------------------------------------------
void SendTGain(void)
{
	WORD i;

	UCSR0B &= 0x7F;   		// UART0 Rx Interrupt disable
	UCSR0B |= 0x40;   		// UART0 Tx Interrupt enable

	while(gTx0Cnt);			// wait till buffer empty
	for(i=0;i<MAX_wCK;i++){					// Runtime P,D gain set from Motion structure
		if(Motion.wCK[i].Exist)				// set P,D if wCK exists
			SendSetCmd(i, 11, Motion.wCK[i].RPgain, Motion.wCK[i].RDgain);
	}
	gTx0BufIdx++;
	sciTx0Data(gTx0Buf[gTx0BufIdx-1]);		// Initiate the transmit


	while(gTx0Cnt);			// wait till buffer empty
	for(i=0;i<MAX_wCK;i++){					// Runtime I gain set from Motion structure
		if(Motion.wCK[i].Exist)				// set I if wCK exists
			SendSetCmd(i, 24, Motion.wCK[i].RIgain, Motion.wCK[i].RIgain);
	}
	gTx0BufIdx++;
	sciTx0Data(gTx0Buf[gTx0BufIdx-1]);		// Initiate the transmit
}


//------------------------------------------------------------------------------
// Send external data for a scene
// 		
//------------------------------------------------------------------------------
void SendExPortD(void)
{
	WORD i;

	UCSR0B &= 0x7F;   		// UART0 Rx Interrupt disable
	UCSR0B |= 0x40;   		// UART0 Tx Interrupt enable

	while(gTx0Cnt);			// wait till buffer empty
	for(i=0;i<MAX_wCK;i++){					// external data set from Motion structure
		if(Scene.wCK[i].Exist)				// set external data if wCK exists
			SendSetCmd(i, 100, Scene.wCK[i].ExPortD, Scene.wCK[i].ExPortD);
	}
	gTx0BufIdx++;
	sciTx0Data(gTx0Buf[gTx0BufIdx-1]);		// Initiate the transmit
}


//------------------------------------------------------------------------------
// Fill the scene data structure with the scene data from flash pointed to by gSIdx
// Uses:
//		gSIdx;			// current scene index
//		gpT_Table;		// Pointer to flash torque table
//		gpE_Table;		// Pointer to flash Port table
//		gpFN_Table;		// Pointer to flash frames table (int*)
//		gpRT_Table;		// Pointer to flash transition time table (int*)
//		gpPos_Table;	// Pointer to flash position table
//------------------------------------------------------------------------------
void GetSceneFromFlash(void)
{
	WORD i;
	
	Scene.NumOfFrame = pgm_read_word(gpFN_Table+(gSIdx * 2));	// get the number of frames in scene
	gNumOfFrame = Scene.NumOfFrame;
	Scene.RTime = pgm_read_word(gpRT_Table+(gSIdx * 2));		// get the run time of scene[msec]
	for(i=0;i<Motion.NumOfwCK;i++){			// clear the data for the wCK in scene
		Scene.wCK[i].Exist		= 0;
		Scene.wCK[i].SPos		= 0;
		Scene.wCK[i].DPos		= 0;
		Scene.wCK[i].Torq		= 0;
		Scene.wCK[i].ExPortD	= 0;
	}
	for(i=0;i<Motion.NumOfwCK;i++){			// get the flash data for the wCK
		Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].Exist		= 1;
		Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].SPos		= pgm_read_byte(gpPos_Table+(Motion.NumOfwCK*gSIdx+i));
		Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].DPos		= pgm_read_byte(gpPos_Table+(Motion.NumOfwCK*(gSIdx+1)+i));
		Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].Torq		= pgm_read_byte(gpT_Table+(Motion.NumOfwCK*gSIdx+i));
		Scene.wCK[pgm_read_byte(&(wCK_IDs[i]))].ExPortD		= pgm_read_byte(gpE_Table+(Motion.NumOfwCK*gSIdx+i));
	}
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
		gSIdx = i;
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
		
		rprintf ("%d %x, ", i, tmp);
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
