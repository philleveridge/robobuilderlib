#include "macro.h"
#include <avr/io.h>

#include "rprintf.h"
#include "femto.h"


#define	RXC			7
#define RX_COMPLETE		(1<<RXC)
#define HEADER			0xff
#define TIME_OUT		250

#define DATA_REGISTER_EMPTY (1<<UDRE)

#define	PWR_LED2_ON		CLR_BIT7(PORTC)   
#define	PWR_LED2_OFF		SET_BIT7(PORTC)

extern volatile WORD   g10Mtimer;
extern void     delay_ms(int ms);

//------------------------------------------------------------------------------
// UART0 Receive  Routine
//------------------------------------------------------------------------------

int getWck(WORD timeout)
{
	g10Mtimer = timeout;

rprintf ("t=%d\n", g10Mtimer);
	
	while(!(UCSR0A&(1<<RXC)) )
	{ 	// test for received character
		if (g10Mtimer==0) return -1;
	}
	return UDR0;
}

//------------------------------------------------------------------------------
// UART0 Transmit  Routine
//------------------------------------------------------------------------------

void putWck (BYTE b)
{
//	while ( (UCSR0A & DATA_REGISTER_EMPTY) == 0 );
//	UDR0 = b;
rprintf ("ts=%x\n", b);	
	while(!(UCSR0A&(1<<UDRE))) {}	// wait until data register is empty
	UDR0=b;
}

//////////////////////////////// Serial Interface Functions ///////////////////////////

void wckInit(unsigned int ubrr)
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

/******************************************************************************/
/* Function that sends Operation Command Packet(4 Byte) to wCK module */
/* Input : Data1, Data2 */
/* Output : None */
/******************************************************************************/
void wckSendOperCommand(BYTE Data1, BYTE Data2)
{
	char CheckSum;
	CheckSum = (Data1^Data2)&0x7f;
	putWck(HEADER);
	putWck(Data1);
	putWck(Data2);
	putWck(CheckSum);
}

void wckSendSetCommand(BYTE Data1, BYTE Data2, BYTE Data3, BYTE Data4)
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

BYTE cpos[32];
BYTE nos=0;

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

int response[32];

int wckReadPos(BYTE id, BYTE d1)
{
	UCSR0B = (1<<RXEN)|(1<<TXEN);

	if (id>=0 && id<=31)
	{
		wckSendOperCommand(0xa0|id, NULL);
		response[0] = getWck(TIME_OUT);
		response[1] = getWck(TIME_OUT);
		if (response[0]<0 || response[1]<0) return 0;
		return 1;
	}
	return 0;
}

int wckPassive(int id)
{
	if (id>=0 && id<=31)
	{
		wckSendOperCommand(0xc0|(id), 0x10);
		response[0] = getWck(TIME_OUT);
		response[1] = getWck(TIME_OUT);
		if (response[0]<0 || response[1]<0) return 0;
		return 1;
	}
	return 0;
}

int wckMovePos(int sid, int pos, int tor)
{
	if (sid<0 || sid>30 || pos<0 || pos>254 ||tor<0 || tor>4) 	
	{
		return 0;
	}
	else	
	{
		wckSendOperCommand((tor<<5)|sid, pos);
		response[0] = getWck(TIME_OUT);
		response[1] = getWck(TIME_OUT);	
		if (response[0]<0 || response[1]<0) return 0;
		return 1;
	}
}

int wckwriteIO(int id, int b1, int b2)   // i.e. (wckwriteIO 15 true true)
{
	if (id>=0 && id<=31)
	{	
        	wckSendSetCommand((7 << 5) | (id % 31), 0x64, ((b1) ? 1 : 0) | ((b2) ? 2 : 0), 0);		
		response[0] = getWck(TIME_OUT);
		response[1] = getWck(TIME_OUT);
		if (response[0]<0 || response[1]<0) return 0;
		return 1;	
	}
	return 0;
}

tOBJ wckstandup(tOBJ r)   // i.e. (standup)
{
	standup(18);
	return r;
}

tOBJ synchServo(tOBJ a)   // i.e. (sycnServo lastid torq '(positions))
{
	tOBJ r=emptyObj();
/*	BYTE lastid = (p->head.number)&0x1F;
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
			if (p == NULL)
			{
				return throw(0);
			}
			if (p->head.type==INTGR)
			{
				n=p->head.number;							
				DEBUG(rprintf("- %d",n);)				
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
*/
	return r;
}

tOBJ moveServo(tOBJ a)   // i.e. (moveServo time frames '(positions))
{
	tOBJ r=emptyObj();
	int n=0;
/*	
	BYTE msec  = (p->head.number)&0x1F;
	p=p->tail;
	BYTE frame = (p->head.number)&0x03;
	p=p->tail;
	
	if (p->head.type == CELL)
	{
		p=(tCELLp)(p->head.cell);
		tOBJ l = olen(p->head);
		if (l.type==INTGR)
		{
			BYTE bytearray[l.number];
			for (n=0; n<l.number; n++)
			{
				if (p->head.type != INTGR) return throw(1);
				bytearray[n] = p->head.number;
				p=p->tail;
			}
			// PlayPose(msec, frame, bytearray, n);   // uncomment when checked!!
		}	
	};
*/	
	return r;
}



