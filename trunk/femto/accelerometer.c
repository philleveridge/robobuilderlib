// Standard Input/Output functions
#include <stdio.h>

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "global.h"
#include "macro.h"
#include "math.h"
#include "accelerometer.h"


//
//  PORTE PE5
//  PORTE PE4
//  DDRE  DDE4, DDE5
//


#define CLOCK_LOW      CLR_BIT4(PORTE)
#define CLOCK_HIGH     SET_BIT4(PORTE)
#define DATA_LOW       CLR_BIT5(PORTE)
#define DATA_HIGH      SET_BIT5(PORTE)
#define NOP			   asm("nop")

#define Slave_Addr		0x70


//current channel
volatile int x_value;
volatile int y_value;
volatile int z_value;



void start_accel()
{
    SET_BIT4(DDRE);
	SET_BIT5(DDRE);	
	DATA_HIGH;
	CLOCK_HIGH;	
	NOP;
	NOP;
	DATA_LOW;
	NOP;
	NOP;
	CLOCK_LOW;
	NOP;
	NOP;
}

void stop_accel()
{
    SET_BIT4(DDRE);
	SET_BIT5(DDRE);	
	DATA_LOW;
	CLOCK_HIGH;	
	NOP;
	NOP;
	DATA_HIGH;
	NOP;
	NOP;
    CLR_BIT4(DDRE);
	CLR_BIT5(DDRE);	
}

// reads from tilt
BYTE read_value()
{
	BYTE b=0;
	CLR_BIT5(DDRE);	  		// I2C - input
  	BYTE i;
	for (i=0; i<8; i++)
	{
		b <<=(BYTE)1;
		NOP;
		NOP;
		NOP;
		NOP;
		CLOCK_HIGH; 			
		NOP;
		NOP;
		
		if (PINE & BV(PINE5)) 	
		{
			b |= (BYTE)1;  
		}

		NOP;
		NOP;
		CLOCK_LOW; 	  
	}  
	SET_BIT5(DDRE);		  // I2C - output
	return b; 
}

// write to tilt
void write_value(BYTE data)
{
	BYTE i;
	SET_BIT5(DDRE);	
	for (i=0; i<8; i++)
	{
		if (data & (BYTE)0x80)
		{
			DATA_HIGH;
		}
		else
		{
			DATA_LOW;
		}
		NOP;
		NOP;
		CLOCK_HIGH;
		NOP;
		NOP;
		NOP;
		NOP;
		CLOCK_LOW;
		NOP;
		NOP;
		data <<= (BYTE)1;
	}  
}

void ack()
{
	CLR_BIT5(DDRE);	
	NOP;
	NOP;
	DATA_HIGH;
	NOP;
	NOP;
	CLOCK_HIGH;
	NOP;
	NOP;
	CLOCK_LOW;
	NOP;
	NOP;
	SET_BIT5(DDRE);		
	NOP;
	NOP;
}

void next_byte()
{
	SET_BIT5(DDRE);	
	NOP;
	NOP;
	DATA_LOW; 		
	NOP;
	NOP;
	CLOCK_HIGH; 	
	NOP;
	NOP;
	CLOCK_LOW; 		
	NOP;
	NOP;
	DATA_HIGH; 	   
	NOP;
	NOP;
}

void done_read()
{
	SET_BIT5(DDRE);	
	NOP;
	NOP;
	DATA_HIGH;  		
	NOP;
	NOP;
	CLOCK_HIGH; 		
	NOP;
	NOP;
	CLOCK_LOW;  		
	NOP;
	NOP;
}

int cbyte(BYTE b)
{
	int i;
	if (b>127) 
	{
		i=(int)b-256;
	}
	else
	{
		i=(int)b;
	}
	return i;
}


void tilt_read()
{
	BYTE tmp;
	
	start_accel();
	
	write_value(Slave_Addr);	  	//? slave Addr write mode
	ack();
	 
	write_value(0x02);  			//?   0000 0010

	ack();

	stop_accel();
	
	NOP; NOP; NOP; NOP;

	start_accel();

	write_value(Slave_Addr+0x01);	  //Save_Addr + ReadBit
	
	ack();

	tmp = read_value();
	
	next_byte();

	x_value = cbyte(read_value());

	next_byte();

	tmp = read_value();

	next_byte();

	y_value = cbyte(read_value());

	next_byte();

	tmp = read_value();
	
	next_byte();

	z_value = cbyte(read_value());

	done_read();	

	stop_accel();
}

void tilt_setup()
{
	start_accel();
	
	write_value(Slave_Addr);	 //write mode
	
	ack();
	 
	write_value(0x14);  		//0001 0100   (Guess - Data Rate and Resolution ?)
 
	ack();

	write_value(0x03); 			//0000 0111   (Guess - Control register - enabel X, Y ,Z axis)
	
	ack();

	stop_accel();
}

#define	SCK_HIGH            SET_BIT4(PORTE)
#define	SCK_LOW             CLR_BIT4(PORTE)
#define	SDI_HIGH            SET_BIT5(PORTE)
#define	SDI_LOW             CLR_BIT5(PORTE)
#define	SCK_SET_OUTPUT      SET_BIT4(DDRE)
#define	SCK_SET_INPUT       CLR_BIT4(DDRE)
#define	SDI_SET_OUTPUT      SET_BIT5(DDRE)
#define	SDI_SET_INPUT       CLR_BIT5(DDRE)
#define P_ACC_SCK(A)	    if(A) SET_BIT4(PORTE);else CLR_BIT4(PORTE)
#define P_ACC_SDI(A)	    if(A) SET_BIT5(PORTE);else CLR_BIT5(PORTE)
#define SDI_CHK				CHK_BIT5(PINE)

//current channel
volatile int gAccX;
volatile int gAccY;
volatile int gAccZ;


//==============================================================//
// Start
//==============================================================//
void AccStart(void)
{
SDI_SET_OUTPUT;
SCK_SET_OUTPUT;
	P_ACC_SDI(1);
	P_ACC_SCK(1);
	NOP;
	NOP;
	P_ACC_SDI(0);
	NOP;
	NOP;
	P_ACC_SCK(0);
	NOP;
	NOP;
}


//==============================================================//
// Stop
//==============================================================//
void AccStop(void)
{
	SDI_SET_OUTPUT;
	SCK_SET_OUTPUT;
	P_ACC_SDI(0);
	P_ACC_SCK(1);
	NOP;
	NOP;
	P_ACC_SDI(1);
	NOP;
	NOP;
	SDI_SET_INPUT;
	SCK_SET_INPUT;
}


//==============================================================//
//
//==============================================================//
void AccByteWrite(BYTE bData)
{
	BYTE	i;
	BYTE	bTmp;

SDI_SET_OUTPUT;
	for(i=0; i<8; i++){
		bTmp = CHK_BIT7(bData);
    	if(bTmp){
			P_ACC_SDI(1);
		}else{
			P_ACC_SDI(0);
		}
		NOP;
		NOP;
		P_ACC_SCK(1);;
		NOP;
		NOP;
		NOP;
		NOP;
		P_ACC_SCK(0);
		NOP;
		NOP;
		bData =	bData << 1;
	}
}


//==============================================================//
//
//==============================================================//
char AccByteRead(void)
{
	BYTE	i;
	char	bTmp = 0;

	SDI_SET_INPUT;
	for(i = 0; i < 8;	i++){
		bTmp = bTmp << 1;
		NOP;
		NOP;
		NOP;
		NOP;
		P_ACC_SCK(1);
		NOP;
		NOP;
		if(SDI_CHK)	bTmp |= 0x01;
		NOP;
		NOP;
		P_ACC_SCK(0);
	}
	SDI_SET_OUTPUT;

	return	bTmp;
}


//==============================================================//
//
//==============================================================//
void AccAckRead(void)
{
SDI_SET_INPUT;
	NOP;
	NOP;
	P_ACC_SDI(1);
	NOP;
	NOP;
	P_ACC_SCK(1);
	NOP;
	NOP;
	P_ACC_SCK(0);
	NOP;
	NOP;
SDI_SET_OUTPUT;
	NOP;
	NOP;
}


//==============================================================//
//
//==============================================================//
void AccAckWrite(void)
{
SDI_SET_OUTPUT;
	NOP;
	NOP;
	P_ACC_SDI(0);
	NOP;
	NOP;
	P_ACC_SCK(1);
	NOP;
	NOP;
	P_ACC_SCK(0);
	NOP;
	NOP;
	P_ACC_SDI(1);
	NOP;
	NOP;
}


//==============================================================//
//
//==============================================================//
void AccNotAckWrite(void)
{
SDI_SET_OUTPUT;
	NOP;
	NOP;
	P_ACC_SDI(1);
	NOP;
	NOP;
	P_ACC_SCK(1);
	NOP;
	NOP;
	P_ACC_SCK(0);
	NOP;
	NOP;
}


//==============================================================//
//==============================================================//
void Acc_init(void)
{
	AccStart();
	AccByteWrite(0x70);
	AccAckRead();
	AccByteWrite(0x14);
	AccAckRead();
	AccByteWrite(0x03);
	AccAckRead();
	AccStop();
}

//==============================================================//
//==============================================================//

void Acc_GetData(void)
{
	//?
}

void AccGetData(void)
{
	signed char	bTmp = 0;

	AccStart();
	AccByteWrite(0x70);
	AccAckRead();
	AccByteWrite(0x02);
	AccAckRead();
	AccStop();

	NOP;
	NOP;
	NOP;
	NOP;

	AccStart();
	AccByteWrite(0x71);
	AccAckRead();

	bTmp = AccByteRead();
	AccAckWrite();

	bTmp = AccByteRead();
	AccAckWrite();
	gAccX = bTmp;

	bTmp = AccByteRead();
	AccAckWrite();

	bTmp = AccByteRead();
	AccAckWrite();
	gAccY = bTmp;

	bTmp = AccByteRead();
	AccAckWrite();

	bTmp = AccByteRead();
	AccNotAckWrite();
	gAccZ = bTmp;

	AccStop();
}

