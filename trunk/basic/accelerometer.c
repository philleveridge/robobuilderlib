// Standard Input/Output functions
#include <stdio.h>

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "global.h"
#include "macro.h"
#include "math.h"
#include "rprintf.h"
#include "accelerometer.h"

#define NOP			   asm("nop")

#define Slave_Addr		0x70

//current channel
volatile int x_value;
volatile int y_value;
volatile int z_value;

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


//==============================================================//
// Start
//==============================================================//
void I2C_Start(void)
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
void I2C_Stop(void)
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
void I2C_ByteWrite(BYTE bData)
{
	BYTE	i;
	BYTE	bTmp;

	SDI_SET_OUTPUT;
	for(i=0; i<8; i++)
	{
		bTmp = CHK_BIT7(bData);
    	if(bTmp)
		{
			P_ACC_SDI(1);
		}
		else
		{
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
char I2C_ByteRead(void)
{
	BYTE	i;
	char	bTmp = 0;

	SDI_SET_INPUT;
	for(i = 0; i < 8;	i++)
	{
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
void I2C_AckRead(void)
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
void I2C_AckWrite(void)
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
void I2C_NotAckWrite(void)
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

/*************************************************************************8

Generic IC2 routines to support any device on the bus

http://www.robot-electronics.co.uk/htm/using_the_i2c_bus.htm

**************************************************************************/

int I2C_write (int addr, int ocnt, BYTE * outbuff) 
{
// send ocnt bytes from outbuff to addr
// return ack==1 or nak==0
// TBD :: need to add timeout function around ack();

	int i=0;
	
	I2C_Start();

	I2C_ByteWrite(addr);	  	//? slave Addr write mode
	I2C_AckRead();
	 
	for (i=0; i<ocnt; i++)
	{
		I2C_ByteWrite(outbuff[i]);
		I2C_AckRead();
	}
	I2C_Stop();

	return 1;
}

void I2C_read (int addr, int ocnt, BYTE *outbuff, int icnt, BYTE *inbuff)
{
	// send ocnt bytes from outbuff to addr and then read ibuf bytes into inbuff
	int i=0;
	
	I2C_Start();

	I2C_ByteWrite(addr);	  	//? slave Addr write mode
	I2C_AckRead();
	
	for (i=0; i<ocnt; i++)
	{
		I2C_ByteWrite(outbuff[i]);
		I2C_AckRead();
	}
	
	// do I need a wait here ?
	
	for (i=0; i<icnt; i++)
	{
		inbuff[i] = I2C_ByteRead();
		if (i<icnt-1) 
			I2C_AckWrite();
	}
	
	I2C_NotAckWrite();
	I2C_Stop();
}


//==============================================================


//==============================================================

void Acc_init(void)
{
	I2C_Start();
	I2C_ByteWrite(Slave_Addr);
	I2C_AckRead();
	I2C_ByteWrite(0x14);
	I2C_AckRead();
	I2C_ByteWrite(0x03);
	I2C_AckRead();
	I2C_Stop();
}

//==============================================================


//==============================================================

void Acc_GetData(void)
{
	BYTE bTmp = 0;

	I2C_Start();
	I2C_ByteWrite(Slave_Addr);
	I2C_AckRead();
	I2C_ByteWrite(0x02);
	I2C_AckRead();
	I2C_Stop();

	NOP;
	NOP;
	NOP;
	NOP;

	I2C_Start();
	I2C_ByteWrite(Slave_Addr + 1);
	I2C_AckRead();

	bTmp = I2C_ByteRead();
	I2C_AckWrite();

	bTmp = I2C_ByteRead();
	I2C_AckWrite();
	x_value = cbyte(bTmp);

	bTmp = I2C_ByteRead();
	I2C_AckWrite();

	bTmp = I2C_ByteRead();
	I2C_AckWrite();
	y_value = cbyte(bTmp);

	bTmp = I2C_ByteRead();
	I2C_AckWrite();

	bTmp = I2C_ByteRead();
	I2C_NotAckWrite();
	z_value = cbyte(bTmp);

	I2C_Stop();
}

//==============================================================
// test I2C routines
//==============================================================
extern int	delay_ms	(int d);

void testI2C()
{
	BYTE ob[10];
	BYTE ib[10];
	
	rprintfStr ("I2C test");

	// acc init
	ob[0]=0x14; ob[1]=0x03;
	I2C_write(0x70, 2, ob);
	
	for (int z=0; z<20; z++)
	{
		// acc get
		ob[0]=0x02;
		I2C_write(0x70, 1, ob);
		I2C_read (0x71, 0, ob, 6, ib);
		
		rprintf ("%d, %d, %d\n", cbyte(ib[1]), cbyte(ib[3]), cbyte(ib[5]));
		
		delay_ms(250);
	}
}



