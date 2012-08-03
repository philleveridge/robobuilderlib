#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>

#include "global.h"
#include "main.h"
#include "macro.h"
#include "rprintf.h"
#include <util/delay.h>

#include "wckmotion.h"
#include "HunoBasic.h"
#include "e-motion.h"    //extra-motions (gedit??)

extern void delay_ms(int);

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
	wckGetByte(TIME_OUT2);
	Position = wckGetByte(TIME_OUT2);
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

//------------------------------------------------------------------------------
// Send external data for a scene
// 		
//------------------------------------------------------------------------------
void wckWriteIO(char ServoID, char IO)
{
	wckSendSetCommand((7<<5)|ServoID, 0x64, IO, IO);
	wckGetByte(TIME_OUT2);
	wckGetByte(TIME_OUT2);			
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



/*------------------------------------------------------------------------------
// Fill the motion data structure from flash.  Uses:
//		gpPg_Table: pointer to array of P gain component for each servo in flash
//		gpDp_Table: pointer to array of D gain component for each servo in flash
//		gpIg_Table: pointer to array of I gain component for each servo in flash
//		gpZero_Table: pointer to servo initial positions (ultimately ignored)

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

------------------------------------------------------------------------------*/

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
	
	// 11. E-motion RSHOOT
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
	
	// 12. E-motion RSIDEWALK
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
	
	// 13. E-motion LSIDEWALK
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
	
	// 14. E-motion STANDUPR
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
	
	// 15. E-motion STANDUPF
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
	
	// 16. E-motion HUNODEMO_SITDOWN
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
	
	// 17. E-motion HUNODEMO_HI
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
	
	// 18. E-motion HUNODEMO_KICKLEFTFRONTTURN
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
	
	// 19. E-motion HANDSTANDS1
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


/************************************************************************/


// if flag set read initial positions

BYTE cpos[32];
int offset[16];
BYTE nos=0;

const BYTE basic18[] = { 143, 179, 198, 83, 106, 106, 69, 48, 167, 141, 47, 47, 49, 199, 192, 204, 122, 125};
const BYTE basic16[] = { 125, 179, 199, 88, 108, 126, 72, 49, 163, 141, 51, 47, 49, 199, 205, 205 };
const BYTE basicdh[] = { 143, 179, 198, 83, 105, 106, 68, 46, 167, 140, 77, 70, 152, 165, 181, 98, 120, 124, 99};
 
int getservo(int id)
{
/*
	wckSendOperCommand(0xa0|id, 0);
	int b1 = wckGetByte(TIME_OUT2);
	int b2 = wckGetByte(TIME_OUT2);
	
	if (b1<0 || b2<0)
		return -1;
	else
		return b2;
*/
	return wckPosRead(id&0xFF);
}

BYTE readservos(int n)
{
	BYTE i;
        if (n==0) n=31;
	for (i=0; i<n; i++)
	{
		int p = getservo(i);
		if (p<0 || p>255) break;	
	}	
	nos=i;
	return i;
}

BYTE passiveservos(int n)
{
	BYTE i;
        if (n==0) n=31;
	for (i=0; i<n; i++)
	{
		int p = wckSetPassive(i);
		if (p<0 || p>255) break;
	}	
	return i;
}

// Different type of move interpolation
// from http://robosavvy.com/forum/viewtopic.php?t=5306&start=30
// original by RN1AsOf091407

int PP_mtype=Linear;

double CalculatePos_Accel(int Distance, double FractionOfMove) 
{
    return FractionOfMove * (Distance * FractionOfMove);
}

double CalculatePos_Decel(int Distance, double FractionOfMove)
{
    FractionOfMove = 1 - FractionOfMove;
    return Distance - (FractionOfMove * (Distance * FractionOfMove));
}

double CalculatePos_Linear(int Distance, double FractionOfMove)
{
    return (Distance * FractionOfMove);
}

double CalculatePos_AccelDecel(int Distance, double FractionOfMove)
{
    if ( FractionOfMove < 0.5 )     // Accel:
        return CalculatePos_Accel(Distance /2, FractionOfMove * 2);
    else if (FractionOfMove > 0.5 ) //'Decel:
        return CalculatePos_Decel(Distance/2, (FractionOfMove - 0.5) * 2) + (Distance * 0.5);
    else                            //'= .5! Exact Middle.
        return Distance / 2;
}

double GetMoveValue(int mt, int StartPos, int EndPos, double FractionOfMove)
{
    int Offset,Distance;
    if (StartPos > EndPos)
    {
        Distance = StartPos - EndPos;
        Offset = EndPos;
        switch (mt)
        {
            case Accel:
                return Distance - CalculatePos_Accel(Distance, FractionOfMove) + Offset;
            case AccelDecel:
                return Distance - CalculatePos_AccelDecel(Distance, FractionOfMove) + Offset;
            case Decel:
                return Distance - CalculatePos_Decel(Distance, FractionOfMove) + Offset;
            case Linear:
                return Distance - CalculatePos_Linear(Distance, FractionOfMove) + Offset;
        }
    }
    else
    {
        Distance = EndPos - StartPos;
        Offset = StartPos;
        switch (mt)
        {
            case Accel:
                return CalculatePos_Accel(Distance, FractionOfMove) + Offset;
            case AccelDecel:
                return CalculatePos_AccelDecel(Distance, FractionOfMove) + Offset;
            case Decel:
                return CalculatePos_Decel(Distance, FractionOfMove) + Offset;
            case Linear:
                return CalculatePos_Linear(Distance, FractionOfMove) + Offset;
        }
    }
    return 0.0;
}

// Play d ms per step, f frames, from current -> spod
void PlayPose(int d, int f, int tq, BYTE spod[], int flag)
{
	int i;	

	if (flag!=0) 
	{
		readservos(0);	// set nos and reads cpos
		nos=flag;
	}
	
	int dur=d/f;
	if (dur<25) dur=25; //25ms is quickest
	
	for (i=0; i<f; i++)
	{
		BYTE temp[nos];
		for (int j=0; j<nos; j++)
		{
			//temp[j] = cpos[j] + (float)((i)*intervals[j]+0.5);
			temp[j] = (BYTE)GetMoveValue(PP_mtype, cpos[j], spod[j], (double)i / (double)f);
		}
		wckSyncPosSend(nos-1, tq, temp, 0);
		delay_ms(dur);
	}
	
	for (i=0; i<nos; i++)
	{
		cpos[i]=spod[i];
	}
	
	wckSyncPosSend(nos-1, tq, cpos, 0);
	delay_ms(dur);
}

void PlayMotion(BYTE n)
{
	int i=0;

	int ns = mlist[n].NoS;	
	int nw = mlist[n].Now;
	
	PGM_P p = mlist[n].PoT;
	PGM_P f = mlist[n].FT;
	PGM_P t = mlist[n].RT;

	for (i=1; i<=ns; i++) //for each scene
	{
		BYTE temp[nw];
		for (int j=0; j<nw; j++)
		{
			temp[j] = pgm_read_byte(p+j+i*nw);  
			if (j<16)
			{
				temp[j] += offset[j];
			}
		}

		int dur=pgm_read_word(t+(i-1)*2);
		int frt=pgm_read_word(f+(i-1)*2);

		PlayPose(dur, frt, 4, temp, (i==1)?16:0);
	}
}

int dm=0;
void setdh(int n) {dm=n;}

void standup (int n)
{
	int nw = n%32;
	BYTE temp[nw];
	for (int j=0; j<nw; j++)
	{
		if (nw<=16)
			temp[j]=basic16[j]; //huno with hip
		else
		{
			if (dm)
				temp[j]=basicdh[j]; //huno with hip
			else
				temp[j]=basic18[j]; //huno with hip
		}
		if (j<16 && (n&32==32))
		{
			temp[j] += offset[j];
		}
	}
	PlayPose(1000, 10, 4, temp, nw); //huno basic
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

WORD send_hex_str(char *bus_str, int n)
{
		BYTE b1;
		WORD r=0;
		char *eos = bus_str+n;

		while  ((bus_str<eos) && *bus_str != 0)
		{
			b1=0;
			if ((*bus_str>='0') && (*bus_str<='9')) b1 = *bus_str-'0' ;
			if ((*bus_str>='A') && (*bus_str<='Z')) b1 = *bus_str-'A' + 10 ;
			bus_str++;
			b1 <<=4;
			if ((*bus_str>='0') && (*bus_str<='9')) b1 += *bus_str-'0' ;
			if ((*bus_str>='A') && (*bus_str<='Z')) b1 += *bus_str-'A' + 10 ;
			wckSendByte(b1);
		}

		r = ((0xFF&wckGetByte(25))<<8) | (0xFF&wckGetByte(25));
		return r;
}

WORD send_hex_array(int *p, int n)
{
		int i;
		BYTE b1;
		WORD r=0;

		for (i=0;i<n;i++)
		{
			b1=(BYTE)p[i];
			wckSendByte(b1);
		}

		r = ((0xFF&wckGetByte(25))<<8) | (0xFF&wckGetByte(25));
		return r;
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
