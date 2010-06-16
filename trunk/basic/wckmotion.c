
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>

#include "global.h"
#include "main.h"
#include "Macro.h"
#include "rprintf.h"
#include <util/delay.h>

#include "wckmotion.h"
#include "HunoBasic.h"
#include "e-motion.h"    //extra-motions (gedit??)

extern void delay_ms(int);


unsigned char PROGMEM wCK_IDs[16]={
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
};

// UART variables-----------------------------------------------------------------
volatile BYTE	gTx0Buf[TX0_BUF_SIZE];		// UART0 transmit buffer
volatile BYTE	gTx0Cnt;					// UART0 transmit length
volatile BYTE	gRx0Cnt;					// UART0 receive length
volatile BYTE	gTx0BufIdx;					// UART0 transmit pointer
volatile BYTE	gRx0Buf[RX0_BUF_SIZE];		// UART0 receive buffer


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


//////////////////////////////// Serial Interface Functions ///////////////////////////

void wckReInit(unsigned int ubrr)
{
	while ((UCSR0A & (1 << TXC)) == 0) ; //wait until any transmission complete

	/* Dis-able receiver and transmitter */
	UCSR0B &= ~((1<<RXEN)|(1<<TXEN));

	/* Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	
	/* Set frame format: 8data, 2stop bit */
	//UCSR0C = (1<<USBS)|(3<<UCSZ0);

	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN)|(1<<TXEN);
}

//------------------------------------------------------------------------------
// Transmit char to UART when UART is ready
//------------------------------------------------------------------------------
void wckFlush( void )
{
	unsigned char dummy;
	while ( UCSR0A & (1<<RXC) ) dummy = UDR0;
}

//------------------------------------------------------------------------------
// Transmit char to UART when UART is ready
//------------------------------------------------------------------------------
void wckSendByte(char td)
{
	while(!(UCSR0A&(1<<UDRE))); 	// wait until data register is empty
	UDR0=td;
}

//------------------------------------------------------------------------------
// Get character when received. or timeout
//------------------------------------------------------------------------------
extern volatile WORD	mstimer;

int wckGetByte(WORD timeout)
{
	mstimer = timeout;
	
	while(!(UCSR0A&(1<<RXC)) ){ 	// test for received character
		if (mstimer==0) return -1;
	}
	return UDR0;
}

//////////////////////////////// Definition of Basic Functions ///////////////////////////

/******************************************************************************/
/* Function that sends Operation Command Packet(4 Byte) to wCK module */
/* Input : Data1, Data2 */
/* Output : None */
/******************************************************************************/
void wckSendOperCommand(char Data1, char Data2)
{
	char CheckSum;
	CheckSum = (Data1^Data2)&0x7f;
	wckSendByte(HEADER);
	wckSendByte(Data1);
	wckSendByte(Data2);
	wckSendByte(CheckSum);
}
/******************************************************************************/
/* Function that sends Set Command Packet(6 Byte) to wCK module */
/* Input : Data1, Data2, Data3, Data4 */
/* Output : None */
/******************************************************************************/
void wckSendSetCommand(char Data1, char Data2, char Data3, char Data4)
{
	char CheckSum;
	CheckSum = (Data1^Data2^Data3^Data4)&0x7f;
	wckSendByte(HEADER);
	wckSendByte(Data1);
	wckSendByte(Data2);
	wckSendByte(Data3);
	wckSendByte(Data4);
	wckSendByte(CheckSum);
}
/*************************************************************************************************/
/* Function that sends Position Move Command to wCK module */
/* Input : ServoID, Torque (0(Max) to 4 (Min)), Position */
/* Output : Load * 256 + Position */
/*************************************************************************************************/
WORD wckPosSend(char ServoID, char Torque, char Position)
{
	WORD Load, curPosition;
	wckSendOperCommand((Torque<<5)|ServoID, Position);
	Load = wckGetByte(TIME_OUT1);
	curPosition = wckGetByte(TIME_OUT1);
	return (Load << 8) | curPosition;
}

/************************************************************************************************/
/* Function that sends Position Read Command to wCK module, and returns the Position. */
/* Input : ServoID */
/* Output : Position */
/************************************************************************************************/
int wckPosRead(char ServoID)
{
	int Position;
	wckSendOperCommand(0xa0|ServoID, 0x00);
	wckGetByte(TIME_OUT1);
	Position = wckGetByte(TIME_OUT1);
	return Position;
}


/******************************************************************************/
/* Function that sends Passive wCK Command to wCK module */
/* Input : ServoID */
/* Output : Position */
/******************************************************************************/
char wckSetPassive(char ServoID)
{
	char Position;
	wckSendOperCommand(0xc0|ServoID, 0x10);
	wckGetByte(TIME_OUT1);
	Position = wckGetByte(TIME_OUT1);
	return Position;
}

/*************************************************************************/
/* Function that sends Break wCK Command to wCK module */
/* Input : None */
/* Output : ServoID if succeed, 0xff if fail */
/**************************************************************************/
char wckPowerDown(void)
{
	char ServoID;
	wckSendOperCommand(0xdf, 0x20);
	ServoID = wckGetByte(TIME_OUT1);
	wckGetByte(TIME_OUT1);
	if(ServoID<31) 
		return ServoID;
	return 0xff; //Receive error
}
/******************************************************************/
/* Function that sends 360 degree Wheel wCK Command */
/* Input : ServoID, SpeedLevel, RotationDir */
/* Return : Rotation Number */
/*****************************************************************/
char wckRotation360(char ServoID, char SpeedLevel, char RotationDir)
{
	char RotNum;
	if (RotationDir == ROTATE_CCW) {
		wckSendOperCommand((6<<5)|ServoID, (ROTATE_CCW<<4)|SpeedLevel);
	} else if (RotationDir == ROTATE_CW) {
		wckSendOperCommand((6<<5)|ServoID, (ROTATE_CW<<4)|SpeedLevel);
	}
	RotNum = wckGetByte(TIME_OUT1);
	wckGetByte(TIME_OUT1);
	return RotNum;
}
/*****************************************************************************/
/* Function that sends Synchronized Position Move Command to wCK module */
/* Input : LastID, SpeedLevel, *TargetArray, Index */
/* Return : None */
/****************************************************************************/
void wckSyncPosSend(char LastID, char SpeedLevel, char *TargetArray, char Index)
{
	int i;
	char CheckSum;
	i = 0;
	CheckSum = 0;
	wckSendByte(HEADER);
	wckSendByte((SpeedLevel<<5)|0x1f);
	wckSendByte(LastID+1);
	while(1) 
	{
		if(i>LastID) 
			break;
		wckSendByte(TargetArray[Index*(LastID+1)+i]);
		CheckSum = CheckSum ^ TargetArray[Index*(LastID+1)+i];
		i++;
	}
	CheckSum = CheckSum & 0x7f;
	wckSendByte(CheckSum);
}
/********************************************************************/
/* Function that sends Baud rate Set Command to wCK module */
/* Input : ServoID, NewBaud */
/* Return : New Baudrate if succeed, 0xff if fail */
/********************************************************************/
char wckBaudrateSet(char ServoID, char NewBaud)
{
	wckSendSetCommand((7<<5)|ServoID, 0x08, NewBaud, NewBaud);
	wckGetByte(TIME_OUT2);
	if(wckGetByte(TIME_OUT2)==NewBaud) 
		return NewBaud;
	return 0xff;
}
/*********************************************************************/
/* Function that sends Gain Set Command to wCK module */
/* Input : ServoID, *NewPgain, *NewDgain */
/* Return : 1 if succeed, 0 if fail */
/********************************************************************/
char wckGainSet(char ServoID, char *NewPgain, char *NewDgain)
{
	char Data1,Data2;
	wckSendSetCommand((7<<5)|ServoID, 0x09, *NewPgain, *NewDgain);
	Data1 = wckGetByte(TIME_OUT2);
	Data2 = wckGetByte(TIME_OUT2);
	if((Data1==*NewPgain) && (Data2==*NewDgain)) 
		return 1;
	return 0;
}
/************************************************************/
/* Function that sends ID Set Command to wCK module */
/* Input : ServoID, NewId */
/* Return : New ID if succeed, 0xff if fail */
/***********************************************************/
char wckIdSet(char ServoID, char NewId)
{
	wckSendSetCommand((7<<5)|ServoID, 0x0a, NewId, NewId);
	wckGetByte(TIME_OUT2);
	if(wckGetByte(TIME_OUT2)==NewId) 
		return NewId;
	return 0xff;
}
/******************************************************************/
/* Function that sends Gain Read Command to wCK module */
/* Input : ServoID, *NewPgain, *NewDgain */
/* Return : 1 if succeed, 0 if fail */
/*****************************************************************/
char wckGainRead(char ServoID, char *Pgain, char *Dgain)
{
	wckSendSetCommand((7<<5)|ServoID, 0x0c, 0, 0);
	*Pgain = wckGetByte(TIME_OUT1);
	*Dgain = wckGetByte(TIME_OUT1);
	if((*Pgain>0) && (*Pgain<51) && (*Dgain<101)) 
		return 1;
	return 0;
}
/**********************************************************************************/
/* Function that sends Over Load Set Command to wCK module */
/* Input : ServoID, NewOverCT */
/* Return : New Overcurrent Threshold if succeed, 0xff if fail */
/**********************************************************************************/
char wckOverCTSet(char ServoID, char NewOverCT)
{
	char Data1;
	wckSendSetCommand((7<<5)|ServoID, 0x0f, NewOverCT, NewOverCT);
	wckGetByte(TIME_OUT2);
	Data1=wckGetByte(TIME_OUT2);
	if(Data1!=0xff) 
		return Data1;
	return 0xff;
}
/******************************************************************************/
/* Function that sends Over Load Read Command to wCK module */
/* Input : ServoID */
/* Return : Overcurrent Threshold if succeed, 0xff if fail */
/******************************************************************************/
char wckOverCTRead(char ServoID)
{
	char Data1;
	wckSendSetCommand((7<<5)|ServoID, 0x10, 0, 0);
	wckGetByte(TIME_OUT1);
	Data1=wckGetByte(TIME_OUT1);
	if(Data1!=0xff) 
		return Data1;
	return 0xff;
}
/***********************************************************************/
/* Function that sends Boundary Set Command to wCK module */
/* Input : ServoID, *NewLBound, *NewUBound */
/* Return : 1 if succeed, 0 if fail */
/**********************************************************************/
char wckBoundSet(char ServoID, char *NewLBound, char *NewUBound)
{
	char Data1,Data2;
	wckSendSetCommand((7<<5)|ServoID, 0x11, *NewLBound, *NewUBound);
	Data1 = wckGetByte(TIME_OUT2);
	Data2 = wckGetByte(TIME_OUT2);
	if((Data1==*NewLBound) && (Data2==*NewUBound)) 
		return 1;
	return 0;
}
/**************************************************************************/
/* Function that sends Boundary Read Command to wCK module */
/* Input : ServoID, *NewLBound, *NewUBound */
/* Return : 1 if succeed, 0 if fail */
/*************************************************************************/
char wckBoundRead(char ServoID, char *LBound, char *UBound)
{
	wckSendSetCommand((7<<5)|ServoID, 0x12, 0, 0);
	*LBound = wckGetByte(TIME_OUT1);
	*UBound = wckGetByte(TIME_OUT1);
	if(*LBound<*UBound) 
		return 1;
	return 0;
}
//////////////////////////////// End of Basic Functions Definition /////////////////////////////


//------------------------------------------------------------------------------
// Send a Synchronized Position Send Command for the scene and frame defined
// by our Scene global, gFrameIdx, and gUnitD.
// (Actually, just stuff such a command into our transmit buffer, gTx0Buf;
// the actual sending of this buffer is done elsewhere.)
//------------------------------------------------------------------------------
static void SyncPosSend(void) 
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
static void GetMotionFromFlash(void)
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
// Set P,D,I parameters of each servo's PID loop from current Motion data.
// 	
//------------------------------------------------------------------------------
void SendTGain(void)
{
	char reply1, reply2;
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

	for(i=0;i<MAX_wCK;i++){					// external data set from Motion structure
		if(Scene.wCK[i].Exist) {			// set external data if wCK exists
			wckSendSetCommand((7<<5)|i, 100, Scene.wCK[i].ExPortD, Scene.wCK[i].ExPortD);

			wckGetByte(TIME_OUT2);
			wckGetByte(TIME_OUT2);			
		}
	}
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
static void GetSceneFromFlash(void)
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
// Set the Timer1 interrupt based on the number of frames and the run time of the scene
// 		
//------------------------------------------------------------------------------
static void CalcFrameInterval(void)
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
static void CalcUnitMove(void)
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
static void MakeFrame(void)
{
	while(gTx0Cnt);			// wait until the transmit buffer is empty
	gFrameIdx++;			// next frame
	SyncPosSend();			// build new frame
}


//------------------------------------------------------------------------------
// Start sending the frame (or whatever else happens to be in our transmit
// buffer, gTx0Buf).
//------------------------------------------------------------------------------
static void SendFrame(void)
{
	if(gTx0Cnt==0)	return;	// return if no frame to send
	gTx0BufIdx++;
	wckSendByte(gTx0Buf[gTx0BufIdx-1]);		// send first byte to start frame send
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
		//while(F_PLAYING)
		//	process_frames();		// wait till scene interpolation complete
	}
}


//------------------------------------------------------------------------------
// Check the F_NEXTFRAME flag, and if it's set (which is done by an interrupt
// scheduled to fire at the desired frame interval), make and send the next
// frame of the current scene.
//------------------------------------------------------------------------------
void process_frames()
{
	if (F_NEXTFRAME) {
		MakeFrame();
		SendFrame();
		F_NEXTFRAME = 0;
	}
}


//------------------------------------------------------------------------------
// Assume the basic standing position.
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
	wckSendByte(gTx0Buf[gTx0BufIdx-1]);		// send first byte to start frame send
	
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


/************************************************************************/


// if flag set read initial positions

static BYTE cpos[32];
 BYTE nos=0;
 
int getservo(int id)
{
	wckSendOperCommand(0xa0|id, NULL);
	int b1 = wckGetByte(TIME_OUT1);
	int b2 = wckGetByte(TIME_OUT1);
	
	if (b1<0 || b2<0)
		return -1;
	else
		return b2;
}

BYTE readservos()
{
	BYTE i;
	for (i=0; i<31; i++)
	{
		int p = wckPosRead(i);		
		if (p<0 || p>255) break;
		cpos[i]=p;	
	}
	rprintf("Servos: %d\r\n", i);	
	nos=i;
	return i;
}


// Play d ms per step, f frames, from current -> pos
void PlayPose(int d, int f, BYTE pos[], int flag)
{
	int i;	
	if (flag!=0) 
	{
		readservos();	// set nos and reads cpos
		nos=flag;
	}
	
	float intervals[nos];	
	
	int dur=d/f;
	if (dur<25) dur=25; //25ms is quickest
	
	for (i=0; i<nos; i++)
	{
		intervals[i]=(float)(pos[i]-cpos[i])/(float)f;
	}
	
	for (i=0; i<f; i++)
	{
		BYTE temp[nos];
		for (int j=0; j<nos; j++)
		{
			temp[j] = cpos[j] + (float)((i)*intervals[j]+0.5);
		}
		wckSyncPosSend(nos-1, 4, temp, 0);
		delay_ms(dur);
	}
}

const BYTE basic18[] = { 143, 179, 198,  83, 106, 106,  69,  48, 167, 141,  47,  47,  49, 199, 192, 204, 122, 125};
const BYTE basic16[] = { 125, 179, 199, 88, 108, 126, 72, 49, 163, 141, 51, 47, 49, 199, 205, 205 };

void standup (int n) 
{
	if (n<18)
		PlayPose(1000, 10, basic16, 16); //huno basic
	else
		PlayPose(1000, 10, basic18, 18); //huno with hip
}


/************************************************************************/
//
// Support for Cyclon head - communicates at 9600 baud on wck BUS
//
/************************************************************************/

#define BR9600		95 
#define BR115200	7 
void delay_ms(int);

void send_bus_str(char *bus_str, int n)
{
			
		BYTE b;
		int ch;
		char *eos = bus_str+n;

		wckReInit(BR9600);
	
		while  ((bus_str<eos) && (b=*bus_str++) != 0)
		{			
			wckSendByte('S');
			wckSendByte(b);
			
			if (b=='p' || b=='t')
			{
				delay_ms(100);	
				if (*bus_str != 0) wckSendByte(*bus_str++);
				delay_ms(100);	
				if (*bus_str != 0) wckSendByte(*bus_str++);
				
			}		
			delay_ms(100);		
			ch = wckGetByte(1000);
			rprintf ("BUS=%d\r\n", ch);
		}
		
		wckReInit(BR115200);
		wckFlush(); // flush the buffer
}

/************************************************************************/
//
// Support for sound chip
//
/************************************************************************/

WORD Sound_Length[25]={
2268,1001,832,365,838,417,5671,5916,780,2907,552,522,1525,2494,438,402,433,461,343,472,354,461,458,442,371};

void SendToSoundIC(BYTE cmd) 
{
	BYTE	CheckSum; 
	
	if (cmd <1 || cmd>25)
	{
		wckSendByte(0x00);
		wckSendByte(0xff);
		return;
	}
	
	TIMSK |= 0x01;
	EIMSK |= 0x40;
		
	PWR_LED2_ON;	// RED on
	UCSR0B = (1<<RXEN)|(1<<TXEN); //enable reads for GetPos !!

	CheckSum = (29^cmd)&0x7f;
	wckSendByte(0xFF);
	delay_ms(1);

	wckSendByte(29);

	delay_ms(1);
	wckSendByte(cmd);
	delay_ms(1);
	wckSendByte(CheckSum);
	
	PWR_LED2_OFF; // RED off	
		
	//wait for response?
	
	PWR_LED1_ON;	// RED on
	
	WORD timeo = Sound_Length[cmd-1] + 200;
	
	int b1 = wckGetByte(timeo);
	
	wckSendByte(b1);
	wckSendByte(b1);
	
	UCSR0B = (1<<RXEN)|(1<<TXEN) |(1<<RXCIE); //enable reads for GetPos !!
	PWR_LED1_OFF; // RED off	
} 

void sound_init()
{
	// low -> high PIN
	// defined in main.h
	P_BMC504_RESET(0);
	delay_ms(20);
	P_BMC504_RESET(1);
}