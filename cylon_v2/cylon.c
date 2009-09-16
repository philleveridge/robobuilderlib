#include "interupt.h"

/* ------------------------------------------------
       cylon() - simulate cylon scanner
 
A1	A0	B7	B6	B5	B4	B3	A3	A2	B0	PA	PB
										
1	1	1	1	1	1	1	1	1	1	15	249
1	0	0	0	0	0	0	0	0	0	2	0
1	1	0	0	0	0	0	0	0	0	3	0
0	1	1	0	0	0	0	0	0	0	1	128
0	0	1	1	0	0	0	0	0	0	0	192
0	0	0	1	1	1	0	0	0	0	0	112
0	0	0	0	1	1	1	0	0	0	0	56
0	0	0	0	0	0	1	1	0	0	8	8
0	0	0	0	0	0	0	1	1	0	12	0
0	0	0	0	0	0	0	0	1	1	4	1
0	0	0	0	0	0	0	0	0	1	0	1
*/


#define CYLON_SCAN_DELAY 50

void cylon(unsigned char cylon_style) {

	/* 10 bits */
	const unsigned char cylon_bits_a[] = {0, 2,3,1,  0,  0,  0, 8,12,4,0, 0};
	const unsigned char cylon_bits_b[] = {0, 0,0,128,192,112,56,8,0, 1,1, 0};
	
	const unsigned char mask_a = 0xf0;
	const unsigned char mask_b = 0x06;
	
	unsigned char i; // array iterator

	if(cylon_style == 0) {

		// traditional (back & forth) cylon scanner

		for(i = 1; i < sizeof(cylon_bits_a); i++) 
		{		
			PORTA &= mask_a;
			PORTA |= cylon_bits_a[i];
			PORTB &= mask_b;
			PORTB |= cylon_bits_b[i];
			delay(CYLON_SCAN_DELAY);
		}

		i = sizeof(cylon_bits_a)-2;
		while (1)
		{
			PORTA = cylon_bits_a[i];
			PORTB = cylon_bits_b[i];
			delay(CYLON_SCAN_DELAY);
			if (i==0) break;
			i--;
		}

	} else if(cylon_style == 1) {

		// single direction scan

		for(i = 0; i < sizeof(cylon_bits_a); i++) 
		{
			PORTA = cylon_bits_a[i];
			PORTB = cylon_bits_b[i];
			delay(CYLON_SCAN_DELAY);
		}


	} else if(cylon_style == 2) {

		// other direction scan

		i = sizeof(cylon_bits_a)-1;
		while (1)
		{
			PORTA = cylon_bits_a[i];
			PORTB = cylon_bits_b[i];
			delay(CYLON_SCAN_DELAY);
			if (i==0) break;
			i--;
		}
	}
}

