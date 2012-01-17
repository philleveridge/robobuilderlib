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


const unsigned char smap[40] = {
	1,  2,  3,  4,  5,  6,  8,  10, 12, 15, 19, 24, 31, 38, 47, 57, 69, 82, 97, 112,
	128,144,159,174,187,199,209,218,225,232,237,241,244,246,248,250,251,252,253,254};


const  prog_char  *specials[] = {
	    "MIC",   "X",    "Y",    "Z",    "PSD", 
		"VOLT",  "IR",   "KBD",  "RND",  "SERVO", 
		"TICK",  "PORT", "ROM",  "TYPE", "ABS", 
		"MAPIR", "KIR",  "FIND", "CVB2I","NE", 
		"NS",    "MAX",  "SUM",  "MIN",  "NORM", 
		"SQRT",  "SIN",  "COS",  "IMAX", "HAM", 
		"RANGE", "SIG",  "DSIG",  "STAND", "ZEROS"
};


int getArg(char **str, long *res)
{
	if (**str=='(') 
	{
		(*str)++;
		eval_expr(str, res);
		if (**str==')')
		{
			(*str)++;
			return 1;
		}

	}
	return 0;
}

int get_special(char **str, long *res)
{
	char *p=*str;
	int t=token_match(specials, str, NOSPECS);
	long v=0;
	int rt=NUMBER;
	
	switch(t) {
	case sMIC:
		{
			int lc;

			for (lc=0; lc<SDATASZ; lc++) 
			{
				v += sData[lc];  // sum the buffer
				sData[lc]=0;     // and clear
			}
			MIC_SAMPLING=1; // on by default, but make sure
		}
		break;
	case sTICK:
		v=gtick;
		break;	
	case sMAX:
	case sIMAX:
		{
			long st=0;
			int k=0,m;

			if (**str!='(')
				break;

			(*str)++;

			if (eval_expr(str, res)==ARRAY)
			{
				if (**str==',')
				{
					(*str)++;
					if (eval_expr(str, &st)!=NUMBER)
						break;
				}
				if (**str!=')')
					break;
				(*str)++;
			}
			m=scene[st];
			for (v=st; v<nis; v++)
			{
				if (scene[v]>m) { m=scene[v]; k=v;}
			}
			v=(t==sMAX)?m:k;
		}
		break;	
	case sMIN:
		if (getArg(str,&v))
		{
			int m=scene[0];
			for (v=0; v<nis; v++)
			{
				if (scene[v]<m) m=scene[v];
			}
			v=m;
		}
		break;
	case sSUM:
		if (getArg(str,&v))
		{
			int m=0;
			for (v=0; v<nis; v++)
			{
				m += scene[v];
			}
			v=m;
		}
		break;
	case sHAM:
		// TBD - calculate HAMMING distance between 2 arrays
		// $HAM(n,@A,@B)
		//eg PRINT $HAM(0,@{3,1,2,3},@{3,3,2,1}
		if (**str=='(') 
		{
			(*str)++;
			eval_expr(str, res); 
			if (**str==',')
			{
				int n=*res;
				(*str)++;
				if (eval_expr(str, res)==ARRAY)
				{
					if (**str==',')
					{
						int i,tnis=nis;
						int tempB[SCENESZ];
						(*str)++;
						for (i=0;i<SCENESZ; i++)
						{
							tempB[i]=scene[i];
						}
						if (eval_expr(str, res)==ARRAY)
						{
							int m=(nis>tnis)?nis:tnis;
							for (i=0;i<m; i++)
							{
								v += ((abs(((i<tnis)?tempB[i]:0) - ((i<nis)?scene[i]:0))<=n)?1:0); 
							}
						}	
						(*str)++;  // ')'
					}
				}
			}
		}
		break;
	case sNORM:
		if (getArg(str,&v))
		{
			long m=0;
			for (v=0; v<nis; v++)
			{
				m += (scene[v]*scene[v]);
			}
			v=Sqrt(m);
		}
		break;
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
	case sRND: // $RND 0 < n < 255 or $RND(6,0,5) ->{6,n,n,n,n,n,n} 0<n<5
		if (**str=='(')
		{
			long i,a=0,b=0,c=0;
			(*str)++;
			eval_expr(str, &a);
			if (**str==',')
			{
				(*str)++;
				eval_expr(str, &b);
				if (**str==',')
				{
					(*str)++;
					eval_expr(str, &c);
				}
			}
			(*str)++;  // ')'

			if (b>c) swap(&b,&c);

			for (i=0; i<a; i++)
			{
				scene[i]=(rand()%(c-b))+b;
			}
			nis=a;
			rt=ARRAY;
		}
		else
		   v=rand();
		break;
	case sIR2ACT: //$IR2ACT(10) -> x
		v=0;
		if (getArg(str,&v))
		{
			v=map[v];
		}
		break;
	case sDSIG: // $DISG(x) -> x*(1-x)
		v=0;
		if (getArg(str,&v))
		{
			v = v*(1-v);
		}
		break;
	case sSIG: //$SIG(n[,t])
		v=0;
		if (getArg(str,&v))
		{
			v=sigmoid(v,2);
		}
		else
		{
			if (**str==',')
			{
				long r;
				(*str)++;
				eval_expr(str, &r);
				v=sigmoid(v,r);
				(*str)++;
			}
		}
		break;
	case sSTAND: // $STAND(x)
		v=0;
		if (getArg(str,&v))
		{
			int i;
			if (v<1)  v=1;
			if (v>18) v=18;
			for (i=0; i<v; i++)
			{
				if (v<=16)
					scene[i]=basic16[i];
				else
					scene[i]=basic18[i];
			}
			nis=i;
			rt=ARRAY;
		}
		break;
	case sZEROS: // $ZEROS(x)
		v=0;
		if (getArg(str,&v))
		{
			int i;
			for (i=0; i<v; i++)
			{
				scene[i]=0;
			}
			nis=i;
			rt=ARRAY;
		}
		break;
	case sABS: // $ABS(x)
		v=0; 
		if (getArg(str,&v))
		{
			v = v<0?-v:v;
		}
		break;
	case sSQRT: // $SQRT(x)
		v=0; 
		if (getArg(str,&v))
		{
			v = Sqrt(v);
		}
		break;
	case sSIN: // $SIN(x)
		v=0; 
		if (getArg(str,&v))
		{
			v = Sin(v%256);
		}
		break;
	case sCOS: // $COS(x)
		v=0; 
		if (getArg(str,&v))
		{
			v = Cos(v%256);
		}
		break;
	case sCVB2I: // $CVB2I(x)  255 -> -1
		v=0; 
		if (getArg(str,&v))
		{
			v = cbyte(v);
		}
		break;
	case sROM: // $ROM(x) or $ROM$(X) or $ROM@(x)
                if (**str=='$')
                {
			(*str)++;
			if (getArg(str,&v))
			{
				v = eeprom_read_byte((BYTE*)(PERSIST+v));
			}
		}
		else
                if (**str=='@')
                {
			(*str)++;
			if (getArg(str,&v))
			{
				int i;
				nis = v;
				for (i=0;i<nis;i++)
					scene[i] = eeprom_read_byte((BYTE*)(PERSIST+i));
			}
		}
		else
                {
			if (getArg(str,&v))
			{
				v = eeprom_read_byte((BYTE*)(FIRMWARE+v));
			}
		}
		break;
	case sSERVO: // SERVO(nn)
		//$servo
		if (**str!='(') 
		{
			v=readservos(0);
		}
		else
		// get position of servo id=nn
		if (getArg(str,&v))
		{
			v = wckPosRead(v);
		}
		break;
	case sRANGE:
		if (**str=='(') 
		{
			long a,b,c;
			(*str)++;
			eval_expr(str, &a); 
			if (**str==',')
			{
				(*str)++;
				eval_expr(str, &b); 
				if (**str==',')
				{
					(*str)++;
					eval_expr(str, &c); 
				}
				if (**str!=')')
				{
					BasicErr=1;
					break;
				}
				(*str)++;
				if (a<b) v=b;
				else
					if (a>c) v=c;
					else 
						v=a;
			}
		}
		break;
	case sFIND:
		//$FIND(x,@A)
		if (**str=='(') 
		{
			(*str)++;
			eval_expr(str, res); 
			if (**str==',')
			{
				int n=*res;
				(*str)++;
				if (eval_expr(str, res)==ARRAY)
				{
					int i;
					v=0;
					for (i=0; i<nis; i++)
					{
						if (scene[i]==n)
						{
							v=i; break;
						}
						if (scene[i]>n)
						{
							if (i>0) v=i-1; 
							break;
						}
					}
					if (i==nis) v=nis-1; // no match
					if (**str==')')
					{
						(*str)++;
					}
				}
			}
		}
		break;		
	case sPORT: // PORT:A:n
		// get position of servo id=nn
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
		return (*str-p); // not finished yet
	default:
		return -1;
	}
	*res=v;
	//t=strlen(specials[t]);
	return rt;
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
		v=(smap[v+20]-127)/4;
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



