#include "interupt.h"
#include "sound.h"

typedef unsigned int config;
config at 0x2007 __CONFIG = 	
	_CP_OFF &
	_WDT_OFF &
	_BODEN_OFF &
	_PWRTE_OFF &
	_INTRC_OSC_NOCLKOUT &
	_MCLRE_ON &
	_LVP_OFF;

extern void cylon(unsigned char cylon_style);

int get_byte()
{
	int i;
	
	while(!RCIF)  // wait for character
	{
		nop();
	}

	i= RCREG;
	return i;
}

void put_byte(int i)
{
	while(!TRMT)
	{
		nop();
	}	
	TXREG=i;	// Transmit		
}

void test()
{
	int i=0;
	
			
	PORTA = 0x00;	
	for (i=0; i<`2000; i++)
	{
		PORTA ^= 0x10;	
		delay_us(300);
	}
	
	
	
	for (i=0; i<10; i++)
	{
		delay(200);
		
		PORTA = 0;
		PORTB = 0;
		
		delay(200);
		
		PORTA=0x0F;  //0000 1111	
		PORTB=0xF9;  //1111 1001		
	}
	
	for (i=0; i<10; i++)
	{		`
		delay(200);
		// 1100 
		PORTA=(unsigned char)12; PORTB=(unsigned char)225;

		delay(200);
		// 1011 1110 0001
		PORTA=(unsigned char)11; PORTB=(unsigned char)152;
	}
	
	play_tone(D5, DUR);
	play_tone(Fsh5, 2*DUR);
	play_tone(E5, DUR);
			
	PORTA=(unsigned char)5; PORTB=(unsigned char)72;		
}

void main(void)
{
	static unsigned char i;
	static int d;
	
	init_hw();
	
	test();

	while(1)
	{
		d=get_byte();
		switch (d)
		{
		case 'A':
			cylon(0);
			break;
		case 'B':
			cylon(1);
			break;
		case 'C':
			cylon(2);
			break;
		case 'T':
			play_tone(D5, DUR);
			play_tone(E5, DUR);
			play_tone(D5, DUR);
			play_tone(G5, DUR);
			play_tone(Fsh5, 2*DUR);
			break;
		case 'S': 
			PORTA=(unsigned char)11; PORTB=(unsigned char)152;
			break;
		case 'L': 
			PORTA=(unsigned char)12; PORTB=(unsigned char)225;
			break;
		case 'R': 
			PORTA=(unsigned char)13; PORTB=(unsigned char)200;
			break;
		case 'X': 
			PORTA=(unsigned char)5; PORTB=(unsigned char)72;
			break;
		case 'P': 
			i=get_byte();
			PORTA=i;
			i=get_byte();
			PORTB=i;
			break;
		case 't': 
			d=get_byte();
			i=get_byte();
			play_tone(d,i);
			break;
		}	
		put_byte(d); //echo back

	}
}
