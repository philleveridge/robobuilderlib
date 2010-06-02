#include "Macro.h"
#include <avr/io.h>

#define	RXC				7
#define RX_COMPLETE		(1<<RXC)
#define HEADER			0xFF 
#define TIME_OUT		100
#define NULL			'\0'
#define DATA_REGISTER_EMPTY (1<<UDRE)

extern volatile WORD   g10Mtimer;
extern void     delay_ms(int ms);

//femto.c
extern void printline(char *c);  
extern void printint(int);  	
extern void printstr(char*);	
extern void printnumber(int n, int w, char pad)  ;

//------------------------------------------------------------------------------
// UART1 Receive  Routine
//------------------------------------------------------------------------------

int getWck(WORD timeout)
{
	g10Mtimer = timeout;
	
	while(!(UCSR0A&(1<<RXC)) ){ 	// test for received character
		if (g10Mtimer==0) return -1;
	}
	return UDR0;
}

//------------------------------------------------------------------------------
// UART1 Transmit  Routine
//------------------------------------------------------------------------------

void putWck (BYTE b)
{
	while ( (UCSR0A & DATA_REGISTER_EMPTY) == 0 );
	UDR0 = b;
}


/******************************************************************************/
/* Function that sends Operation Command Packet(4 Byte) to wCK module */
/* Input : Data1, Data2 */
/* Output : None */
/******************************************************************************/
void wckSendOperCommand(char Data1, char Data2)
{
	char CheckSum;
	CheckSum = (Data1^Data2)&0x7f;
	putWck(HEADER);
	putWck(Data1);
	putWck(Data2);
	putWck(CheckSum);
}

void wckSendSetCommand(char Data1, char Data2, char Data3, char Data4)
{
	char CheckSum;
	CheckSum = (Data1^Data2^Data3^Data4)&0x7f;
	putWck(HEADER);
	putWck(Data1);
	putWck(Data2);
	putWck(Data3);
	putWck(Data4);
	putWck(CheckSum);
}

void wckSyncPosSend(BYTE LastID, BYTE SpeedLevel, BYTE *TargetArray, BYTE Index)
{
	int i;
	char CheckSum;
	i = 0;
	CheckSum = 0;
	putWck(HEADER);
	putWck((SpeedLevel<<5)|0x1f);
	putWck(LastID+1);
	while(1) 
	{
		if(i>LastID) 
			break;
		putWck(TargetArray[Index*(LastID+1)+i]);
		CheckSum = CheckSum ^ TargetArray[Index*(LastID+1)+i];
		i++;
	}
	CheckSum = CheckSum & 0x7f;
	putWck(CheckSum);
}

// if flag set read initial positions

static BYTE cpos[32];
static BYTE nos=0;

int getservo(int id)
{
	wckSendOperCommand(0xa0|id, NULL);
	int b1 = getWck(TIME_OUT);
	int b2 = getWck(TIME_OUT);
	
	if (b1<0 || b2<0)
		return -1;
	else
		return b2;
}

int readservos()
{
	int i;
	for (i=0; i<31; i++)
	{
		int p = getservo(i);
		//printint(i); printstr("-"); printint(p); printline("C");
		
		if (p<0) break;
		cpos[i]=p;	
	}
	nos=i;
	//printint(nos); printline(" nos");
	return i;
}

// Play d ms per step, f frames, from current -> pos
void PlayPose(int d, int f, BYTE *pos, int flag)
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

BYTE basic18[] = { 143, 179, 198,  83, 106, 106,  69,  48, 167, 141,  47,  47,  49, 199, 192, 204, 122, 125};

void standup () 
{
	PlayPose(1000, 10, basic18, 18); 
}

