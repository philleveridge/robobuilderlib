// experimental_mode.c
//
//	This is our "experimental" mode where we feel free to hack and try
//	things that may or may not work.  It consists mainly of a lot of little
//	routines that the user can access via the serial interface.

#include <avr/eeprom.h> 
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

extern WORD    gBtnCnt;				// counter for PF button press
extern WORD    gTltFwd;				// Tilt 
extern WORD    gTltLft;				// Tilt

extern int    gX;				// 
extern int    gY;				// 
extern int    gZ;				// 

extern int 	autobalance;
extern int 	response;
extern int  params[];


extern const prog_char version[];

static unsigned char *motionBuf;

void Perform_Action(BYTE action);
BYTE Read_Events(void);

#define EPAUSE {rprintf(">");while (uartGetByte()<0); rprintf("\r\n");}
extern void print_motionBuf(int bytes);

uint8_t EEMEM HUNO_TYPE[1];  			// this is where the tokenised code will be stored

void set_type(uint8_t c)
{
	eeprom_write_byte(HUNO_TYPE, c);	
}

uint8_t get_type()
{
	return eeprom_read_byte(HUNO_TYPE);
}

void delay_ms(int m)
{
	_delay_ms(m);
}

uint8_t get_noservos()
{
	int noservos=0;
	switch (get_type())
	{
	case HUNO_BASIC: 
	    noservos=16;
		break;
	case HUNO_ADVANCED: 
	    noservos=19;
		break;
	case HUNO_OTHER: 
	    noservos=18;
		break;	
	}
	return noservos;
}

void send_bus_str(char *bus_str, int n)
{
			
		BYTE b;
		int ch;
		char *eos = bus_str+n;

		wckReInit(BR9600);
	
		while  ((bus_str<eos) && (b=*bus_str++) != 0)
		{			
			wckSendByte('S');
			wckSendByte(b);
			
			if (b=='p' || b=='t')
			{
				delay_ms(100);	
				if (*bus_str != 0) wckSendByte(*bus_str++);
				delay_ms(100);	
				if (*bus_str != 0) wckSendByte(*bus_str++);
				
			}		
			delay_ms(100);		
			ch = wckGetByte(1000);
			rprintf ("BUS=%d\r\n", ch);
		}
		
		wckReInit(BR115200);
		wckFlush(); // flush the buffer
}


void experimental_binloop();

#define MAX_INP_BUF 32

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

void check_buttons(BYTE *action)
{
	BYTE tmp;

	tmp = PINA & 0x03;			// buttons are bits 0 and 1 on port A
	if((tmp!=0x03))
	{								// low if a button is pressed
		if(++gBtnCnt>100){   		// loop 100 times
			if(tmp==0x02){		// if PF1 only is pressed
				*action=0x00;
			}
		}
	}
	else{
	    gBtnCnt=0;
    }
}


void check_IR(BYTE *action)
{
	if ( gIRReady) 
	{
		gIRReady = FALSE;
		ptime();
		rprintf(" IR Received %x from %x\r\n", gIRData, gIRAddr);
		if (gIRData==0x01) *action=0x00; // A pressed
		if (gIRData==0x02) *action=0x01; // B pressed
		if (gIRData==0x03) *action=0x04; // Turn left
		if (gIRData==0x04) *action=0x08; // Forward
		if (gIRData==0x05) *action=0x05; // Turn Right
		if (gIRData==0x0A) *action=0x09; // Back
		
		if (gIRData==0x07) *action=0x20; // [] pressed
		if (gIRData==0x1c) *action=0x30; // * + [] pressed   (Flash Lights)
		if (gIRData==0x3F) *action=0x40; // # + 0 pressed    (Run ADC test)
		if (gIRData==0x34) *action=0x50; // # + V pressed    (Run tilt test)

		if (gIRData==0x2A) *action=0xC1; // * + 0 pressed    (Run program)
		
	}
}

void check_behaviour(BYTE *action)
{

#ifdef PSD_SENSOR

	if (params[PSDL]!=0 && adc_psd()>params[PSDL])  // if PSD sensor goes off
	{
		if (response) 
		{
			//depending on mood :)
			ptime(); rprintf("PSD event %x\r\n", gDistance);	
			int mood = rand()%4;
			switch(mood)
			{
			case 0:
				*action=0x00; //Punch left 
				break;
			case 1:
				*action=0x01; //punch right
				break;
			case 2:
				*action=0x12; //front kick !
				break;
			case 3:
				*action=0x09; //walk back
				break;
			}
		}
	}
	
#endif
		
	if (adc_mic()>params[MICL])  //if mic semsor levl reached
	{
		if (response) 
		{
			ptime(); rprintf("MIC event %x\r\n", gSoundLevel);	

			//depending on mood :)
			int mood = rand()%2;
			switch(mood)
			{
			case 0:
				*action=0x10; //sit down
				break;
			case 1:
				*action=0x11; //wave hi
				break;
			}	
		}
	}
}

void check_serial(BYTE *action)
{
	int c,l;
	BYTE buff[MAX_INP_BUF];
	
	int ch = uartGetByte();
	if (ch >= 0)
	{
		rprintf("%c", ch);	
		
		if (ch==0x4C) *action=0x02;  	  //'L' pressed
		if (ch==0x52) *action=0x03;       //'R' pressed
		if (ch==0x46) *action=0x08;       //'F' pressed
		if (ch==0x42) *action=0x09;       //'B' pressed


		if (ch==0x7a) *action=0x74;       //'z' pressed
		if (ch==0x77) *action=0x70;       //'w' pressed
		if (ch==0x73) *action=0x71;       //'s' pressed
		if (ch==0x61) *action=0x72;       //'a' pressed
		if (ch==0x64) *action=0x73;       //'d' pressed
		
		if (ch==0x72) *action=0x75;       //'r' pressed

		if (ch==0x70) *action=0x20;       //'p' pressed
		if (ch==0x6c) *action=0x30;       //'l' pressed			
		if (ch==0x69) *action=0x40;       //'i' pressed		
		
		
		if (ch==0x74) *action=0x50;       //'query mode


		if (ch=='!') *action=0xC1;       //'program run mode
		if (ch=='&') *action=0xC0;       //'program entry mode
		if (ch=='#') *action=0xE8; 		// binary mode
		
		
		if (ch=='q' || ch == 'Q' ) 
			*action=0x90;       
		

		if (ch==0x76) *action=0x80;       //'v' pressed
		
		if (ch==27 || ch == 'P') 		// exit to idleMode
		{
			*action=0xFF;       			
			gNextMode = kIdleMode;		
		}
		
		if ('?' == ch) {
			rprintf("Experimental Mode ('h' for help)\r\n");
		}
		
		if (ch==0x78 || ch==0x58)        //'x' or 'X' pressed
		{
			// PC control mode
			// 2 hex digits = length
			
			l=getHex(2);

			// foreach byte
			for (c=0; (c<l && c<MAX_INP_BUF); c++)
			{			
			//  read each hex digit into buffer
				buff[c]=getHex(2);
			}
						
			// transmit  buffer
			for (c=0; (c<l && c<MAX_INP_BUF); c++)
			{	
				sciTx0Data(buff[c]&0xFF);
			}

			if (ch==0x78)
			{
				// get response (or timeout)					
				BYTE b1 = sciRx0Ready();
				BYTE b2 = sciRx0Ready();
				// echo response
				rprintf ("=%x%x\r\n", b1,b2);
			}
			else
			{
				rprintf (" ok\r\n");
			}
		
			*action=0xFF;       
		}
		
		if (ch=='u' )        //like X but at 9600 baud
		{		
			int f=1;
					
			UBRR0H=0x00;
			UBRR0L=BR9600; //motion.h	
			
			while(f)
			{
				rprintf("wcK interactive (at 9600) : ");	
				while ((ch = uartGetByte())<0) ;
				rprintf("%c", ch);	
				
				if (ch != '.')
				{
					wckSendByte('S');
					wckSendByte(ch&0xFF);
					
					if (ch=='p' || ch=='t')
					{
						unsigned char b;
						b = getHex(2);		
						if (b != 0) wckSendByte(b);
						b = getHex(2);		
						if (b != 0) wckSendByte(b);			
					}
					
					_delay_ms(50);
					ch = wckGetByte(1000);
					rprintf ("%d\r\n", ch);
				}
				else
				{
					f=0;
				}
			}
			UBRR0H=0x00;
			UBRR0L=BR115200;
		}
		

		if (ch==0x68 || ch==0x48) 				//'h' pressed
			*action=0xEE; 
		
		if (ch==0x65) 	  						//'e' pressed 
		{			
			*action=getHex(2);      				//Action code
			rprintf(" Special event [%x]\r\n", *action);	
		}

		if (ch==0x4D) 							// 'M' pressed
		{
			//modify param command (for tuning only)
			int op;
			int pv=0;
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
		
	
		if (ch==0x6D) 							// 'm' pressed
		{
			int nb=0;
			int b0;
			int b1;
			
			while ((b0=uartGetByte())<0);
			while ((b1=uartGetByte())<0);
			int bytes = ((int)b1 << 8) | b0;

			motionBuf = GetNextMotionBuffer();
			
			rprintf("^");
	
			while (nb<bytes)
			{				
				while ((b0=uartGetByte())<0);
				motionBuf[nb++] = b0;
			}
			rprintf("Received %d bytes\r\n", nb);

			print_motionBuf(nb);
			LoadMotionFromBuffer(motionBuf);			
			PlaySceneFromBuffer(motionBuf, 0);
			complete_motion(motionBuf);
			
			rprintf("Done\r\n", nb);

		}
		
	}
}

void check_balance(BYTE *action)
{
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
			
			if (diff>0) *action=0x71; else *action=0x70;
		}
	}
}

//------------------------------------------------------------------------------
// Check if an event has occured that causes an Action
//------------------------------------------------------------------------------

BYTE Read_Events(void)
{
	BYTE Action=0xFF;				//No action

	// -------------------------------------------------------------------------------------------------------
	// Check for event and set Action state
	// -------------------------------------------------------------------------------------------------------

	check_buttons(&Action);
		
	// -------------------------------------------------------------------------------------------------------
	// Check for IR event and set Action state
	// -------------------------------------------------------------------------------------------------------
	
	check_IR(&Action);
	
	// -------------------------------------------------------------------------------------------------------
	// Simple behaviour
	// -------------------------------------------------------------------------------------------------------
	
	check_behaviour(&Action);	
	
	// -------------------------------------------------------------------------------------------------------
	// terminal input get input and  set Action state
	// -------------------------------------------------------------------------------------------------------

	check_serial(&Action);
	
	// -------------------------------------------------------------------------------------------------------
	//tilt events (change in X,y,Z by certain limit)
	// -------------------------------------------------------------------------------------------------------
	
	check_balance(&Action);

	return Action;
}
	
// -------------------------------------------------------------------------------------------------------
// perfom action
// -------------------------------------------------------------------------------------------------------

void Perform_Action (BYTE Action)
{	

	BYTE	lbtmp;
	WORD	ptmpA, ptmpB;
	
	int i;
	int tlt = 0;		

	if (Action>=0 && Action <=0x12)
	{
	/*
		0x00:  //PunchLeft
		0x01:  //PunchRight
		0x02:  //SidewalkLeft
		0x03:	//SidewalkRight
		0x04:  //TurnLeft
		0x05:  //TurnRight
		0x06:  //GetupBack
		0x07:  //GetupFront
		0x08:  //WalkForward
		0x09:  //WalkBackward
		0x0A:  //lshoot
		0x0B:  //rshoot
		0x0C:  //rsidewalk
		0x0D:  //lsidewalk
		0x0E:  //standupr
		0x0F:  //standupf
		0x10:  //sitdown
		0x11:  //hi
		0x12:  //kick left front turn

		if (get_type()==HUNO_BASIC)
		{
			SampleMotion(Action);  					// comm.c: perform the sample motion
			ptime(); rprintf("Do Motion %x\r\n", Action);
		}
		else
		{
			ptime(); rprintf("Only when in HUNO BASIC mode\r\n");
		}
		*/
	}
	else
	switch (Action)
	{

	case 0x20:
		ptime(); rprintf("Basic Pose\r\n");
		//if (get_type()==HUNO_BASIC)
		//{
		//	BasicPose();
		//}
		//else
		{
			rprintf("N/A\r\n");
		}		
		break;
	case 0x30:
		//flash lights
		for (i=0; i<20; i++)
		{
			PF1_LED1_ON;
			_delay_ms(250);
			PF1_LED1_OFF;
			_delay_ms(250);
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
		//UCSR0B &= 0xBF;
		//UCSR0B |= 0x80;   		// UART0 RxInterrupt enable	
	
		ptmpB = wckPosRead(0x02);
		ptmpA = wckPosRead(0x07);
		
		ptime(); rprintf("ReadPos (2)(7) %x - %x, ", ptmpB, ptmpA);	
		
		if (Action == 0x70) tlt = params[TLT];
		if (Action == 0x71) tlt = -params[TLT];
		
		ptmpA &= 0xFF;
		ptmpB &= 0xFF;
		
		ptmpA = ((ptmpA+tlt) & 0xFF); // must be 0-254
		ptmpB = ((ptmpB-tlt) & 0xFF); // must be 0-254
		
		rprintf("MovePos (%d)  %x - %x\r\n", tlt, ptmpB, ptmpA);	
		
		wckPosSend(2, params[TORQ], ptmpB);			
		wckPosSend(7, params[TORQ], ptmpA);

		break;
	case 0x72:  //lean left
	case 0x73:  //lean right
		ptime(); rprintf("Dynamic Balance Test L/R (%x)\r\n", Action);	
		break;
		
	case 0x80:
		//version
		rprintf("er=");
		rprintfProgStr(version);
		rprintf("Built: " __DATE__ "\r\n");  // Must have CR, else PC code hangs
		break;
		

	case 0x90:
		//query mode
		{
		int n=get_noservos();
		rprintf("%x,", n);	
		
		for (BYTE id=0; id<n; id++)
		{
				ptmpA = wckPosAndLoadRead(id);
				rprintf("%x", ptmpA);	
		}
		lbtmp=adc_psd();
		rprintf("%x", lbtmp);	
		lbtmp=adc_mic();
		rprintf("%x", lbtmp);	
		ptmpA=adc_volt();
		rprintf("%x", ptmpA);
		tilt_read(0);
		rprintf("%x", x_value);
		rprintf("%x", y_value);
		rprintf("%x  ", z_value);
		ptime(); 
		rprintf("\r\n");	
		}
		break;
	case 0x91:
		//query decimal mode
		{
		int n=get_noservos();
		rprintf("%d,", n);	

		for (BYTE id=0; id<n; id++)
		{
				ptmpA = wckPosAndLoadRead(id);
				rprintf("%d,", ptmpA);	
		}
		lbtmp=adc_psd();
		rprintf("%d,", lbtmp);	
		lbtmp=adc_mic();
		rprintf("%d,", lbtmp);	
		ptmpA=adc_volt();
		rprintf("%d,", ptmpA);
		tilt_read(0);
		rprintf("%d,", (int)x_value);
		rprintf("%d,", (int)y_value);
		rprintf("%d  ", (int)z_value);
		ptime(); 
		rprintf("\r\n");	
		}
		break;	
		
	// very experimental - BASIC
	// Short term aim - simple eprograms controlling motions
	/*
	case 0xC0:
		basic_load();
		break;
	case 0xC1:
		basic_run(0);
		break;		
	case 0xC2:
		basic_clear();
		break;	
	case 0xC3:
		basic_list();
		break;
	case 0xC4:
		basic_list();
		basic_run(1);
		break;				
	case 0xC5:
		dump_firmware();
		break;	
	//case 0xC6:
	//	basic_download();
	//	break;	
	case 0xC7:
		dump();
		break;	
		
		//Robot type - prep for 20 dof	
	case 0xD8:
		switch(get_type())
		{
		case HUNO_BASIC:
			rprintf("Robot type - BASIC\r\n"); break;
		case HUNO_ADVANCED:
			rprintf("Robot type - ADANCED\r\n"); break;
		case HUNO_OTHER:
			rprintf("Robot type - OTHER\r\n"); break;
		}
		break;	
	case 0xD9:
		rprintf("set Robot type Huno\r\n");
		set_type(HUNO_BASIC);
		break;	
	case 0xDA:
		rprintf("set Robot type Adv\r\n");
		set_type(HUNO_ADVANCED);
		break;		
	case 0xDB:
		rprintf("set Robot type Other\r\n");
		set_type(HUNO_OTHER);
		break;	
		*/
		
	case 0xD0:
		//experimental
		gNextMode = kChargeMode;
		break;	
		
	case 0xE8:
		//binary mode
		experimental_binloop();
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
		"q  Display all servo positions\r\n"
		"w  lean fwd\r\n"
		"a  lean left\r\n"
		"d  lean right\r\n"
		"s  lean back\r\n"
		"v  version number\r\n"
		"M  modify param [n][+|-|=] [nn]\r\n"
		"r  respond to event on/off\r\n"
		"l  light flash test\r\n"
		"e	[nn] set action event nn (two digits)\r\n"	
		
		"x	[nn][...] transmit to wCK bus [nn] bytes ans wait for 2 nyte response\r\n"
		"X	as above but no reponse packets\r\n"
		"m  move - synchronous move - as serialslavee mode but waits\r\n"
		
		"&  Enter a new program\r\n"
		"?  Display current mode\r\n"
		"!  Run current program in memory\r\n"

		"#  binary mode\r\n"
		);	
		break;	
	}
} 


void experimental_mainloop()
{
	WORD lMSEC;
	rprintf ("Experimental mode\r\n");
	while (kExperimentalMode == gNextMode) {
		lMSEC = gMSEC;
		
		BYTE act = Read_Events();
		
		if (act != 0xFF) Perform_Action(act);
		
		while(lMSEC==gMSEC);
	}
}







