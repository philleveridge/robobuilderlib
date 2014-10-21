//==============================================================================
//	 DCMP - Direct Control Mode Plus
//   Homebre firmware for Robobuilder RBC control unit
//   by l3v3rz
//==============================================================================
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "main.h"
#include "macro.h"
#include "adc.h"
#include "global.h"
#include "accelerometer.h"
#include "ir.h"

#include "uart.h"
#include "rprintf.h"
#include "wckmotion.h"

// Standard Input/Output functions


#define BR115200			7 
#define DATA_REGISTER_EMPTY (1<<UDRE)

#define 	MAJOR_VER	3
#define 	MINOR_VER	20

extern volatile WORD	mstimer;

//defined adc.c
extern void Acc_init(void);
extern void AccGetData(void);
extern BYTE sData[];
extern int 	sDcnt;
extern void sample_sound(int);
extern volatile BYTE   MIC_SAMPLING;

void printint(int);
void printline(char *c);

extern BYTE F_PS_PLUGGED;
extern void delay_ms(int);
extern void BreakModeCmdSend(void);
extern void ChargeNiMH(void);
extern void SelfTest1(void);
extern void DetectPower(void);

//define ir.c
extern int irGetByte(void);


// UART variables-----------------------------------------------------------------

WORD	gRx1Step;
BYTE	gRxData;
WORD	gRx1_DStep;
BYTE	gFileCheckSum;
BYTE	gCMD;


extern int dbg;
extern volatile BYTE   gSEC;
extern volatile BYTE   gMIN;
extern volatile BYTE   gSoundLevel;
extern void     lights(int n);
extern void     Get_AD_MIC(void);

BYTE 	outb[10];
BYTE 	outbc;
BYTE	gCNT;
BYTE	gAddr;

int flag=1;

void dcmp_recv();

int dcmp_mode() 
{
	int b;	
	rprintfProgStr (PSTR("DCMP MODE\r\n"));

	while (flag)
	{ 
		if ((b=uartGetByte())>=0)
		{
			//  character on input (rx0) -> write to wclBus (tx1)
			wckSendByte(b);
			dcmp_recv(b);
		}
		if ((b=wckGetByte(0))>=0)
		{
			//  character on input (rx1) -> write to pcBus (tx0)
			rprintfChar(b);
		}
		delay_ms(1);
	}

	rprintfProgStr (PSTR("DCMP EXIT\r\n"));
}

void dcmp_recv(int ch) 
{
		
	if(ch == 0xff){
		gRx1_DStep = 1;
		gFileCheckSum = 0;
		return;
	}
	
	switch(gRx1_DStep){
		case 1:
			if(ch == 0xBE) // check for 101 11110   (read Position Servo 30)
			{
				gRx1_DStep = 2;
			}
			else 
				gRx1_DStep = 0;
			gFileCheckSum ^= ch;
			break;
		case 2:
			gCMD = ch;
			if (gCMD==0x0D || gCMD==0x0E)
				gRx1_DStep = 4;
			else			
				gRx1_DStep = 3;
			gFileCheckSum ^= ch;
			break;
		case 4:
			// IC2_on and IC2_out
			// ch will be numer of bytes to follow
			gAddr=ch;
			gRx1_DStep=5;
			break;
		case 5:
			// IC2_on and IC2_out
			// ch will be numer of bytes to follow
			gCNT=ch;
			outbc=0;
			gRx1_DStep=6;
			gFileCheckSum=0;
			break;
		case 6:
			if (outbc < gCNT)
			{
				gFileCheckSum ^= ch;
				outb[outbc] = ch;
				outbc++;
			}
			if(outbc==gCNT)
			{
				PF1_LED2_OFF; 
				gRx1_DStep=3;
			}			
			break;
		case 3:
			if (dbg) PF1_LED1_ON;
			if(ch == (gFileCheckSum & 0x7f))
			{
				// Depends on gGMD		
				BYTE b1=0, b2=0;
				
				if (gCMD>0x40 && gCMD<(0x40+26))
				{
					//PLAY_SOUND = gCMD-0x40;
					gCMD=0;
					return;
				}

				if ((gCMD>=0x20 && gCMD<=0x27) || (gCMD>=0x30 && gCMD<=0x37) )
				{
  					//Switch LED ON or OFF
					if (gCMD>=0x20 && gCMD<=0x2F)
					{
						if ((gCMD&1)==1) RUN_LED2_ON; else RUN_LED2_OFF;
						if ((gCMD&2)==2) RUN_LED1_ON; else RUN_LED1_OFF;
						if ((gCMD&4)==4) PWR_LED1_ON; else PWR_LED1_OFF;
						if ((gCMD&8)==8) PWR_LED2_ON; else PWR_LED2_OFF;
					}

					if (gCMD>=0x30 && gCMD<=0x3F)
					{
						if ((gCMD&1)==1) ERR_LED_ON;  else ERR_LED_OFF;
						if ((gCMD&2)==2) PF1_LED1_ON; else PF1_LED1_OFF;
						if ((gCMD&4)==4) PF1_LED2_ON; else PF1_LED2_OFF;
						if ((gCMD&8)==8) PF2_LED_ON;  else PF2_LED_OFF;
					}

					b1 =    (((PORTA&(1<<6)) != 0)?  1:0) + //  PA6 RUN G
						(((PORTA&(1<<5)) != 0)?  2:0) + //  PA5 RUN b
						(((PORTC&(1<<7)) != 0)?  4:0) + //  PC7 PWR R
						(((PORTG&(1<<2)) != 0)?  8:0) + //  PG2 PWR G
						(((PORTA&(1<<7)) != 0)? 16:0) + //  PA7 ERROR R
						(((PORTA&(1<<3)) != 0)? 32:0) + //  PA3 PF1 R
						(((PORTA&(1<<2)) != 0)? 64:0) + //  PA2 PF1 B
						(((PORTA&(1<<4)) != 0)?128:0);  //  PA4 Pf2 O
				}

				switch (gCMD)
				{
				case 0x00:
					b1=MAJOR_VER;
					b2=MINOR_VER;
					break;
				case 0x01:  
					Acc_GetData();
					b1 = y_value;
					b2 = z_value;
					break;
				case 0x02:
					Acc_GetData();
					b1 = x_value;
					b2 = z_value;	
					break;
				case 0x03:
					PSD_on();
					break;
				case 0x04:
					PSD_off();
					break;
				case 0x05:
					Get_AD_PSD();
					b1 = gDistance;
					break;
				case 0x06:
					Get_VOLTAGE();
					b1 = gVOLTAGE/256;
					b2 = gVOLTAGE%256;
					break;
				case 0x07:
					b1 = irGetByte();
					b2 = PINA & 0x03;
					break;					
				case 0x08:
					sample_sound(1); // on 
					break;
				case 0x09:
					sample_sound(0); // on 
					break;					
				case 0x0A:
					{
					WORD t=0;
					int lc;
					for (lc=0; lc<SDATASZ; lc++) 
					{
						lights(sData[lc]);
						t += sData[lc];  // sum the buffer
						sData[lc]=0;     // and clear
					}
					b1=t/256;
					b2=t%256;
					}
					break;					
				case 0x0B:
					b1 = gSEC;
					b2 = gMIN;
					break;	
				case 0x0C:
					Get_AD_MIC();
					b1 = gSoundLevel;
					lights(b1);
				case 0x0D:
					{
						BYTE inb[10];
						int icnt=outb[0];
						I2C_read (gAddr, gCNT-1, outb+1, icnt, inb);
						for (int k=0; k<icnt; k++)
						{
							rprintfChar(inb[k]);
						}
						gCMD=0;
						return;
					}
					break;
				case 0x0E:
					b1=I2C_write (gAddr, gCNT, outb) ;
					b2=0;
					break;	
				case 0x0F: // multi-data return (x,y,z,PSD,IR,buttons,sound) - 7 bytes
					Get_AD_PSD();
					rprintfChar(gDistance);
					Acc_GetData();
					rprintfChar(x_value);
					rprintfChar(y_value);
					rprintfChar(z_value);
					rprintfChar(irGetByte());
					rprintfChar(PINA & 0x03);
					{
						WORD t=0;
						int lc;
						for (lc=0; lc<SDATASZ; lc++) 
						{
							t += sData[lc];  // sum the buffer
							sData[lc]=0;     // and clear
						}
						rprintfChar(t % 256);
					}
					gRx1_DStep = 0;
					gCMD=0;
					return;
				case 0x10: // Complete sound buffer multi-data return 
					rprintfChar(SDATASZ);
					{
						int lc;
						for (lc=0; lc<SDATASZ; lc++) 
						{
							rprintfChar(sData[lc]);     // and clear
						}
					}
					break;
				case 0x11: // Lights
					b1 =    (((PORTA&(1<<6)) != 0)?  1:0) + //  PA6 RUN G
						(((PORTA&(1<<5)) != 0)?  2:0) + //  PA5 RUN b
						(((PORTC&(1<<7)) != 0)?  4:0) + //  PC7 PWR R
						(((PORTG&(1<<2)) != 0)?  8:0) + //  PG2 PWR G
						(((PORTA&(1<<7)) != 0)? 16:0) + //  PA7 ERROR R
						(((PORTA&(1<<3)) != 0)? 32:0) + //  PA3 PF1 R
						(((PORTA&(1<<2)) != 0)? 64:0) + //  PA2 PF1 B
						(((PORTA&(1<<4)) != 0)?128:0);  //  PA4 Pf2 O
					b2 = PINA & 0x03;
					break;
				case 0x12: // Lights off
					//gNHB=0;
					break;
				case 0x13: // Lights on
					//gNHB=1;
					break;
				case 0x14: // Exit DCMP
					flag=0;
					break;

				}			
				rprintfChar(b1);				
				rprintfChar(b2);
				gCMD=0;
			}
			else
			{
				rprintfChar(gFileCheckSum);
				rprintfChar(ch);
				gCMD=0;
			}
			gRx1_DStep = 0;
			break;
	}
	return;
}






