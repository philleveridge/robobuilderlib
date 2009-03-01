//==============================================================================
//						Digital Input Output Routines
//==============================================================================

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "Main.h"
#include "Macro.h"
#include "dio.h"
#include "comm.h"
#include "battery.h"
#include "rprintf.h"
#include "adc.h"
#include "ir.h"
#include "uart.h"
#include "accelerometer.h"

#include <util/delay.h>


extern WORD    gBtnCnt;				// counter for PF button press
extern WORD    gTltFwd;				// Tilt 
extern WORD    gTltLft;				// Tilt

extern WORD    gMSEC;				// 
extern BYTE    gSEC;				// 
extern BYTE    gMIN;				// 
extern BYTE    gHOUR;				// 

extern int    gX;				// 
extern int    gY;				// 
extern int    gZ;				// 

extern int 	autobalance;
extern int 	response;
extern int  params[];
extern prog_char *version;

void ptime()
{
	rprintf("%d:%d:%d-%d ",gHOUR,gMIN,gSEC,gMSEC);
}

int getDec(int d)
{
	int n=0;
	int ch;
	
	for (int i=0; i<d; i++)
	{
			while ((ch = uartGetByte())<0) ;
			rprintf("%c", ch);	
			if ((ch>=0x30) && (ch<=0x39)) { n = n*10 + (ch-0x30); }
	}
	return n;
}

int getHex(int d)
{
	int n=0;
	int ch;
	
	for (int i=0; i<d; i++)
	{
			while ((ch = uartGetByte())<0) ;
			rprintf("%c", ch);	
			if ((ch>=0x30) && (ch<=0x39)) { n = n*16 + (ch-0x30); }
			if ((ch>=0x41) && (ch<=0x46)) { n = n*16 + (ch-0x41+10); }
	}
	return n;
}


//------------------------------------------------------------------------------
// Perform  Motion when event occurs
// Read_and_Go   (or is it Read_and_WeEP?)
//------------------------------------------------------------------------------


void Read_and_Do(void)
{
	BYTE	lbtmp;
	WORD	ptmpA, ptmpB;
	
	int i;
	int tlt = 0;		
	
	// -------------------------------------------------------------------------------------------------------
	// Check for event and set Action state
	// -------------------------------------------------------------------------------------------------------

	BYTE Action=0xFF;				//No action

	lbtmp = PINA & 0x03;			// buttons are bits 0 and 1 on port A
	if((lbtmp!=0x03))
	{								// low if a button is pressed
		if(++gBtnCnt>100){   		// loop 100 times
			if(lbtmp==0x02){		// if PF1 only is pressed
				Action=0x00;
			}
		}
	}
	else{
	    gBtnCnt=0;
    }
	
	
	// -------------------------------------------------------------------------------------------------------
	// Check for IR event and set Action state
	// -------------------------------------------------------------------------------------------------------
	
	if ( gIRReady) 
	{
		gIRReady = FALSE;
		ptime();
		rprintf(" IR Received %x from %x\r\n", gIRData, gIRAddr);
		if (gIRData==0x01) Action=0x00; // A pressed
		if (gIRData==0x02) Action=0x01; // B pressed
		if (gIRData==0x03) Action=0x04; // Turn left
		if (gIRData==0x04) Action=0x08; // Forward
		if (gIRData==0x05) Action=0x05; // Turn Right
		if (gIRData==0x0A) Action=0x09; // Back
		
		if (gIRData==0x07) Action=0x20; // [] pressed
		if (gIRData==0x1c) Action=0x30; // * + [] pressed   (Flash Lights)
		if (gIRData==0x3F) Action=0x40; // # + 0 pressed    (Run ADC test)
		if (gIRData==0x34) Action=0x50; // # + V pressed    (Run tilt test)
		
	}
	
	// -------------------------------------------------------------------------------------------------------
	// Simple behaviour
	// -------------------------------------------------------------------------------------------------------
	
	if (params[PSDL]!=0 && adc_psd()>params[PSDL])  // if PSD sensor goes off
	{
		ptime(); rprintf("PSD event %x\r\n", psd_value);	
		if (response) 
		{
			//depending on mood :)
			int mood = rand()%4;
			switch(mood)
			{
			case 0:
				Action=0x00; //Punch left 
				break;
			case 1:
				Action=0x01; //punch right
				break;
			case 2:
				Action=0x12; //front kick !
				break;
			case 3:
				Action=0x09; //walk back
				break;
			}
		}
	}
	
	if (adc_mic()>params[MICL])  //if mic semsor levl reached
	{
		ptime(); rprintf("MIC event %x\r\n", mic_vol);	
		if (response) 
		{
			//depending on mood :)
			int mood = rand()%2;
			switch(mood)
			{
			case 0:
				Action=0x10; //sit down
				break;
			case 1:
				Action=0x11; //wave hi
				break;
			}	
		}
	}
	
	
	// -------------------------------------------------------------------------------------------------------
	// terminal input get input and  set Action state
	// -------------------------------------------------------------------------------------------------------

	int ch = uartGetByte();
	if (ch >= 0)
	{
		rprintf("%c", ch);	
		
		if (ch==0x4C) Action=0x02;  	  //'L' pressed
		if (ch==0x52) Action=0x03;       //'R' pressed
		if (ch==0x46) Action=0x08;       //'F' pressed
		if (ch==0x42) Action=0x09;       //'B' pressed


		if (ch==0x7a) Action=0x74;       //'z' pressed
		if (ch==0x77) Action=0x70;       //'w' pressed
		if (ch==0x73) Action=0x71;       //'s' pressed
		if (ch==0x61) Action=0x72;       //'a' pressed
		if (ch==0x64) Action=0x73;       //'d' pressed
		
		if (ch==0x72) Action=0x75;       //'r' pressed

		if (ch==0x70) Action=0x20;       //'p' pressed
		if (ch==0x6c) Action=0x30;       //'l' pressed			
		if (ch==0x69) Action=0x40;       //'i' pressed		
		if (ch==0x74) Action=0x50;       //'t' pressed
		

		if (ch==0x76) Action=0x80;       //'v' pressed
		
		if (ch==0x78)       			  //'x' pressed
		{
			// PC control mode
			// 2 hex digits = length
			int c;
			int l=getHex(2);
			int buff[10];
			// foreach byte
			for (c=0; (c<l && c<10); c++)
			{			
			//  read each hex digit into buffer
				buff[c]=getHex(2);
			}
						
			// transmit  buffer
			for (c=0; (c<l && c<10); c++)
			{	
				sciTx0Data(buff[c]&0xFF);
			}		
			// get response (or timeout)
			
			BYTE b1 = sciRx0Ready();
			BYTE b2 = sciRx0Ready();
			// echo response
			rprintf ("=%x%x\r\n", b1,b2);
		
			Action=0xFF;       
		}
		
		if (ch==0x68 || ch==0x48) Action=0xEE; //'h' pressed
		
		if (ch==0x65) 	  						//'e' pressed 
		{			
			Action=getHex(2);      				//Action code
			rprintf(" Special event [%x]\r\n", Action);	
		}

		if (ch==0x6D) 							// 'm' pressed
		{
			//modify param command (for tuning only)
			int op;
			int pv;
			int pn=getDec(1);

			while ((op = uartGetByte())<0) ;
			rprintf("%c", ch);	
			if (op != '?')
			{
				pv = getHex(2);
			}
			
			if ( (pn>=0) && (pn<10))
			{
				if (op=='+') params[pn] += pv;
				if (op=='-') params[pn] -= pv;
				if (op=='=') params[pn] = pv;
				rprintf(" P[%d]=%x\r\n", pn, params[pn]);
			}
		}

	}
	
	// -------------------------------------------------------------------------------------------------------
	//tilt events (change in X,y,Z by certain limit)
	// -------------------------------------------------------------------------------------------------------
	
	if (autobalance)
	{
			int diff;
			tilt_read(0);
			if (abs(gX-x_value)>5)
			{
				diff = gX-x_value;
				ptime(); rprintf("Tilt event X [%d] (%d,%d,%d)\r\n", diff, x_value, y_value, z_value);	
			}
			if (abs(gY-y_value)>5)
			{
				diff = gY-y_value;
				ptime(); rprintf("Tilt event y [%d] (%d,%d,%d)\r\n", diff, x_value, y_value, z_value);	
			}		
			if (abs(gZ-z_value)>10)
			{
				diff = gZ-z_value;
				ptime(); rprintf("Tilt event z [%d] (%d,%d,%d)\r\n", diff, x_value, y_value, z_value);	
				
				if (diff>0) Action=0x71; else Action=0x70;
			}
	}
	
	// -------------------------------------------------------------------------------------------------------
	// perfom action
	// -------------------------------------------------------------------------------------------------------
	
	switch (Action)
	{
	case 0x00:  //PunchLeft
	case 0x01:  //PunchRight
	case 0x02:  //SidewalkLeft
	case 0x03:	//SidewalkRight
	case 0x04:  //TurnLeft
	case 0x05:  //TurnRight
	case 0x06:  //GetupBack
	case 0x07:  //GetupFront
	case 0x08:  //WalkForward
	case 0x09:  //WalkBackward
	case 0x0A:  //lshoot
	case 0x0B:  //rshoot
	case 0x0C:  //rsidewalk
	case 0x0D:  //lsidewalk
	case 0x0E:  //standupr
	case 0x0F:  //standupf
	case 0x10:  //sitdown
	case 0x11:  //hi
	case 0x12:  //kick left front turn

		SampleMotion(Action);  					// comm.c: perform the sample motion
		ptime(); rprintf("Do Motion %x\r\n", Action);
		break;
	case 0x20:
		ptime(); rprintf("Basic Pose\r\n");
		BasicPose();
		break;
	case 0x30:
		//flash lights
		for (i=0; i<20; i++)
		{
			_delay_ms(250);
			PF1_LED1_OFF;
			_delay_ms(250);
			PF1_LED1_ON;
		}
		break;
	case 0x40:
		ptime(); rprintf("ADC Test - loops until any button on IR pressed\r\n");	
		while(!gIRReady)
		{		
			adc_test(1);  					// ADC test
			_delay_ms(250);
		}
		gIRReady = FALSE;
		break;
	case 0x50:
		ptime(); rprintf("Tilt Test - loops until any button on IR pressed\r\n");	
		while(!gIRReady)
		{
			tilt_read(1);  					// tilt test (DEBUG MODE)
			_delay_ms(250);
		}
		gIRReady = FALSE;
		break;
	case 0x74: //zero (x,y,x)
		if (autobalance)
		{
			ptime(); rprintf("Accel off\r\n");	
			autobalance=0; // off
		}
		else
		{
			tilt_read(0);
			gX=x_value;gY=y_value;gZ=z_value;
			ptime(); rprintf("Accel on - Zero(%d,%d,%d)\r\n", gX,gY,gZ);	
			autobalance=1; // on
		}
		break;
	case 0x75: //respond on/off
		ptime(); rprintf("Response (%x)\r\n", response);	
		response = !response;
		break;
	case 0x70: //lean forward
	case 0x71: //lean back
		ptime(); rprintf("Dynamic Balance Test F/B (%x)\r\n", Action);	
		
		// Knees ID7, ID2
		UCSR0B &= 0xBF;
		UCSR0B |= 0x80;   		// UART0 RxInterrupt enable	
	
		ptmpB = PosRead(0x02);
		ptmpA = PosRead(0x07);
		
		ptime(); rprintf("ReadPos (2)(7) %x - %x, ", ptmpB, ptmpA);	
		
		if (Action == 0x70) tlt = params[TLT];
		if (Action == 0x71) tlt = -params[TLT];
		
		ptmpA = ((ptmpA+tlt) & 0xFF); // must be 0-254
		ptmpB = ((ptmpB-tlt) & 0xFF); // must be 0-254
		
		rprintf("MovePos (%d)  %x - %x\r\n", tlt, ptmpB, ptmpA);	
		
		PosMove(2, params[TORQ], ptmpB);			
		PosMove(7, params[TORQ], ptmpA);

		break;
	case 0x72:  //lean left
	case 0x73:  //lean right
		ptime(); rprintf("Dynamic Balance Test L/R (%x)\r\n", Action);	
		break;
		
	case 0x80:
		//version
		rprintf("Ver=0.5\r\n");
		break;
		
	case 0xD0:
		//experimental
		rprintf("Battery charging\r\n");
		batt_charge();
		break;

	case 0xEE:
		//help
		
		rprintf(
		"elp\r\n" 
		"L  left\r\n"
		"R  right\r\n"
		"F  forward\r\n"
		"B  back\r\n"
		"z  tilt on/off\r\n"
		"i  ADC test (PSD, Mic, Bat Level)\r\n"
		"t  Tilt test (loop showing accelerometer values until IR pressed\r\n"
		"p  Basic pose\r\n"
		"w  lean fwd\r\n"
		"a  lean left\r\n"
		"d  lean right\r\n"
		"s  lean back\r\n"
		"v  version number\r\n"
		"m  modify param [n][+|-|=] [nn]\r\n"
		"r  respond to event on/off\r\n"
		"l  light flash test\r\n"
		"e	[nn] set action event nn (two digits)\r\n"	
		"x	[nn][...] transmit to wCK bus [nn] bytes\r\n");	
		break;	
		
	case 0xFF:
		//Do nothing
		break;
	}
} 


