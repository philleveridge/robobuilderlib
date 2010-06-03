#include "Macro.h"
#include <avr/io.h>
#include "femto.h"

#define	RXC				7
#define RX_COMPLETE		(1<<RXC)
#define HEADER			0xFF 
#define TIME_OUT		100
#define NULL			'\0'
#define DATA_REGISTER_EMPTY (1<<UDRE)

extern volatile WORD   g10Mtimer;
extern void     delay_ms(int ms);

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


/**************************************************************************************************/
//
//    femto functions
//
/**************************************************************************************************/

tOBJ getServo(tCELLp p)   // i.e. (getServo 15)
{
	tOBJ r; r.type=INT;
	if (p->head.type==INT && p->head.number>=0 && p->head.number<=31)
	{
		wckSendOperCommand(0xa0|(p->head.number), NULL);
		int b1 = getWck(TIME_OUT);
		int b2 = getWck(TIME_OUT);
		
		if (b1<0 || b2<0)
			r.type = EMPTY; //timeout
		else
			r.number = b2;
	}
	else
		r.type=EMPTY;
	return r;
}

tOBJ wckwriteIO(tCELLp p)   // i.e. (wckwriteIO 15 true true)
{
	tOBJ r; r.type=INT;
	if (p->head.type==INT && p->head.number>=0 && p->head.number<=31)
	{
		int id = p->head.number;
		p=p->tail;
		if (p->head.type != BOOL) return throw(3);
		int b1 = p->head.number;
		p=p->tail;
		if (p->head.type != BOOL) return throw(3);
		int b2 = p->head.number;
		
        wckSendSetCommand((7 << 5) | (id % 31), 0x64, ((b1) ? 1 : 0) | ((b2) ? 2 : 0), 0);
		
		b1 = getWck(TIME_OUT);
		b2 = getWck(TIME_OUT);
		
		if (b1<0 || b2<0)
			r.type = EMPTY; //timeout
		else
			r.number = b2;		
	}
	else
		r=throw(3);
	return r;
}

tOBJ wckstandup(tCELLp p)   // i.e. (standup)
{
	tOBJ r; r.type=EMPTY;
	standup(18);
	return r;
}

tOBJ passiveServo(tCELLp p)   // i.e. (passiveServo 15)
{
	tOBJ r; r.type=INT;
	if (p->head.type==INT && p->head.number>=0 && p->head.number<=31)
	{
		wckSendOperCommand(0xc0|(p->head.number), 0x10);
		int b1 = getWck(TIME_OUT);
		int b2 = getWck(TIME_OUT);
		
		if (b1<0 || b2<0)
			r.type = EMPTY; //timeout
		else
			r.number = b2;
	}
	else
		r.type=EMPTY;
	return r;
}
tOBJ sendServo(tCELLp p)   // i.e. (sendServo 12 50 4)
{
	tOBJ r; r.type=INT;
	int sid = p->head.number;
	p=p->tail;
	int pos = p->head.number;
	p=p->tail;
	int tor = p->head.number;
	if (sid<0 || sid>30 || pos<0 || pos>254 ||tor<0 || tor>4) 	
	{
		r=throw(3); // incorrect aparams
	}
	else	
	{
		wckSendOperCommand((tor<<5)|sid, pos);

		int b1 = getWck(TIME_OUT);
		int b2 = getWck(TIME_OUT);	

		if (b1<0 || b2<0)
			r.type = EMPTY; //timeout
		else
			r.number = (b1<<8|b2);
	}
	return r;
}

tOBJ synchServo(tCELLp p)   // i.e. (sycnServo lastid torq '(positions))
{
	tOBJ r;	r.type=EMPTY;
	BYTE lastid = (p->head.number)&0x1F;
	p=p->tail;
	BYTE tor    = (p->head.number)&0x03;
	p=p->tail;
	if (p->head.type == CELL)
	{
		BYTE pos[lastid+2];
		p=(tCELLp)(p->head.cell);
		
		for (int i=0; i<lastid+1; i++)
		{
			int n;
			if (p == null)
			{
				return throw(0);
			}
			if (p->head.type==INT)
			{
				n=p->head.number;							
				DEBUG(printint(n); printstr("-");)				
				pos[i] = n;
			}
			else
			{
				return throw(1);
			}

			p=p->tail;
		}
		wckSyncPosSend(lastid, tor, pos, 0); // uncomment if works!
	}
	else
	{
		return throw(2);
	}
	return r;
}

tOBJ len(tCELLp);

tOBJ moveServo(tCELLp p)   // i.e. (moveServo time frames '(positions))
{
	tOBJ r;	r.type=EMPTY;
	int n=0;
	
	BYTE msec  = (p->head.number)&0x1F;
	p=p->tail;
	BYTE frame = (p->head.number)&0x03;
	p=p->tail;
	
	if (p->head.type == CELL)
	{
		p=(tCELLp)(p->head.cell);
		tOBJ l = len(p);
		if (l.type==INT)
		{
			BYTE bytearray[l.number];
			for (n=0; n<l.number; n++)
			{
				if (p->head.type != INT) return throw(1);
				bytearray[n] = p->head.number;
				p=p->tail;
			}
			// PlayPose(msec, frame, bytearray, n);   // uncomment when checked!!
		}	
	}
	else
		return throw(2);
	
	return r;
}
