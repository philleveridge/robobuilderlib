#ifdef AVR
#include <avr/eeprom.h> 
#include <avr/pgmspace.h>

#include "global.h"
#include "main.h"
#include "macro.h"

#include "adc.h"
#include "ir.h"
#include "accelerometer.h"
#include "wckmotion.h"
#include "rprintf.h"
#endif

#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include "win.h"
#endif

#ifdef LINUX
#include "linux.h"
#endif

#include "edit.h"
#include "express.h"
#include "functions.h"

#define SCENESZ 128

extern int				scene[];	  // generic array
extern unsigned char	cpos[];
extern int				nis;
extern int				BasicErr;
extern BYTE				sData[];
extern int 				sDcnt;
extern BYTE				nos;
extern volatile BYTE	MIC_SAMPLING;
extern volatile WORD	gtick;

extern BYTE EEMEM		FIRMWARE        	[];  // leave blank - used by Robobuilder OS
extern BYTE EEMEM		BASIC_PROG_SPACE	[];  // this is where the tokenised code will be stored
extern BYTE EEMEM		PERSIST				[];  // persistent data store

const unsigned char map[] = {0, 7, 6, 4, 8, 5, 2, 17, 3, 0, 9, 1, 10, 11, 12, 13, 14, 15, 16, 18, 19};

extern long epop(); //from express.c

const unsigned char smap[40] = {
	1,  2,  3,  4,  5,  6,  8,  10, 12, 15, 19, 24, 31, 38, 47, 57, 69, 82, 97, 112,
	128,144,159,174,187,199,209,218,225,232,237,241,244,246,248,250,251,252,253,254};

int get_special(char **str, long *res, int t)
{
	long v=*res;	
	int lc;

	switch(t) {
	case sSQRT: // $SQRT(x)
		v = Sqrt(v);
		break;
	case sSIN: // $SIN(x)
		v = Sin(v%256);
		break;
	case sABS: // $ABS(x)
		v = v<0?-v:v;
		break;
	case sCOS: // $COS(x)
		v = Cos(v%256);
		break;
	case sCVB2I: // $CVB2I(x)  255 -> -1
		v = cbyte(v);
		break;
	case sGREY: 
		// "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\|()1{}[]?-_+~<>i!lI;:,"^`'. "
		v =  " .:-=+*#%@"[v%10];
		break;
	case sTURTLE: 
		{
		char *p="^>V<";
		char *t = strchr(p,(char)v);
		if (t==(char *)0)
			v= -1;
		else
			v= t-p;
		}
		break;
	case sIR2ACT: //$IR2ACT(10) -> x
		v=(long)map[(int)v];
		break;
	case sNORM:
		for (lc=0; lc<nis; lc++)
		{
			v += (scene[lc]*scene[lc]);
		}
		v=Sqrt(v);
		break;
	case sMIN:
		v=scene[0];
		for (lc=0; lc<nis; lc++)
		{
			if (scene[lc]<(int)v) v=(long)scene[lc];
		}
		break;
	case sSUM:
		v=0;
		for (lc=0; lc<nis; lc++)
		{
			v += (long)scene[lc];
		}
		break;
	case sDSIG: // $DISG(x) -> x*(1-x)
		v = v*(1-v);
		break;
	case sSERVO: // SERVO(nn)
		//$servo
		if (v==32)
			v=readservos(0);
		else
			v=wckPosRead(v);
		break;
	case sMIC:
		for (lc=0; lc<SDATASZ; lc++) 
		{
			v += sData[lc];  // sum the buffer
			sData[lc]=0;     // and clear
		}
		MIC_SAMPLING=1; // on by default, but make sure
		break;
	case sSTAND: // $STAND(x)
		if (v<1)  v=1;
		if (v>18) v=18;
		for (lc=0; lc<v; lc++)
		{
			if (v<=16)
				scene[lc]=basic16[lc];
			else
				scene[lc]=basic18[lc];
		}
		nis=lc;
		return ARRAY;
	case sZEROS: // $ZEROS(x)
		for (lc=0; lc<(int)v && lc<SCENESZ; lc++)
		{
			scene[lc]=0;
		}
		nis=lc;
		return ARRAY;
	case sGX:
		Acc_GetData();
		v=x_value;
		break;	
	case sGY:
		Acc_GetData();
		v=y_value;
		break;		
	case sGZ:
		Acc_GetData();
		v=z_value;
		break;	
	case sNE:
		v=nis;
		break;	
	case sTYPE:
	case sNS:
		v=nos;
		break;	
	case sPSD:
		Get_AD_PSD();
		v = gDistance;
		break;
	case sVOLT:
		Get_VOLTAGE();
		v = gVOLTAGE;
		break;
	case sIR:
		while ((v= irGetByte())<0) ; //wait for IR
		break;
	case sKBD:
		while ((v= uartGetByte())<0) ; // wait for input
		break;	
	case sKIR:
		v=uartGetByte();
		if (v<0) v=irGetByte();
		break;
	case sTICK:
		v=gtick;
		break;
	case sRND: 
		if (v==3)
		{
			long a=0,b=0,c=0;
			c=epop();
			b=epop();
			a=epop();
			if (b>c) swap(&b,&c);

			for (lc=0; lc<a; lc++)
			{
				scene[lc]=(rand()%(c-b))+b;
			}
			nis=(int)a;
			return ARRAY;
		}
		else
		   v=rand();
		break;	
	case sSIG: //$SIG(n,t)
		if (v==2) 
		{
			long r=epop();
			v=epop();
			v=sigmoid(v,r);
		}
		break;	
	case sRANGE:
		if (v==3) 
		{
			long a,b,c;
			c=epop();
			b=epop();
			a=epop();
			if (a<b) 
				v=b;
			else
			if (a>c) 
				v=c;
			else 
				v=a;
		}
		break;
	case sFIND:
		if (v==2) 
		{
			int n;
			epop(); // this will be ZERO
			n=(int)epop();
			for (lc=0; lc<nis; lc++)
			{
				if (scene[lc]==n)
				{
					v=(long)lc; break;
				}
				if (scene[lc]>n)
				{
					if (lc>0) v=(long)(lc-1); 
					break;
				}
			}
			if (lc==nis) v=(long)(nis-1); // no match
		}
		break;		
	case sPORT: // PORT:A:n
		v=0;
		if (**str==':') {
			(*str)++;
			}
		if (**str>='A' && **str<='G' ) {
			v= (**str-'A');
			(*str)++;
			}	
		t=8;
		if (**str==':') {   // Optional Bit specficied
			(*str)++;
			if (**str>='0' && **str<='7' ) {
				t=  (**str - '0');
				(*str)++;
				}
		}				
		*res=get_bit(v, t);         //need to read port with PINA etc 
		return NUMBER; 

	case sROM: // $ROM(x) or $ROM$(X) or $ROM@(x)
		v = eeprom_read_byte((BYTE*)(FIRMWARE+v));
		break;
/*              if (**str=='$')
                {
			v = eeprom_read_byte((BYTE*)(PERSIST+v));
		}
                if (**str=='@')
                {
			nis =(int) v;
			for (lc=0;lc<nis;lc++)
				scene[lc] = eeprom_read_byte((BYTE*)(PERSIST+lc));
		}*/
	case sMAX:
	case sIMAX:
		// MAX(@A,[n]) 
		{
		int m=scene[(int)v];
		int k;
		for (lc=0; lc<nis  && lc<SCENESZ; lc++)
		{
			if (scene[lc]>m) { m=scene[lc]; k=lc;}
		}
		v=(t==sMAX)?m:k;
		}
		break;	
	case sHAM:
		// calculate HAMMING distance between 2 arrays
		// $HAM(n,@A,@B) - withdrawn
		v=0;
		break;

	default:
		return -1;
	}
	*res=v;
	return NUMBER;
}


int str_expr(char *str)
{
	char *p=str;
	if (*str == '\0') return 0;

	while (*str != '"') str++;
	return str-p;
}


int put_special(int var, int n)
{
	if (var>= 30)
	{
		set_bit((var-30)/10, (var % 10), n);
	}
	return 0;
}

int sigmoid(int v, int t)
{

	switch (t)
	{
	case 0: return v; // no change
	case 1: return (v>0)?10:0; //delta
	case 2: return (v>0)?10:-10; //delta
	case 3: return (v>0)?1:0; //delta
	case 4: // 1/(1-e^-x) actually 256/(1-e^(x/4))
		if(v<-20) v=-20;
		if(v>19)  v=19;
		v=(smap[v+20]-127)/4; //commented out for now
		return v;
	}
	return 0;
}

int get_bit(int pn, int bn)
{
	int n;
	switch(pn)
	{
	case 0:
		n = PINA;
		break;
	case 1:
		n = PINB;
		break;
	case 2:
		n = PINC;
		break;
	case 3:
		n = PIND;
		break;
	case 4:
		n = PINE;
		break;
	case 5:
		n = PINF;
		break;
	case 6:
		n = PING;
		break;
	default:
		return 0;
	}
	
	if (bn<8)
	{
		// mask result with bit
		int mask = 1<< bn;
		n &= mask;
	}
	return n;
}



void set_bit(int p, int b, int n)
{
	//rprintf ("Debug - set to port:%d:%d = %d\r\n", p ,b, n);
	
	volatile BYTE *port;
	BYTE mask;
	
	if (b<0 || b>8) return;
		
	switch(p)
	{
	case 0:
		port=&PORTA;
		break;
	case 1:
		port=&PORTB;
		break;
	case 2:
		port=&PORTC;
		break;
	case 3:
		port=&PORTD;
		break;
	case 4:
		port=&PORTE;
		break;
	case 5:
		port=&PORTF;
		break;
	case 6:
		port=&PORTG;
		break;
	default:
		rprintf ("panic error\r\n");
		return;
	}
	
	if (b==8) // set DDR
	{
		port -= 1; // now points to DDR
		*port = n;
		return;
	}
	
	mask = (1<<b);
	
	if (n==0)	
	{	
		*port &= ~mask; //clear bit
	}
	else
	{
		*port |= mask;  //set bit
	}
}



