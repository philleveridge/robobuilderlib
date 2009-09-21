// experimental_bin_mode.c
//

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

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

/***************************************

binary mode

q  query
m  move
x  raw wCK
v  version
p  exit

****************************************/


void experimental_binloop();

#define MAGIC_RESPONSE	0xEA
#define MAGIC_REQUEST	0xCD
#define VERSION			0x11 /* BIN API VERSION */
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
	uartSendByte( (mt | d) & 0x7F);
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
		buff[c]=b0; 	// load each byte
		cs |= b0;		// calculate checksum
	}
	while ((b0=uartGetByte())<0);

	return (b0 != (cs&0x7f));
}

int bin_respond_x(int mt)
{
	int c;
	BYTE b1;
	BYTE b2;

	// transmit  buffer
	for (c=0; (c<bsize && c<MAX_INP_BUF); c++)
	{	
		sciTx0Data(buff[c]&0xFF);
	}
	
	uartSendByte(MAGIC_RESPONSE);
	uartSendByte(mt);

	if (mt=='X')
	{
		// get response (or timeout)					
		b1 = sciRx0Ready();
		b2 = sciRx0Ready();
		
		uartSendByte(b1);
		uartSendByte(b2);
		uartSendByte( (mt | b1 | b2 ) & 0x7F);
	}
	else
	{
		b1 = 0; 			// could use to send status	
		uartSendByte(b1);  	// junk
		uartSendByte(b1);  	// junk
		uartSendByte( (mt | b1 ) & 0x7F); //checksum
	}
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
		cs |= b0;					// calculate checksum
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
		
		if (mt=='q' || mt=='v' || mt=='p' || mt=='C' || mt=='S') 
		{			
			while ((cs=uartGetByte())<0);

			if (cs != ((b0 | mt) &0x7f))
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
		case 'v':
			bin_respond_ver(r); break;
		case 'q':
			bin_respond_query(r); break;
		case 'm':
			bin_respond_m(r); break;
		case 'x': case 'X' :
			bin_respond_x(r); break;		
		case 'p':
			return;
		default:
			bin_respond_error(PROTOCOL_ERROR);
		}
			
		_delay_ms(1);
	}
}

