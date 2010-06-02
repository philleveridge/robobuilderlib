/*
64/71
User’s Manual v1.07 wCK series


void main(void)
{
	char i, old_position;
	Initialize(); // Initialize peripheral devices(prepare for serial port)
	id = 0;
	old_position = ActDown(id); // Read the initial position of a wCK with ID 0
	while(1) {
		now_position = ActDown(id); // Read current position
		// If position value decreased, rotate to ccw direction for 1 second and turn to passive mode for 1 second
		if(now_position<old_position) {
			Rotation360(id, 10, ROTATE_CCW);
			delay_ms(1000);
			ActDown(id);
			delay_ms(1000);
		}
		// If position value increased, rotate to cw direction for 1 second and turn to passive mode for 1 second
		else if(now_position>old_position) {
			Rotation360(id, 10, ROTATE_CW);
			delay_ms(1000);
			ActDown(id);
			delay_ms(1000);
		}
		old_position = ActDown(id); // Read current position and save it
		delay_ms(300);
	}
}
*/


//////////////////////////////// Definition of Basic Functions ///////////////////////////


/******************************************************************************/
/* Function that sends Operation Command Packet(4 Byte) to wCK module */
/* Input : Data1, Data2 */
/* Output : None */
/******************************************************************************/
void SendOperCommand(char Data1, char Data2)
{
	char CheckSum;
	CheckSum = (Data1^Data2)&0x7f;
	SendByte(HEADER);
	SendByte(Data1);
	SendByte(Data2);
	SendByte(CheckSum);
}
/******************************************************************************/
/* Function that sends Set Command Packet(6 Byte) to wCK module */
/* Input : Data1, Data2, Data3, Data4 */
/* Output : None */
/******************************************************************************/
void SendSetCommand(char Data1, char Data2, char Data3, char Data4)
{
	char CheckSum;
	CheckSum = (Data1^Data2^Data3^Data4)&0x7f;
	SendByte(HEADER);
	SendByte(Data1);
	SendByte(Data2);
	SendByte(Data3);
	SendByte(Data4);
	SendByte(CheckSum);
}
/*************************************************************************************************/
/* Function that sends Position Move Command to wCK module */
/* Input : ServoID, SpeedLevel, Position */
/* Output : Current */
/*************************************************************************************************/
char PosSend(char ServoID, char SpeedLevel, char Position)
{
	char Current;
	SendOperCommand((SpeedLevel<<5)|ServoID, Position);
	GetByte(TIME_OUT1);
	Current = GetByte(TIME_OUT1);
	return Current;
}
/************************************************************************************************/
/* Function that sends Position Read Command to wCK module */
/* Input : ServoID */
/* Output : Position */
/************************************************************************************************/
char PosRead(char ServoID)
{
	char Position;
	SendOperCommand(0xa0|ServoID, NULL);
	GetByte(TIME_OUT1);
	Position = GetByte(TIME_OUT1);
	return Position;
}
/******************************************************************************/
/* Function that sends Passive wCK Command to wCK module */
/* Input : ServoID */
/* Output : Position */
/******************************************************************************/
char ActDown(char ServoID)
{
	char Position;
	SendOperCommand(0xc0|ServoID, 0x10);
	GetByte(TIME_OUT1);
	Position = GetByte(TIME_OUT1);
	return Position;
}
/*************************************************************************/
/* Function that sends Break wCK Command to wCK module */
/* Input : None */
/* Output : ServoID if succeed, 0xff if fail */
/**************************************************************************/
char PowerDown(void)
{
	char ServoID;
	SendOperCommand(0xdf, 0x20);
	ServoID = GetByte(TIME_OUT1);
	GetByte(TIME_OUT1);
	if(ServoID<31) 
		return ServoID;
	return 0xff; //Receive error
}
/******************************************************************/
/* Function that sends 360 degree Wheel wCK Command */
/* Input : ServoID, SpeedLevel, RotationDir */
/* Return : Rotation Number */
/*****************************************************************/
char Rotation360(char ServoID, char SpeedLevel, char RotationDir)
{
	char ServoPos, RotNum;
	if(RotationDir==ROTATE_CCW) 
	{
		SendOperCommand((6<<5)|ServoID, (ROTATE_CCW<<4)|SpeedLevel);
	}
	else if(RotationDir==ROTATE_CW) 
	{
		SendOperCommand((6<<5)|ServoID, (ROTATE_CW<<4)|SpeedLevel);
	}
	RotNum = GetByte(TIME_OUT1);
	GetByte(TIME_OUT1);
	return RotNum;
}
/*****************************************************************************/
/* Function that sends Synchronized Position Move Command to wCK module */
/* Input : LastID, SpeedLevel, *TargetArray, Index */
/* Return : None */
/****************************************************************************/
void SyncPosSend(char LastID, char SpeedLevel, char *TargetArray, char Index)
{
	int i;
	char CheckSum;
	i = 0;
	CheckSum = 0;
	SendByte(HEADER);
	SendByte((SpeedLevel<<5)|0x1f);
	SendByte(LastID+1);
	while(1) 
	{
		if(i>LastID) 
			break;
		SendByte(TargetArray[Index*(LastID+1)+i]);
		CheckSum = CheckSum ^ TargetArray[Index*(LastID+1)+i];
		i++;
	}
	CheckSum = CheckSum & 0x7f;
	SendByte(CheckSum);
}
/********************************************************************/
/* Function that sends Baud rate Set Command to wCK module */
/* Input : ServoID, NewBaud */
/* Return : New Baudrate if succeed, 0xff if fail */
/********************************************************************/
char BaudrateSet(char ServoID, char NewBaud)
{
	SendSetCommand((7<<5)|ServoID, 0x08, NewBaud, NewBaud);
	GetByte(TIME_OUT2);
	if(GetByte(TIME_OUT2)==NewBaud) 
		return NewBaud;
	return 0xff;
}
/*********************************************************************/
/* Function that sends Gain Set Command to wCK module */
/* Input : ServoID, *NewPgain, *NewDgain */
/* Return : 1 if succeed, 0 if fail */
/********************************************************************/
char GainSet(char ServoID, char *NewPgain, char *NewDgain)
{
	char Data1,Data2;
	SendSetCommand((7<<5)|ServoID, 0x09, *NewPgain, *NewDgain);
	Data1 = GetByte(TIME_OUT2);
	Data2 = GetByte(TIME_OUT2);
	if((Data1==*NewPgain) && (Data2==*NewDgain)) 
		return 1;
	return 0;
}
/************************************************************/
/* Function that sends ID Set Command to wCK module */
/* Input : ServoID, NewId */
/* Return : New ID if succeed, 0xff if fail */
/***********************************************************/
char IdSet(char ServoID, char NewId)
{
	SendSetCommand((7<<5)|ServoID, 0x0a, NewId, NewId);
	GetByte(TIME_OUT2);
	if(GetByte(TIME_OUT2)==NewId) 
		return NewId;
	return 0xff;
}
/******************************************************************/
/* Function that sends Gain Read Command to wCK module */
/* Input : ServoID, *NewPgain, *NewDgain */
/* Return : 1 if succeed, 0 if fail */
/*****************************************************************/
char GainRead(char ServoID, char *Pgain, char *Dgain)
{
	SendSetCommand((7<<5)|ServoID, 0x0c, 0, 0);
	*Pgain = GetByte(TIME_OUT1);
	*Dgain = GetByte(TIME_OUT1);
	if((*Pgain>0) && (*Pgain<51) && (*Dgain<101)) 
		return 1;
	return 0;
}
/**********************************************************************************/
/* Function that sends Over Load Set Command to wCK module */
/* Input : ServoID, NewOverCT */
/* Return : New Overcurrent Threshold if succeed, 0xff if fail */
/**********************************************************************************/
char OverCTSet(char ServoID, char NewOverCT)
{
	char Data1;
	SendSetCommand((7<<5)|ServoID, 0x0f, NewOverCT, NewOverCT);
	sciRxReady(TIME_OUT2);
	Data1=sciRxReady(TIME_OUT2);
	if(Data1!=0xff) 
		return Data1;
	return 0xff;
}
/******************************************************************************/
/* Function that sends Over Load Read Command to wCK module */
/* Input : ServoID */
/* Return : Overcurrent Threshold if succeed, 0xff if fail */
/******************************************************************************/
char OverCTRead(char ServoID)
{
	char Data1;
	SendSetCommand((7<<5)|ServoID, 0x10, 0, 0);
	sciRxReady(TIME_OUT1);
	Data1=sciRxReady(TIME_OUT1);
	if(Data1!=0xff) 
		return Data1;
	return 0xff;
}
/***********************************************************************/
/* Function that sends Boundary Set Command to wCK module */
/* Input : ServoID, *NewLBound, *NewUBound */
/* Return : 1 if succeed, 0 if fail */
/**********************************************************************/
char BoundSet(char ServoID, char *NewLBound, char *NewUBound)
{
	char Data1,Data2;
	SendSetCommand((7<<5)|ServoID, 0x11, *NewLBound, *NewUBound);
	Data1 = GetByte(TIME_OUT2);
	Data2 = GetByte(TIME_OUT2);
	if((Data1==*NewLBound) && (Data2==*NewUBound)) 
		return 1;
	return 0;
}
/**************************************************************************/
/* Function that sends Boundary Read Command to wCK module */
/* Input : ServoID, *NewLBound, *NewUBound */
/* Return : 1 if succeed, 0 if fail */
/*************************************************************************/
char BoundRead(char ServoID, char *LBound, char *UBound)
{
	SendSetCommand((7<<5)|ServoID, 0x12, 0, 0);
	*LBound = GetByte(TIME_OUT1);
	*UBound = GetByte(TIME_OUT1);
	if(*LBound<*UBound) 
		return 1;
	return 0;
}
//////////////////////////////// End of Basic Functions Definition /////////////////////////////