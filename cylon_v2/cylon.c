#include "interupt.h"

/* ------------------------------------------------
       cylon() - simulate cylon scanner
   ------------------------------------------------ */


#define CYLON_SCAN_DELAY 50

void cylon(unsigned char cylon_style) {

	/* 10 bits */
	
	const unsigned char cylon_bits[] = {
				// 	A1	A0	B7	B6	B5	B4	B3	A3	A2	B0
		0,  0,  //  
		2,	0,  //  1	0	0	0	0	0	0	0	0	0
		3,	0,  //  1	1	0	0	0	0	0	0	0	0
		1,	128,//	0	1	1	0	0	0	0	0	0	0
		0,	192,//	0	0	1	1	0	0	0	0	0	0
		0,	112,//	0	0	0	1	1	1	0	0	0	0
		0,	56, // 	0	0	0	0	1	1	1	0	0	0
		8,	8,  //	0	0	0	0	0	0	1	1	0	0
		12,	0,  //	0	0	0	0	0	0	0	1	1	0
		4,	1,  //	0	0	0	0	0	0	0	0	1	1
		0,	1,  //	0	0	0	0	0	0	0	0	0	1
		0,  0   //
	};
	
	unsigned char i; // array iterator

	if(cylon_style == 0) {

		// traditional (back & forth) cylon scanner

		for(i = 0; i < sizeof(cylon_bits); i=i+2) 
		{		
			PORTA = cylon_bits[i];
			PORTB = cylon_bits[i+1];
			delay(CYLON_SCAN_DELAY);
		}

		i = sizeof(cylon_bits)-2;
		while (1)
		{
			PORTA = cylon_bits[i];
			PORTB = cylon_bits[i+1];
			delay(CYLON_SCAN_DELAY);
			if (i==0) break;
			i=i-2;
		}

	} else if(cylon_style == 1) {

		// single direction scan

		for(i = 0; i < sizeof(cylon_bits); i+=2) 
		{
			PORTA = cylon_bits[i];
			PORTB = cylon_bits[i+1];
			delay(CYLON_SCAN_DELAY);
		}


	} else if(cylon_style == 2) {

		// other direction scan

		i = sizeof(cylon_bits)-2;
		while (1)
		{
			PORTA = cylon_bits[i];
			PORTB = cylon_bits[i+1];
			delay(CYLON_SCAN_DELAY);
			if (i==0) break;
			i-=2;
		}
	}
}

