
#include <avr/io.h>
#include "wck.h"

#define HEADER     0xff  
#define NULL       0  
#define ROTATE_CCW 3  
#define ROTATE_CW  4  
#define TIME_OUT1  100  // receive character timeout [msec]
#define TIME_OUT2  250	// timeout for Set routines that change baudrate, etc.

//////////////////////////////// Serial Interface Functions ///////////////////////////

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
char wckGetByte(WORD timeout)
{
	WORD	startT;
	startT = gMSEC;
	while(!(UCSR0A&(1<<RXC)) ){ 	// test for received character
        if(gMSEC<startT) {
			// wait RX_T_OUT for a character
            if((1000 - startT + gMSEC) > timeout) break;
        }
		else if((gMSEC - startT) > timeout) break;
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
char wckPosRead(char ServoID)
{
	char Position;
	wckSendOperCommand(0xa0|ServoID, NULL);
	wckGetByte(TIME_OUT1);
	Position = wckGetByte(TIME_OUT1);
	return Position;
}

/************************************************************************************************/
/* Function that sends Position Read Command to wCK module, and returns the Load and Position. */
/* Input : ServoID */
/* Output : Load * 256 + Position */
/************************************************************************************************/
WORD wckPosAndLoadRead(char ServoID)
{
	WORD Load, Position;
	wckSendOperCommand(0xa0|ServoID, NULL);
	Load = wckGetByte(TIME_OUT1);
	Position = wckGetByte(TIME_OUT1);
	return (Load << 8) | Position;
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
