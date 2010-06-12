// experimental_bin_mode.c
//

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h> 

#include "global.h"
#include "Main.h"
#include "Macro.h"
#include "motion.h"
#include "rprintf.h"
#include "adc.h"
#include "ir.h"
#include "uart.h"
#include "accelerometer.h"
#include "majormodes.h"
#include "wck.h"
#include <util/delay.h>

static unsigned char *motionBuf;
extern void print_motionBuf(int bytes);

extern void set_type(uint8_t c);
extern uint8_t get_type();
extern uint8_t get_noservos();

extern void send_bus_str(char *bus_str, int n);

/***************************************

binary mode

q  query - query everything
m  move
x  raw wCK - no wait
X  raw wck - wait for response
v  version
P/y/Y  PSD read/on/off
I  Infrared and button state
D  Just PSD
Q  Quick query (just PSD and Acc)
l  basic download
H  low speed wck bus comms 
p  exit

****************************************/


void experimental_binloop();

#define MAGIC_RESPONSE	0xEA
#define MAGIC_REQUEST	0xCD
#define VERSION			0x12     /* BIN API VERSION */
#define MAX_INP_BUF 	40

#define PROTOCOL_ERROR	01


void SendWord(WORD x) 
{
	uartSendByte(x&0xff);
	uartSendByte(x>>8);
}

void SendResponse(char mt, uint8_t d)
{
	uartSendByte(MAGIC_RESPONSE);
	uartSendByte(mt);
	uartSendByte(d);
	uartSendByte( (mt ^ d) & 0x7F);
}

/***************************

Error response
do we need this or should it ignore

***************************/

int bin_respond_error(int errno)
{
	SendResponse('z', errno);
	return 0;
}

/***************************

ver command response

***************************/

int bin_respond_ver()
{
	SendResponse('v', VERSION);
	return 0;
}

/***************************

l basic download read
read comiled binary from serial port

***************************/

extern uint8_t EEMEM BASIC_PROG_SPACE[];  // this is where the tokenised code will be stored

/*
int bin_downloadbasic()
{
	int i;
	uint8_t b0=0, b1=0;
	int cs = 0;

	while ((b0=uartGetByte())<0);      
	while ((b1=uartGetByte())<0);
	int bytes = ((int)b1 << 8) | b0;
	cs ^= b0;
	cs ^= b1;
	
	for (i=0; i<bytes; i++)
	{
		while ((b0=uartGetByte())<0); 	
		cs ^= b0;
		eeprom_write_byte(BASIC_PROG_SPACE+i, b0);	
	}
	
	while ((b0=uartGetByte())<0);
	return (b0 != (cs&0x7f));
}
*/

/***************************

H command response

***************************/

int bin_read_H()
{
    char sbuf[10];
	uint8_t b0=0, b1=0;
	int cs = 0;
	int i;

	while ((b0=uartGetByte())<0);   
	cs ^= b0;
	
	if (b0>=10) b0=0;
	
	for (i=0; i<b0; i++)
	{
		while ((b1=uartGetByte())<0); 	
		cs ^= b1;
		sbuf[i] = b1;
	}
	
	if (b1 == (cs&0x7f)) 
		send_bus_str(sbuf, b0);
	
	return (b1 != (cs&0x7f));
}


int bin_respond_H(int mt)
{
	SendResponse('H', VERSION);
	return 0;
}


/***************************

l basic command response



int bin_respond_basicdownload(int mt)
{

	SendResponse('l', VERSION);
	return 0;
}
***************************/


/***************************

Q command response (PSD and XYZ values)

***************************/


int bin_respond_Quickquery(int mt)
{
	BYTE	tmpB;
		
	uartSendByte(MAGIC_RESPONSE);
	uartSendByte(mt);	
	int cs=mt;
	
	tmpB=adc_psd();
	uartSendByte(tmpB); 			//0

	cs ^= tmpB;	

	tilt_read(0);
	
	SendWord((WORD)x_value);  		// 1
	cs = cs ^ (x_value & 0xff) ^ (x_value >> 8) ;
	SendWord((WORD)y_value);  		// 3
	cs = cs ^ (y_value & 0xff) ^ (y_value >> 8) ;
	SendWord((WORD)z_value);  		//5
	cs = cs ^ (z_value & 0xff) ^ (z_value >> 8) ;
	
	uartSendByte( (cs) & 0x7F); 	//7

	return 0;
}

/***************************

A command response ( XYZ values)

***************************/

int bin_respond_Aquery(int mt)
{
	uartSendByte(MAGIC_RESPONSE);
	uartSendByte(mt);	
	int cs=mt;

	tilt_read(0);
	
	SendWord((WORD)x_value);  		// 0 & 1
	cs = cs ^ (x_value & 0xff) ^ (x_value >> 8) ;
	SendWord((WORD)y_value);  		// 2 & 3
	cs = cs ^ (y_value & 0xff) ^ (y_value >> 8) ;
	SendWord((WORD)z_value);  		// 4 & 5
	cs = cs ^ (z_value & 0xff) ^ (z_value >> 8) ;
	
	uartSendByte( (cs) & 0x7F); 	//6

	return 0;
}


/***************************

D command response (PSD only)

***************************/
int bin_respond_Dquery(int mt)
{
	BYTE	tmpB;
		
	uartSendByte(MAGIC_RESPONSE);
	uartSendByte(mt);	
	
	tmpB=adc_psd();
	uartSendByte(tmpB); 			//0

	uartSendByte( (mt ^ tmpB) & 0x7F); 	//7

	return 0;
}

/***************************

I command response (IR and PF1/PF2 values)

***************************/
int bin_respond_Iquery(int mt)
{
	BYTE	tmpB;
	int cs = mt;
		
	uartSendByte(MAGIC_RESPONSE);
	uartSendByte(mt);	
	
	if ( gIRReady) 
	{
		gIRReady = FALSE;
		tmpB = gIRData;
	}
	else
	{
		tmpB=0;
	}
	uartSendByte(tmpB); 	
    cs ^= tmpB;
	
	tmpB=PINA & 3;
    cs ^= tmpB;	
	uartSendByte(tmpB); 	
	
	uartSendByte( (cs) & 0x7F); 	

	return 0;
}


/***************************

q command response

***************************/

int bin_respond_query(int mt)
{
	BYTE	tmpB;
	WORD	tmpW;
	
	int noservos =  get_noservos();
	
	uartSendByte(MAGIC_RESPONSE);
	uartSendByte(mt);	
	uartSendByte(noservos); 		//0
	
	//query mode
	int cs=mt;
	for (BYTE id=0; id<noservos; id++)
	{
			tmpW = wckPosAndLoadRead(id);
			SendWord(tmpW);	
			cs = cs ^ (tmpW & 0xff) ^ (tmpW >> 8) ;
	} 								//0 + nos*2
	
	tmpB=adc_psd();
	uartSendByte(tmpB); 			//1 + nos*2
	cs ^= tmpB;	

	tmpB=adc_mic();
	uartSendByte(tmpB); 			//2  + nos*2
	cs ^= tmpB;	
	
	tmpW=adc_volt();
	SendWord(tmpW);	    			//4  + nos*2
	cs = cs ^ (tmpW & 0xff) ^ (tmpW >> 8) ;

	tilt_read(0);
	
	SendWord((WORD)x_value);  		// 6  + nos*2
	cs = cs ^ (x_value & 0xff) ^ (x_value >> 8) ;
	SendWord((WORD)y_value);  		// 8  + nos*2
	cs = cs ^ (y_value & 0xff) ^ (y_value >> 8) ;
	SendWord((WORD)z_value);  		//10  + nos*2
	cs = cs ^ (z_value & 0xff) ^ (z_value >> 8) ;
	
	uartSendByte( (cs) & 0x7F); 	//11  + nos*2

	return 0;
}

/***************************

x command read / response

***************************/

int buff[MAX_INP_BUF];
int bsize;


int bin_read_x()
{
	int c;
	int b0;
	int cs;
	
	while ((bsize=uartGetByte())<0); // 1 byte length
	
	cs=bsize;
	
	for (c=0; (c<bsize && c<MAX_INP_BUF); c++)
	{			
		while ((b0=uartGetByte())<0);
		//buff[c]=b0; 	// load each byte	
		sciTx0Data(b0&0xFF);		
		cs ^= b0;		// calculate checksum
	}
	while ((b0=uartGetByte())<0);

	return (b0 != (cs&0x7f));
}

int bin_respond_x(int mt)
{
	int c;
	BYTE b1;
	BYTE b2;
	
	uartSendByte(MAGIC_RESPONSE);
	uartSendByte(mt);

	if (mt=='X')
	{
		// transmit  buffer
		//for (c=0; (c<bsize && c<MAX_INP_BUF); c++)
		//{	
			//sciTx0Data(buff[c]&0xFF);
		//}	
		// get response (or timeout)				
		b1 = sciRx0Ready();
		b2 = sciRx0Ready();
		
		uartSendByte(b1);
		uartSendByte(b2);
		uartSendByte( (mt ^ b1 ^ b2 ) & 0x7F);
	}
	else
	{
		b1 = 0; 			// could use to send status	
		uartSendByte(b1);  	// junk
		uartSendByte(b1);  	// junk
		uartSendByte( (mt ^ b1 ^ b1) & 0x7F); //checksum
		
		// transmit  buffer (after sending respose - should be quicker - 
		// not waiting for reply
		//for (c=0; (c<bsize && c<MAX_INP_BUF); c++)
		//{	
		//	sciTx0Data(buff[c]&0xFF);
		//}
	}
	return 0;
}

/***************************

p command read / response
p0 = PSD on
p1 = PSD off
p2 = readPSD

***************************/

int bin_read_p(int mt)
{
	int cs;

	while ((cs=uartGetByte())<0); 	// read cs sent

	if  (((mt ^ MAGIC_REQUEST) & 0x7f) == cs)		// Do they match
	{
		switch(mt)
		{
		case 'Y':
			PSD_on();
			break;
		case 'y':
			PSD_off();
			break;
		case 'P':
			Get_AD_PSD();
			break;
		default:
			return -1;
		}
		return 'P';
	}
	return -1;
}

int bin_respond_p(int mt)
{
	SendResponse(mt, gDistance);
	return 0;
}

/***************************

m command read / response

***************************/

int bin_read_m()
{
	int nb=0;
	int b0;
	int b1;
	int cs;
	
	motionBuf = GetNextMotionBuffer();
	
	while ((b0=uartGetByte())<0);
	while ((b1=uartGetByte())<0);
	int bytes = ((int)b1 << 8) | b0;

	cs = bytes;

	while (nb<bytes)
	{				
		while ((b0=uartGetByte())<0);
		motionBuf[nb++] = b0;
		cs ^= b0;					// calculate checksum
	}

	while ((b0=uartGetByte())<0); 	// read cs sent

	return (b0 != (cs&0x7f));  		// Do they match
}


int bin_respond_m(int mt)
{
	int b1 =0 ;
	
	LoadMotionFromBuffer(motionBuf);			
	
	//left in for now
	//till happy they can't clash
	//could be moded to main loop after that
	
	PlaySceneFromBuffer(motionBuf, 0);
	complete_motion(motionBuf);
	
	// tells client it can send again
	
	SendResponse(mt, b1);
	
	return 0;
}

/***************************

Core packet recieve request code

***************************/

int bin_read_request()
{
	int b0;
	int mt;
	int cs;
	while ((b0=uartGetByte())<0);
	if (b0==MAGIC_REQUEST)
	{
		while ((mt=uartGetByte())<0);
		
		if (mt=='q' || mt=='p' || mt=='v' || mt=='C' || mt=='S' || mt=='Q' || mt=='D'  || mt=='I'  || mt=='A') 
		{			
			while ((cs=uartGetByte())<0);

			if (cs != ((b0 ^ mt) &0x7f))
				return -1;
			else
				return mt;
		}
		
		if (mt=='x' || mt == 'X')
		{	
			if (bin_read_x())
				return -1;
			else
				return mt;
		}		
		
		if (mt=='m')
		{
			if (bin_read_m())
				return -1;
			else
				return mt;
		}	
				
		if (mt=='H')
		{
			if (bin_read_H())
				return -1;
			else
				return mt;
		}	
		
		
		if (mt=='P' || mt=='y' || mt=='Y')
		{
			return  bin_read_p(mt);
		}	

		/*if (mt=='l')
		{
			if (bin_downloadbasic())
				return -1;
			else
				return mt;
		}*/	
	}
	//
	return -1;
}

/***************************

Core loop of binary_mode
You must send a 'p' packet to exit

***************************/

void experimental_binloop()
{
	int r;

	while (1) {
		
		while ((r=bin_read_request())<0) ;
		
		switch (r)
		{
		//case 'l':
		//	bin_respond_basicdownload(r); break;
		case 'v':
			bin_respond_ver(r); break;
		case 'q':
			bin_respond_query(r); break;
		case 'Q':
			bin_respond_Quickquery(r); break;
		case 'D':
			bin_respond_Dquery(r); break;
		case 'A':
			bin_respond_Aquery(r); break;
		case 'I':
			bin_respond_Iquery(r); break;
		case 'm':
			bin_respond_m(r); break;
		case 'x': case 'X' :
			bin_respond_x(r); break;		
		case 'P':
			bin_respond_p(r); break;		
		case 'H':
			bin_respond_H(r); break;	
		case 'p':
			// exit back to idle mode
			return;
		default:
			bin_respond_error(PROTOCOL_ERROR);
		}
			
		_delay_ms(1);
	}
}

