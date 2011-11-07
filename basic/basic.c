/*
An elementry basic language for Robobuilder Humanoid Robot

By l3v3rz 
c 2010

Language Spec:
See wiki for details

http://code.google.com/p/robobuilderlib/wiki/Basic

$Revision$
*/

#ifdef AVR
#include <avr/io.h>
#include <avr/eeprom.h> 
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "rprintf.h"
#include "math.h"
#include "wckmotion.h"

// Standard Input/Output functions

#include "main.h"
#include "global.h"
#include "macro.h"

#include "adc.h"
#include "ir.h"
#include "accelerometer.h"

#define Sqrt sqrt
#endif

#ifdef WIN
#include "win.h"
#endif

#ifdef LINUX
#include "linux.h"
#endif



#include "edit.h"

#define MAX_LINE  		130 
#define MAX_TOKEN 		8
#define MAX_FOR_NEST	3
#define MAX_GOSUB_NEST	2
#define MAX_FOR_EXPR	20 
#define MAX_DEPTH 		6

/***********************************/

BYTE EEMEM FIRMWARE        	[64];  		// leave blank - used by Robobuilder OS
BYTE EEMEM BASIC_PROG_SPACE	[EEPROM_MEM_SZ];  // this is where the tokenised code will be stored
BYTE EEMEM PERSIST			[256];                     // persistent data store

extern void Perform_Action	(BYTE action);
extern int	getHex			(int d);
extern int	delay_ms		(int d);
extern void SampleMotion	(unsigned char); 
extern void sound_init		();
extern void SendToSoundIC	(BYTE cmd) ;

/***********************************/

extern int Cos(BYTE d);
extern int Sin(BYTE d);
extern int fix_fft(short fr[], short fi[], short m, short inverse);

extern int chargemode();

/***********************************/

int   fix_fftr(int f[], int m, int inverse);
void  lights(int n); 
extern BYTE cpos[];
extern WORD send_hex_str(char *bus_str, int n);
extern WORD send_hex_array(int *p, int n);

extern BYTE				sData[];
extern int 				sDcnt;
extern void				sample_sound(int);
extern BYTE				nos;
extern volatile BYTE	MIC_SAMPLING;
extern volatile BYTE    MIC_LEVEL;
extern volatile WORD    MIC_DLY;
extern volatile BYTE    MIC_STOP;
extern volatile BYTE    MIC_RATE;
extern volatile BYTE    MIC_NOS;
extern volatile WORD	gtick;

const prog_char *tokens[] ={
	"LET",   "FOR",    "IF",     "THEN", 
	"ELSE",  "GOTO",   "PRINT",  "GET",
	"PUT",   "END",    "LIST",   "XACT",
	"WAIT",  "NEXT",   "SERVO",  "MOVE",
	"GOSUB", "RETURN", "POKE",   "STAND",
	"PLAY",  "OUT",    "OFFSET", "RUN",
	"I2CO",  "I2CI",   "STEP",   "SPEED", 
	"MTYPE", "LIGHTS", "SORT",   "FFT",
	"SAMPLE","SCALE",  "DATA",
	"SET", 	"INSERT", "DELETE",
	"GEN",  "NETWORK", "SELECT"
};

const  prog_char  *specials[] = {
	    "MIC",   "X",    "Y",    "Z",    "PSD", 
		"VOLT",  "IR",   "KBD",  "RND",  "SERVO", 
		"TICK",  "PORT", "ROM",  "TYPE", "ABS", 
		"MAPIR", "KIR",  "FIND", "CVB2I","NE", 
		"NS",    "MAX",  "SUM",  "MIN",  "NORM", 
		"SQRT",  "SIN",  "COS",  "IMAX", "HAM", 
		"RANGE", "SIG",  "DSIG",  "STAND", "ZEROS"
};


//wait for byte or IR press
int  GetByte()
{
	int b;
	
	RUN_LED1_OFF;
	while (1) 
	{
		if ((b=uartGetByte())>=0) break;
		if ((b=irGetByte())>0)   break;
	}
	RUN_LED1_ON;
	
	return b;
}


const  prog_char *error_msgs[] = {
	"",
	"Syntax error",
	"Invalid Command",
	"Illegal var",
	"Bad number",
	"Bad Line",
	"Bad List"
	};

int errno;
int fmflg;
static int dbg=0;

void PerformAction (int Action)
{	
	if (Action>=0 && Action <=18)
	{
	/*
		0x00:  //PunchLeft
		0x01:  //PunchRight
		0x02:  //SidewalkLeft
		0x03:  //SidewalkRight
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
	*/
		rprintf("Play Motion %d\r\n", Action);
		PlayMotion(Action);
	}
	else
	{
		rprintf("Motion range:  0 - 18\r\n", Action);
	}
}

//next no space char
char getNext(char **p_line) 
{
	char c1=**p_line;
	
	while (c1 == ' ' )
	{
		(*p_line)++;
		c1=**p_line;
	}
	if (c1 != '\0') (*p_line)++;

	return c1;
}

// Read variable - 
// Simple def - must be A-Z
// More complex later
int getVar(char **p_line) 
{
	char c1=getNext(p_line);
	
	if (c1>='A' && c1<= 'Z' )
	{
		return c1-'A';
	}
	else
	return -1;
}

// Read number  - 
// Simple def - must be 0-9
// More complex later (%/& etc)
int getNum(char **p_line) 
{
	int num=0;
	char c1=getNext(p_line);
	
	while (c1>='0' && c1<= '9' )
	{
		num = num*10 + c1-'0';
		c1=**p_line;
		(*p_line)++;	
	}
	(*p_line)--;	
	return num;
}

//simple token match
//i points to next char after toek
//returns token ID or -1 if not found

int getToken(char *str, char *tok)
{
	int n=0;
	char c;
	while ((c=*str))
	{
		str++;
		if (!((c>= 'A' && c <='Z') || (c>= '0' && c <='9')) )  //must be alpha
			break;
		*tok++=c; //copy in dest buff
		n++;
	}	
	*tok='\0';
	return n;
}

int token_match(char *list[], char **p_line, int n)
{
	int t;
	int t1;
	char buff[MAX_TOKEN];
	
	t1=getToken(*p_line, buff);  // this could buffer over run - needs fixing
	
	for (t=0; t<n; t++)
	{
		if (!strcmp(buff, list[t]))
			break;
	}
	
	if (t==n)
	{
		return -1; 		// no_match : match
	}
	else {
		*p_line += t1; 
		return t;
	} 
}

int readLine(char *line)
{
	int ch=0,pch;
	int qf=1;
	int lf=0;
	char *start=line;
	
	rprintfStr ("> ");
	
	while (1)
	{
		// foreach char entered
		pch=ch;

		while ((ch = uartGetByte())<0) ;

		if (lf==1)
		{
			rprintfChar(ch);
			if (ch>=100) { *line++ = (ch/100) +'0'; }
			ch=ch%100;
			if (ch>=10) {  *line++ = (ch/10) +'0'; }
			*line++ = (ch%10) +'0';
			lf=0;
			continue;
		}

		if (ch=='`') {lf=1;rprintfChar(ch);continue;}

		if (ch==9) ch=' ';
				
		if (ch==13 || (start==line && ch=='.') )
		{
			if (ch=='.') *line++='.';
			if (pch==' ') line--; //get rid of trailling space
			*line='\0'; 
			rprintf("\r\n");	
			break;
		}

		if (qf && ch==' ' && pch==' ')
			continue;
		
		if (start==line && ch=='?' )
		{
			rprintf("Program entry mode (. to exit)\r\n");	
			break;
		}
		
		if (ch=='"') {qf=!qf;}

			
		if (ch >= 'a' && ch <= 'z' && qf) ch = ch-'a'+'A';  // Uppercase only
		
		if (ch==8 || ch==127) //Bsapce ?
		{
			if (line>start) 
			{
				line--;
				rprintfChar(ch);
			}
		}
		else
		{
			if (line-start<MAX_LINE-2)
			{
				rprintfChar(ch);	
				*line++ = ch;
			}
		}
	}
	return (line-start);
}

// Repeat
//   Read a line
//   If a '.' stop
//   Convert into tokens
//   Store in Eprom
// Loop

int execute(line_t line, int f);
int fnptr[26];
extern WORD psize;

void basic_load(int tf)
{	
	char line[MAX_LINE];
	int n=0;
	int lc=0;
	char *cp;
					
	struct basic_line newline;	
	
	errno=0;

	rprintfStr("Enter Program '.' to Finish\r\n");
	
	while (1)
	{
		// for each line entered

		if (errno > 0 && errno<6) {
			rprintfStr ("Error - '" );
			rprintfStr (error_msgs[errno]);
			rprintfStr ("'\r\n" );
			rprintf ("Pos=%d [%c]\r\n", (cp-line), line[cp-line]);
			errno=0;
		}
		else if (errno!=0)
		{
			rprintf ("Panic %d\r\n", errno);
			return;
		}

		cp=&line[0];				
		n=readLine(cp);
		
		if ( line[0] =='.' && n==1)
		{
			rprintf("\r\n%d lines entered, [%d/%d] Bytes\r\n", lc, psize, EEPROM_MEM_SZ);
			return;
		}
		
		if (n==0) continue; //ignore blank lines
		
		// convert to token
	
		newline.var=0;
		newline.value=0;
		newline.text=0;
			
		if ((newline.lineno = getNum(&cp))!=0)
		{	
			if (cp>line+n || *cp  !=' ')
			{
				printf ("Delete - %d\r\n", newline.lineno);
				deleteln(newline.lineno);
				continue;
			}
			cp++;
		}

		while (*cp == ' ') cp++;

		if ( (newline.token=token_match((char **)tokens, &cp, NOTOKENS))==255)           // sizeof(tokens)/))<0)
		{
			errno=2;
			continue;
		}
			
		while (*cp == ' ') cp++;
		
		switch (newline.token)
		{
		case LET: 
		case FOR:
			// read Variable		
			if ((newline.var = getVar(&cp))<0)
			{
				errno=3;
			}
			// '='
			if (getNext(&cp) != '=')
			{
				errno=1;
			}
			// expression
			if (newline.token==FOR)
			{
				char *to = strstr(cp," TO ");
				if (to==0) errno=1;
			}
			//else
				newline.text=cp;
			break;
		case POKE: 
		    //poke address, byte
			newline.var = getNum(&cp);
			if (getNext(&cp) != ',')
			{
				errno=1;
			}			
			newline.text=cp;
			break;
		case PUT: 
			if ((newline.var = token_match(specials, &cp, sizeof(specials)))<0)
			{
				errno=3;
			}
			
			if (newline.var == sPORT)
			{
				char pn, pb;
				if (getNext(&cp) != ':')
				{
					errno=3;
				}	

				pn = getNext(&cp) ;
				
				if (pn<'A' || pn >'G') 
				{
					errno=3;
				}
				
				if (getNext(&cp) != ':')
				{
					errno=3;
				}	
				
				pb = getNext(&cp) ;
				
				if ((pb<'0' || pb >'8'))
				{
					errno=3;
				}
				
				//Now have PORT:$pn:$pb
				
				newline.var = 30 + (pn-'A')*10 + (pb-'0') ;
			}
			else
			{
				errno=1;
			}
			// '='
			if (getNext(&cp) != '=')
			{
				errno=1;
			}					
			newline.text=cp;
			break;

		case SERVO:
		case STEP:
			// read servo ID	
			if ((newline.var = getVar(&cp))==255)
			{
				cp--;
				newline.var = getNum(&cp);
				if (newline.var<0 || newline.var>31)
				{
					errno=3;
					break;
				}
				newline.var += 32;
			}
			// '='
			if (getNext(&cp) != '=')
			{
				errno=1;
			}			
			newline.text=cp;
			break;
		case PRINT:
		case SORT:
		    if (*cp=='#') { newline.var=1; cp++;}
		    if (*cp=='%') { newline.var=2; cp++;}
			newline.text=cp;
			break;
		case MOVE:	
		case OUT:	
		case RUN:
		case XACT: 
		case OFFSET:
		case I2CI:
		case I2CO:
		case SPEED:
		case MTYPE:
		case LIGHTS:
		case FFT:
		case STAND: 
		case SCALE:
		case SAMPLE: 
		case SET:
		case INSERT:
		case DELETE:
		case WAIT: 
		case GET:
		case GEN:
		case NETWORK:
		case SELECT:
			newline.text=cp;
			break;
		case LIST:
		case DATA:
			// read Variable
			if (*cp=='!')
			{
				newline.var=32;
				cp++;
			}
			else if ((newline.var = getVar(&cp))<0)
			{
				errno=3;
			}
			// '='
			if (getNext(&cp) != '=')
			{
				errno=1;
			}
			if (newline.token==LIST)
			{
				newline.text=cp;
			}
			else
			{
				//read each number and store in text
				int nob=0; // FF nb b1 b2 ... bn 
				int i;
				BYTE tbuff[64];
				char *p=&line[0];

				if (*cp=='"')
				{
					//i.e. DATA A="abcde"
					nob=0;
					cp++;
					while (*cp != '"' && (nob<63))
					{
						tbuff[nob++] = *cp++;
					}
				}
				else
				{
					while (1)
					{
						int b = getNum(&cp);
						tbuff[nob++] = b;
						if (getNext(&cp) != ',')
							break;
					}
				}
				*p++= 0xff;
				*p++=nob;
				for (i=0; i<nob; i++)
				{
					*p++ = tbuff[i];
				}
				*p++=0;
				newline.text=&line[0];
			}
			break;
/*
		case IF:
			// We should check for THEN and ELSE
			// and handle error
			// replace THEN with ? and ELSE with :
			// IF A THEN B ELSE C =>  GOTO (A)?B:C
			{
			char *p_then,*p_else;
			newline.text=cp-1;
			*(cp-1)='(';
			p_then = strstr(cp, "THEN");
			p_else = strstr(cp, "ELSE");
			if (p_then != 0) 
				strncpy(p_then, ")?  ",4);
			else
				errno=3;
				
			if (p_else != 0) 
				strncpy(p_else,":   ",4);
			else
				strcat(cp,":0");
			}
			break;
*/
		case IF:
			// A THEN B ELSE C =>  A,B[,C]
			{
				char *p_then,*p_else,*cp2;
				newline.text=cp-1;
				p_then = strstr(cp, " THEN");

				if (p_then != 0)
				{
					*p_then++=',';

					while(1) {
					   *p_then=*(p_then+5);
					   p_then++;
					   if (*p_then==0) break;
					}
				}
				else
					errno=3;

				p_else = strstr(cp, " ELSE");

				if (p_else != 0)
				{
					*p_else++=',';

					while(1) {
					   *p_else=*(p_else+5);
					   p_else++;
					   if (*p_else==0) break;
					}
				}
				else
					strcat(cp,",0");
			}
			break;
		case PLAY: 
		case GOTO: 
		case GOSUB:
			newline.value = getNum(&cp);	// read line	
			break;
		case END:
		case RETURN:
			newline.text=0;
			break;
		case NEXT:
			if ((newline.var = getVar(&cp))<0)
			{
				errno=3;
			}
			break;
		default:
			errno=2;
			break;
		}			
		
		if (errno==0)
		{	
			if (newline.lineno==0)
			{
				if (newline.token==LIST || newline.token==DATA
						|| newline.token==FOR || newline.token==GOTO
						|| newline.token==NEXT || newline.token==RETURN
						|| newline.token==IF || newline.token==GOSUB)
				{
					rprintf("Command not available in immediate mode\r\n");
				}
				fmflg=0;
				if (tf==1)
					gtick=0;
				execute(newline,0); // immediate execute
				if (tf==1)
					rprintf("Elapsed Time %dms\r\n", gtick);
			}
			else
			{
				lc++;
				insertln(newline); 	// store struct in eeprom
			}
		}
	}
}

/*************************************************************************************************************

RUNTIME routines

*************************************************************************************************************/
#define SCENESZ 128

int variable[26]; // only A-Z at moment
int scene[SCENESZ];	  // generic array
int nis=0;        // number of item isn array
int  eval_list(char *p);
unsigned char eval_expr(char **str, int *res);
enum {STRING, NUMBER, ARRAY, ERROR, CONDITION } ;
int speed=2;
int mtype=2;

int cs=0;
int lnc[20];

int findcache(int l)
{
	int i=0;
	while (i<cs)
	{
		if (lnc[i]==l)
		{
			return lnc[i+1];
		}
		i+=2;
	}
	return 0;
}

void linenocache(int l, int m)
{
	if (findcache(l)==0) // hit?
	{
		//add to cache
		if (cs<18)
		{
			lnc[cs]=l;
			lnc[cs+1]=m;
			cs +=2;
		}
	}
}

void swap(int *x,int *y)
{
   int temp;
   temp = *x;
   *x = *y;
   *y = temp;
}
 
int choose_pivot(int i,int j )
{
   return((i+j) /2);
}
 
void quicksort(int list[],int m,int n)
{
   int key,i,j,k;
   if( m < n)
   {
      k = choose_pivot(m,n);
      swap(&list[m],&list[k]);
      key = list[m];
      i = m+1;
      j = n;
      while(i <= j)
      {
         while((i <= n) && (list[i] <= key))
                i++;
         while((j >= m) && (list[j] > key))
                j--;
         if( i < j)
                swap(&list[i],&list[j]);
      }
      // swap two elements
      swap(&list[m],&list[j]);
      // recursively sort the lesser list
      quicksort(list,m,j-1);
      quicksort(list,j+1,n);
   }
}

const unsigned char smap[40] = {
	1,  2,  3,  4,  5,  6,  8,  10, 12, 15, 19, 24, 31, 38, 47, 57, 69, 82, 97, 112,
	128,144,159,174,187,199,209,218,225,232,237,241,244,246,248,250,251,252,253,254};

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

int getArg(char **str, int *res)
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

const unsigned char map[] = {0, 7, 6, 4, 8, 5, 2, 17, 3, 0, 9, 1, 10, 11, 12, 13, 14, 15, 16, 18, 19};

int get_special(char **str, int *res)
{
	char *p=*str;
	int t=token_match(specials, str, NOSPECS);
	int v=0;
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
			int st=0;
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
			int m=0;
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
			int i,a=0,b=0,c=0;
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
				int r;
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
	case sSIN: // $SQRT(x)
		v=0; 
		if (getArg(str,&v))
		{
			v = Sin(v%256);
		}
		break;
	case sCOS: // $SQRT(x)
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
			int a,b,c;
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
					errno=1;
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


int math(int n1, int n2, char op)
{
	switch (op) {
	case '+': 
		n1=n1+n2; break;
	case '-':
		n1=n1-n2; break;
	case '*':
		n1=n2*n1; break;
	case '/':
		n1=n1/n2; break;
	case '&':
		n1=n2 && n1; break;
	case '|':
		n1=n1 || n2; break;
	case '%':
		n1=n1%n2; break;
	case '>':
		n1=(n1>n2)?1:0; break;		
	case '<':
		n1=(n1<n2)?1:0; break;	
	case 'l':
		n1=(n1<=n2)?1:0; break;	
	case 'g':
		n1=(n1>=n2)?1:0; break;	
	case 'n':
		n1=(n1!=n2)?1:0; break;	
	case '=':
		n1=(n2==n1)?1:0; break;		
	}
	return n1;
}

int str_expr(char *str)
{
	char *p=str;
	if (*str == '\0') return 0;

	while (*str != '"') str++;
	return str-p;
}

unsigned char eval_expr(char **str, int *res)
{
	char c;
	
	int n1=0;
	int stack[MAX_DEPTH]; 
	char ops[MAX_DEPTH];
	
	int sp=0;
	int op=0;
	int tmp=0;
	int done=0;
	
	while (**str != '\0' && !done)
	{
		if ((c = **str)==')')
			break;
		
		(*str)++;

		if (c>='0' && c<='9')
		{
			n1 = n1*10 + c - '0';
		}
		else
		if (c>='A' && c<='Z')
		{
			n1 = variable[c-'A'];
		}
		else
		switch (c)
		{
		case '(':
			eval_expr(str, &tmp);
			n1 = tmp;
			(*str)++;
			break;
		case '?' :
		case '+' :
		case '-' :
		case '*' :
		case '/' :
		case '>' :
		case '<' :
		case '=' :
		case '&' :
		case '|' :
		case '%' :
		case ':' :
			if (c=='>' && **str=='=') {c='g'; (*str)++;}
			if (c=='<' && **str=='=') {c='l'; (*str)++;}
			if (c=='<' && **str=='>') {c='n'; (*str)++;}
			ops[op++]=c;
			stack[sp++]=n1;
			n1=0;
			break;
		case '"':
			return STRING;
		case ' ':
			break; //ignore sp
		case '@':
			if (**str=='#')
            {
				int i;
				(*str)++;
				for (i=0;i<16; i++)
				{
					scene[i] = offset[i];
				}
				nis=16;
			}
			else
            if (**str=='{')
            {
                // literal
                int cnt;
                (*str)++;
                cnt=eval_list(*str);
                *str = *str+cnt;
                if (**str!='}')
                {
					break;
                }
				(*str)++;
            }
			else
            if (**str=='!')
            {
				//use current array
				(*str)++;
			}
			else
            if (**str=='<')
            {
				//use sound array
				for (n1=0; (n1<MIC_NOS && n1<SDATASZ && n1<SCENESZ); n1++) 
				{
					scene[n1] = sData[n1];     // and clear
				}
				nis=MIC_NOS;
				(*str)++;
			}
			else
            if (**str=='?')
            {
				//use servo pos array
				readservos(nos);

				for (n1=0; n1<nos; n1++) 
				{
					scene[n1] = cpos[n1];     // and clear
				}
				nis=nos;
				(*str)++;
			}
            else
            {
				char tmpA[128];
				n1 = **str-'A';
				if (n1<0 || n1 >25)
				{
					break;
				}
				n1 = variable[n1];
				readtext(n1, tmpA);				
				if (tmpA[0]==0xFF) // DATA
				{
					int i=0;
					nis=tmpA[1];
					for (i=0; i<nis; i++)
						scene[i]=(unsigned char)tmpA[i+2];
				}
				else
					eval_list(tmpA);
				//
				(*str)++;
			}
			if (**str == '[')
			{
				int ind;
				(*str)++;
				eval_expr(str, &ind);
				if (ind>=nis) ind=nis-1;
				if (ind<0)    ind=0;

				if (**str == ',')
				{
					//sub list
					int ct;
					(*str)++;
					eval_expr(str, &ct);
					if (ct>=nis || ct==0) ct=nis-1;
					if (ct>0 && tmp>=0 && ct>tmp)
					{
						int i=0;
						for (i=tmp; i<=ct; i++)
						{
							scene[i-tmp]=scene[i];
						}
						nis=ct-tmp+1;
					}
					(*str)++;
				}
				else
				{
					n1 = scene[ind];
					(*str)++;
					break;
				}
			}
			if (**str == '+' || **str == '-' || **str == '.')
			{
				//add array
				int i,tnis;
				int tempB[SCENESZ];
				char o = **str;
				(*str)++;
				for (i=0;i<SCENESZ; i++)
				{
					tempB[i]=scene[i];
				}
				tnis=nis;
				if (eval_expr(str,res)==ARRAY)
				{
					if (o=='.')
					{
						if (tnis+nis<SCENESZ) 
						{
							int m=(nis>tnis)?nis:tnis;
							for (i=m-1;i>=0; i--)
							{
								if (i<nis)
									scene[i+tnis] = scene[i];
								if (i<tnis)
									scene[i]     = tempB[i];
							}
							nis=nis+tnis;
						}
					}
					else
					{
						int m=(nis>tnis)?nis:tnis;
						for (i=0;i<m; i++)
						{
							if (o == '+') 
								scene[i] = ((i<tnis)?tempB[i]:0) + ((i<nis)?scene[i]:0); 
							else 
								scene[i] = ((i<tnis)?tempB[i]:0) - ((i<nis)?scene[i]:0); 
						}
						nis=m;
					}
				}
			}

			return ARRAY;
			break;
		case '$':
			//special var?
			if (get_special(str, &n1)==ARRAY)
				return ARRAY;
			break;
		default:
			done=1;
			(*str)--;
		}
	}

	stack[sp++] = n1;

	while (op>0) {
		if (ops[op-1]==':')
		{
			if (op > 1 && ops[op - 2] == '?' && sp>2)
			{
				if (stack[sp - 3] == 0)
					stack[sp - 3] = stack[sp - 1];
				else
					stack[sp - 3] = stack[sp - 2];
				op--;
				sp-=2;
			}
			else
			{
				rprintf("eval error %d, %d\r\n", op, sp);
				return ERROR; 
			}
		}
		else
		{
			stack[sp-2] = math(stack[sp-2],stack[sp-1],ops[op-1]);
			sp--;
		}
		op--;
	}

	*res = stack[0];
	return NUMBER;
}


int gotoln(int gl)
{
	int p;

	if ((p=findcache(gl))>0)
	{
		return p;
	}
	p = findln(gl);
	if (p<3 || getlineno(p) != gl ) return -1; // no such line
	linenocache(gl,p);
	return p;
}


int put_special(int var, int n)
{
	if (var>= 30)
	{
		set_bit((var-30)/10, (var % 10), n);
	}
	return 0;
}

int eval_list(char *p)
{
	// eval list "5,1,2,3,4,5" ->scene[5]
	int i,n;
	char *t=p;
	
	eval_expr(&p, &nis);
		
	if (*p++ != ',') { errno=6; return 0; }
	for (i=0;i<nis;i++)
	{
		n=0;
		eval_expr(&p, &n);
		if (i!=(nis-1) && *p++ != ',') { errno=6; return 0; }
		scene[i]=n;
	}
	return p-t;
}

int  forptr[MAX_FOR_NEST];   // Upto 3 nested for/next
char nxtptr[MAX_FOR_NEST][MAX_FOR_EXPR];   // Upto 3 nested for/next

int fp; 


int gosub[MAX_GOSUB_NEST];  // 3 nested gosubs
int gp;  
		
void basic_run(int dbf)
{
	fmflg=0;

	nxtline=0;

	if (nextchar()!=0xAA) {
		rprintfStr("No program loaded\r\n");
		return;
	}

	//point top at EEPROM
	rprintfStr("Run Program \r\n");

	firstline();

	basic_start(dbf);
}

void basic_start(int dbf)
{
	// Repeat
	//   load from memory
	//   Execute action
	// Loop

	BYTE tmp=0;
	line_t line;
	char buf[MAX_LINE];
	
	errno=0;

	fp=0; // Upto 3 nested for/next

	gp=0;  // 3 nested gosubs

	cs=0; // reset line number cache

	while (tmp != 0xCC && nxtline < EEPROM_MEM_SZ )
	{
		int tc;
		if (errno !=0)
		{
			rprintf("Runtime error %d on line %d\r\n", errno, line.lineno);
			return;
		}

		line = readln(buf);
		if (line.lineno==0)
		{
			rprintf("End\r\n");
			return;
		}
		
		tc = nextchar();	// terminator character ?
		
		if (dbf) {
			int kp;
			rprintf ("TRACE :: %d - ", line.lineno); 
			rprintfStr (tokens[line.token]);;
			while ((kp=uartGetByte())<0) ; // wait for input
			if (kp == 27)  
			{
				rprintfStr (" ** User Break\r\n"); 
				return;
			}
			rprintfCRLF();
		}
		
		if (uartGetByte() == 27)  {
			rprintf ("User Break on line %d\r\n", line.lineno); 
			return;
		}

		if (line.lineno==0) break;

		/* execute code */
		tmp = execute(line, dbf);
		
		if (dbf && (line.token==LET || line.token==GET || line.token==FOR || line.token==SERVO || line.token==NEXT ))
		{
				rprintf ("%c = %d\r\n", line.var+'A', variable[line.var]);
		}
		if (tmp == 0) tmp=tc;
	}
}

int execute(line_t line, int dbf)
{
	//   Get token
	//   Execute action
	//	 Move to next line
	BYTE tmp=0;
	char *p;			
	int n;

	switch (line.token)
	{
	case FOR: 
		{
			char *to;
			// remember where next instruction is
 			forptr[fp] = nxtline;
			// eval expr1 of line.text = "expr1 TO expr2"
			// i.e var=expr1
			n=0;
			p=line.text;

			to = strstr(p," TO ");
			if (to==0) 
				errno=1;
			else
			{
				strncpy(nxtptr[fp],to+4, MAX_FOR_EXPR);
				*to='\0'; // null terminate expr1
				if (eval_expr(&p, &n)==NUMBER)
					variable[line.var] = n;		
			}
			fp++;
		}
		break;
	case NEXT: 
		// increment var and check condition
		if (fp>0) {
			int t_ptr=forptr[fp-1];

			// increment var
			variable[line.var]++;
			// test against expr2 i.e var<=expr2
			n=0;
			p=nxtptr[fp-1];
			if (eval_expr(&p, &n)!=NUMBER)
			{
				errno=1;
				break; //need to handle error	
			}			
			if (variable[line.var] <= n) { 
				// if true set ptr=stack; 
				setline(t_ptr); tmp=1;
			}	
			else
			{
				fp--;
			}
		}
		break;
	case POKE: // POKE A,B  or POKE A,$B or POKE 0,@
                {
                        int f=1;
 			int i;
	 		n=0;
			p=line.text;
		        if (*p=='$')
			{
		            p++;
                            f=0;
			}
 			if (*p=='@')
			{
		            p++;
			    for (i=0; i<nis; i++)
				   eeprom_write_byte(PERSIST+i, scene[i]);      
			}
			if (eval_expr(&p, &n)==NUMBER)
			{
				// put result into address line.var
				BYTE addr=line.var;
                                if (f)
				   eeprom_write_byte(FIRMWARE+addr, n);
                                else
				   eeprom_write_byte(PERSIST+addr, n);                             
			}
			else
				errno=1;
                }
		break;
	case PUT: 
		n=0;
		p=line.text;
		if (eval_expr(&p, &n)==NUMBER)
			put_special(line.var, n);
		else
			errno=1;
		break;
	case LET: 
		n=0;
		p=line.text;
		if (eval_expr(&p, &n)==NUMBER)
			variable[line.var] = n;
		else
			errno=1;
		//else
		//handle error
		//rprintf ("assign %c= %d\r\n", line.var, n); // DEBUG
		break;
	case SERVO:
		{
			int v=line.var;
			if (v>=0 && v<=31)
				v=variable[v];
			else
				v=v-32;
			n=0;
			p=line.text;
			if (*p=='@') // set passive mode
			{
				// set pass servo id=line.var
				wckSetPassive(v);
			}
			else
			if (*p=='~') // set IO mode
			{
				p++;
				eval_expr(&p, &n);
				wckWriteIO(v, n) ;
			}
			else
			{
				eval_expr(&p, &n);
				if (n>00 && n<=254)
				{
					// set pos servo id=line.var, n
					// char SpeedLevel
					wckPosSend(v, speed, n);
				}
			}
		}
		break;
	case STAND: 
		if (eval_expr(&line.text, &n)==NUMBER)
			standup(n);
		else
			standup(nos);
		break;
	case GOTO: 
		{
			int t = gotoln(line.value);
			if (t<0)
			{
				errno=3; return 0xcc;	
			}		
			setline(t);
			tmp=1;
		}
		break;	
	case PRINT: 
		n=0;
		p=line.text;
		while (1)
		{
			if (*p=='\0') break; // done

			switch (eval_expr(&p, &n))
			{
			case NUMBER:
				rprintf ("%d", n);
				break;
			case STRING:
				n=str_expr(p);
				if (line.var==1)
				{
					WORD w=send_hex_str(p, n);
					scene[0]=w/256;
					scene[1]=w%256;
					nis=2;
				}
				else if (line.var==2)
				{
					send_bus_str(p, n);
				}
				else
				{
					rprintfStrLen(p,0,n);
				}
				p=p+n+1;
				break;
			case ARRAY:
				if (nis>0)
				{
					if (line.var==1)
					{
						// send to wxkbus
						WORD w=send_hex_array(scene,nis);
						scene[0]=w/256;
						scene[1]=w%256;
						nis=2;
					}
					else
					{
						int k,w=0;
						if (*p==':')
						{
							p++;
							if (eval_expr(&p, &w) != NUMBER)
							{
								errno=3; break;
							}
						}
						if (w<=0)  w=nis;
						if (w>nis) w=nis;
						for (k=0; k<=(nis/w); k++)
						{
							if (k*w<nis) rprintf ("%d", scene[(k*w)]);
							for (n=1; (n<w) && ((k*w+n)<nis); n++) {rprintf (",%d",scene[(k*w)+n]);}
							if (k*w<nis) rprintfCRLF();
						}
					}
				}
				else
					rprintfStr("Null");
			}

			if (*p=='\0') break; // done

			if (*p!=';' && *p!=',') {
				errno=4; //rprintf ("synatx prob= [%d]\r\n", *p); // DEBUG
				break;
			}
			if (*p==';' && *(p+1)=='\0') // last one
				break;
			p++;
		}
		// check for last ; (no crlf)
		if (*p!=';')	
			rprintfStr ("\r\n"); 			
		break;	
	case IF:		
		n=0;
		p=line.text;

		if (eval_expr(&p, &n)==NUMBER)
		{
			if (*p++ != ',')
			{
				errno=5; break;
			}

			if (n == 0)
			{	
				//false
				//skip to next comma
				while (*p!=0 && *p!=',') p++;
				p++;
			}

			if (eval_expr(&p, &n)==NUMBER && n!=0)
			{
				int t = gotoln(n);
				if (t<0)
					errno=5;
				else
					setline(t);
				tmp=1;
			}
		}
		break;		
	case MOVE: 
		// MOVE @A,20,2000
		// MOVE @A
		// No args - send servo positions synchronously
		// with args (No Frames / Time in Ms) - use MotionBuffer
		p=line.text;
			
		if (p!=0 && *p != 0)
		{	
			int j, fm=0, tm=0;
			BYTE pos[32];

			if (eval_expr(&p, &fm) != ARRAY)
			{
				errno=1;
				break;
			}
			if (*p=='\0')
			{
				if (nis>0) 
					wckSyncPosSend(nis-1, speed, scene, 0);
			}

			if (*p++ != ',') { errno=1; break;}
			eval_expr(&p, &fm);
			if (*p++ != ',') { errno=1; break;}
			eval_expr(&p, &tm);

			for (j=0; j<nis; j++)
			{
				if (j<16)
				{
					scene[j] += offset[j];
				}

				if (scene[j]>=0 && scene[j]<=254)
					pos[j] = scene[j];
				if (scene[j]<0)
					pos[j] = 0;
				if (scene[j]>254)
					pos[j] = 254;

				if (dbg) rprintf("DBG:: MOVE %d=%d\n", j, pos[j]);
			}

			if (!dbg) PlayPose(tm, fm, 4, pos, (fmflg==0)?nis:0);
			fmflg=1;
		}
		//
		break;
	case I2CO: 	//IC2O Addr,@{5,1,1,1,1}
	{
		BYTE ob[20];
		int i, addr=0;
		p=line.text;
		if (eval_expr(&p, &addr)==NUMBER)
		{
			if (*p++ != ',') { errno=1; break;}
			if (eval_expr(&p, &n) != ARRAY)
			{
				errno=1;
				break;
			}
			for (i=0; i<nis; i++)
			  ob[i]=scene[i];
			I2C_write(addr, nis, ob);
		}
		break;
	}
	case I2CI: 	//IC2I Addr,n[,@{}]
		{
		int i, addr=0, ibc=0;
		p=line.text;			
		if (eval_expr(&p, &addr)==NUMBER)
		{
			BYTE ob[20];
			BYTE ib[20];
			
			if  (*p++ != ',')
			{
				errno=1;
				break;	
			}
			if (eval_expr(&p, &ibc)==NUMBER)
			{
				if  (*p == ',')
				{
					if (eval_expr(&p, &n) != ARRAY)
					{
						errno=1;
						break;
					}
				}
				else
					nis=0;
			}	
			for (i=0; i<nis; i++)
			  ob[i]=scene[i];
			I2C_read (addr, nis, ob, ibc, ib);
			
			for (i=0; i<ibc; i++)
			  scene[i]=ib[i];					
		}
		}
		break;			
	case LIST: 
	case DATA: 
		if (line.var==32)
		{
			if (line.token==DATA) // DATA
			{
				int i=0;
				nis=line.text[1];
				for (i=0; i<nis; i++)
					scene[i]=(unsigned char)line.text[i+2];
			}
			else
				eval_list(line.text);
		}
		else
		{
			variable[line.var] = lastline+8;
		}
		break;		
	case XACT:
	case RUN:
		n=0;
		p=line.text;
		if (eval_expr(&p, &n) != NUMBER)
		{
			errno=1;
			break;
		}
		PerformAction(n);		
		break;
	case WAIT: 
		n=line.value;
		p=line.text;
		if (*p!=0 )
		{
			if (eval_expr(&p, &n) != NUMBER)
			{
				errno=1;
				break;
			}
		}
		delay_ms(n);
		break;
	case OFFSET: 
		{
			// OFFSET @A	load @A into offset
			// OFFSET #		load Basic18-Basic16 into offset
			// OFFSET		zero offset
			int i;
			p=line.text;
			if (p==0 || *p=='\0')
			{				
				for (i=0; i<16; i++)
				{
					offset[i]=0;
				}
			}
			else if (*p == '#')
			{
				for (i=0; i<16; i++)
				{
					offset[i]=(basic18[i]-basic16[i]);
				}
			}
			else if (eval_expr(&p, &n)==ARRAY)
			{
				for (i=0; i<16 && i<nis; i++)
				{
					offset[i]=scene[i];
				}
			}
			else
				errno=1;
		}
		break;
	case OUT: 
		{
		int l=1;
		n=0;
		p=line.text;
		switch (eval_expr(&p, &n))
		{
		case NUMBER:
			if (*p==',')
			{
				p++;
				eval_expr(&p, &l);
			}
			while (l-->0) rprintfChar(n);
			break;
		case ARRAY:
			for (l=0; l<nis; l++)
				rprintfChar(scene[l]);
			break;
		}
		}
		break;
	case STEP: 
		//STEP servo=from,to[,inc][,dlay]
		{
			int sf, st, si=5, sp, sn, sw=75, cnt=0, sd=8;
			int v=line.var;
			if (v>=0 && v<=31)
				v=variable[v];
			else
				v=v-32;
			n=0;
			p=line.text;
			if (eval_expr(&p, &sf)!=NUMBER)
			{
				errno=3; break;
			}
			if (*p++ != ',')
			{
				errno=2; break;
			}
			if (eval_expr(&p, &st)!=NUMBER)
			{
				errno=3; break;
			}
			if (*p==',')
			{
				p++;
				if (eval_expr(&p, &si)!=NUMBER)
				{
					errno=3; break;
				}
				sd=si+1;
				if (*p==',')
				{
					p++;
					if (eval_expr(&p, &sd)!=NUMBER)
					{
						errno=3; break;
					}
					if (*p==',')
					{
						p++;
						if (eval_expr(&p, &sw)!=NUMBER)
						{
							errno=3; break;
						}
					}
				}
			}

			sp = wckPosRead(v); // get servo current position
			
			//rprintf("STEP %d %d %d %d %d\n", sf, st, sp, si, sd, sw);

			if (sf < st) 
			{
				if (sp < sf) 
				{
					wckPosSend(v, speed, sf);
					sp=sf;
					delay_ms(sw);
				}
				sn=sp;
				while (cnt++<25 && (sp-sn)<sd && sn<st)
				{
					sp += si;
					if (sp>0 && sp<=254)
					{
						wckPosSend(v, speed, sp);
					}
					//sleep
					delay_ms(sw);
					sn = wckPosRead(v);
					//rprintf("-> %d %d %d %d\n", cnt, sp, sn, sd);
				}
			}
			else
			{
				if (sp > sf) 
				{
					wckPosSend(v, speed, sf);
					sp=sf;
					delay_ms(sw);
				}
				sn=sp;
				while (cnt++<25 && (sn-sp)<sd && sn>st)
				{
					sp -= si;
					if (sp>0 && sp<=254)
					{
						wckPosSend(v, speed, sp);
					}
					//sleep
					delay_ms(sw);
					sn = wckPosRead(v);
					//rprintf("<- %d %d %d %d\n", cnt, sp, sn, sd);
				}
			}
		}
		break;
	case PLAY: 
		SendToSoundIC(line.value);
		break;
	case LIGHTS:
		p=line.text;
		if (eval_expr(&p, &n)!=NUMBER)
		{
			errno=3; break;
		}
		if (*p==',')
		{
			//LIGHTS A,@{5,5,10,15,20,25}
			int tn;
			p++;
			if (eval_expr(&p, &tn)!=ARRAY)
			{
				errno=3; break;
			}
			MIC_SAMPLING=0;
			if (nis==5)
				blights(n, scene);
			else
				errno=3; // must be 5 elements
		}
		else
		{
			MIC_SAMPLING=0;
			lights(n);
		}
		break;
	case MTYPE:
		p=line.text;
		if (eval_expr(&p, &n)!=NUMBER)
		{
			errno=3; break;
		}
		if (n<0) PP_mtype=0;
		PP_mtype=n%4;
	case SPEED: 
		p=line.text;
		if (eval_expr(&p, &n)!=NUMBER)
		{
			errno=3; break;
		}
		if (speed<0) n=0;
		speed=n%4;
		break;
	case END: 
		rprintfStr ("End of program\r\n"); 
		return 0xCC;
	case GOSUB: 
		{
			int t;

			gosub[gp++]=nxtline;
			t=gotoln(line.value);
			if (t<0)
				return 0xCC;	// this needs an error message		
			setline(t);
			tmp=1;	
		}
		break;
	case RETURN: 

		if (gp>0) {
			setline(gosub[--gp]);
			tmp=1;
		} else {
			errno=7;
		}
		break;
	case DELETE:
	case SELECT:
		// i.e. DELETE 5
		// or   DELETE *
		//      DELETE 5,7
		//      DELETE 5,*
		{
			int i,n2;
			p=line.text;
			if (*p=='*')
			{
				p++;
				nis=0;
				break;
			}
			if (eval_expr(&p, &n)!=NUMBER)
			{
				errno=3; break;
			}
			if (n<0 || n>=128)
			{
				errno=3; break;
			}
			if (*p==',' && *(p+1)=='*')
			{
				p+=2;
				nis=n;
				break;
			}
			n2=1;
			if (*p==',')
			{
				p++;
				if (eval_expr(&p, &n2)!=NUMBER)
				{
					errno=3; break;
				}
				if (n2>n && n2<nis)
					n2=1+n2-n;
				else
				{
					errno=3;break;
				}
			}
			if(line.token==DELETE)
			{
				for (i=n; i<nis; i++)
					scene[i]=scene[i+n2];
				nis=nis-n2;
			}
			else
			{
				// select
				for (i=0; i<n2; i++)
					scene[i]=scene[i+n];
				nis=n2;
			}
		}
		break;
	case SET:
	case INSERT:
		// i.e. SET I,V
		// current array ![I]=V
		{
			int ind=0;

			p=line.text;
			if (eval_expr(&p, &n)!=NUMBER)
			{
				errno=3; break;
			}
			ind=n;
			if (*p++ != ',' || n<0 || n>=128)
			{
				errno=3; break;
			}
			if (eval_expr(&p, &n)!=NUMBER)
			{
				errno=3; break;
			}
			if (line.token==INSERT)
			{
				for (int i=nis-1; i>=ind; i--)
					scene[i+1]=scene[i];
				nis++;
			}
			else
				if (ind>=nis) nis=ind+1; // reset size
			scene[ind]=n;
		}
		break;
	case SORT: 
		if (line.var==1)
		{
			//Length of gen, no of generation, no to surive
			// e.g.
			// PRINT @{12,1,1,1,2,2,2,3,3,3,1,5,2}
			// SORT #3,3,3

			int i, param[3];
			p=line.text;
			for (i=0; i<3; i++)
			{
				eval_expr(&p, &param[i]);
				if (*p==',' && i<2)
				{
					p++;
				}
				else if (*p !=0)
				{
					errno=3; break;
				}
			}

			int sho=0; //1;
			int lgn=param[0];
			int nog=param[1];
			int nts=param[2];

			if (sho)
			{
				rprintf("length    = %d\n", lgn);
				rprintf("N of Gen  = %d\n", nog);
				rprintf("N to Save = %d\n", nts);
				if (nts<1 || nts>nog)
				{
					errno=3; break;
				}
			}

			for (i=0; i<nts; i++)
			{
				int j,k,mx;
				int sof=lgn*nog + i;

				mx=scene[sof];
				//rprintf("loop %d (%d,%d)\n", i, sof, mx);

				for (j=sof+1; j<(lgn+1)*nog; j++)
				{
					if (scene[j] > mx )
					{
						//swap fit factors
						//rprintf("swap %d and %d\n", i, i+(j-sof));
						scene[sof]= scene[j];
						scene[j]=mx;
						mx=scene[sof];
						//rprintf("MAX=%d\n", mx);
						//swap genes
						for (k=0; k<lgn; k++)
						{
							int t=scene[k+i*lgn];
							scene[k+i*lgn]=scene[k+(i+j-sof)*lgn];
							scene[k+(i+j-sof)*lgn]=t;
						}
					}
				}
			}
			nis = nts*lgn;
		}
		else
		{
			if (eval_expr(&line.text, &n) != ARRAY)
			{
				errno=1;
				break;
			}
			quicksort(scene,0,nis-1);
		}
		break;
	case SAMPLE: 
		//SAMPLE n,d
		// trigger level =n, max delay= d, (4ms per sample)
		{
			int n=4, d=2000; // defaults
			p=line.text;
			if (eval_expr(&p, &n)==NUMBER)
			{
				if (*p==',')
				{
					int i;
					p++;
					eval_expr(&p, &d);
					
					MIC_LEVEL=n;
					MIC_RATE=4; //4ms
					MIC_NOS=64; //max size for FFT
					sDcnt=0;
					
					if (*p==',')
					{
						p++;
						eval_expr(&p, &n);
						MIC_RATE = n;
						
						if (*p==',')
						{
							p++;
							eval_expr(&p, &n);
							if (n>0 && n<=128)
								MIC_NOS = n;
						}
					}

					for (i=0; i<MIC_NOS; i++) 
					{
						sData[i]=0;     // and clear
					}
					sample_sound(1);				

					MIC_STOP=1;  
					MIC_DLY=d/MIC_RATE; 
#ifdef AVR
					while (MIC_STOP==1)
					{
						// wait until sampling complete
					}
#else
					printf ("WIN: SAMPLE Level=%d,Dly=%d,Rate=%d,Nos=%d\n", MIC_LEVEL, MIC_DLY, MIC_RATE, MIC_NOS);
#endif
					if (MIC_DLY==0)
						nis=0;
					else
					{
						for (i=0; i<MIC_NOS; i++) 
						{
							scene[i]=sData[i];     // and clear
						}
						nis=MIC_NOS;
					}
				}
				else
					errno=1;
			}
		}
		break;
	case SCALE: 
		{
			int i,s;
			p=line.text;
			if (eval_expr(&p, &n) != ARRAY)
			{
				errno=1;
				break;
			}
			n=0;
			if (*p==',')
			{
				p++;
				eval_expr(&p, &n);
			}
			if (n>1)  //scale the array
			{
				s=scene[0];
				for (i=0; i<nis; i++)
				{
					if (scene[i]>s) s=scene[i];
					scene[i+nis]=0; // zero imag
				}
				s=n/s;
				for (i=0; i<nis; i++)
				{
					scene[i]=scene[i]*s; // scale
				}
			}
		}
		break;
	case FFT: 
		{
			int m,i,s;
			p=line.text;
			if (eval_expr(&p, &n) != ARRAY)
			{
				errno=1;
				break;
			}
			n=0;
			if (*p==',')
			{
				p++;
				eval_expr(&p, &n);
			}
			if (nis==8)
				m=3; // 16 elements
			else if (nis==16)
				m=4;
			else if (nis==32)
				m=5;
			else if (nis==64) // max must be <1/2 SDATASZ (for imag)
				m=6;
			else
			{
				rprintf ("Invalid sample size must be one of 8,16,32 or 64\n");
				errno=1;
				break;
			}
			if (n>1)  //scale the array
			{
				s=scene[0];
				for (i=0; i<nis; i++)
				{
					if (scene[i]>s) s=scene[i];
					scene[i+nis]=0; // zero imag
				}
				s=n/s;
				for (i=0; i<nis; i++)
				{
					scene[i]=scene[i]*s; // scale
				}
			}

			if (dbf) 
			{

				rprintf ("%d", nis);
				for (i=0; i<nis; i++)
				{
					rprintf (",%d", scene[i]);
				}
				rprintfStr("\r\n");
				
				rprintf ("FFT (%d) = %d\r\n", m, fix_fft(scene, &scene[nis], m, 0));
				
				for (i=0; i<nis; i++)
				{
					rprintf ("%d) = (%d %d) %d\r\n", i, 
						(int)scene[i],  
						(int)scene[nis+i], 
						(int)Sqrt((scene[i]*scene[i]) + (scene[nis+i]*scene[nis+i])));
				}

			}
			else
			{
				fix_fft(scene, &scene[nis], m, 0);
			}

			for (i=0; i<nis; i++)
			{
				scene[i]=(int)Sqrt((scene[i]*scene[i]) + (scene[nis+i]*scene[nis+i])); //power
			}

		}
		break;
	case GET:
		//tbc
		{
			int ch,c=0;
			while (ch!=10 && ch!=13)
			{
				while ((ch = uartGetByte())<0) ; // wait for input
				scene[c++]=ch;
				rprintfChar(ch);
			}
			nis=c-1;
		}
		break;
	case GEN:
		// GEN [No Gn], length, Mute rate%, Mute rnge, Val[min/max], type
		// GEN 4 16 5 2 0 254
		{
			int i, param[7];
			p=line.text;
			for (i=0; i<7; i++)
			{
				eval_expr(&p, &param[i]);
				if (*p==',' && i<6)
				{
					p++;
				}
				else if (*p !=0)
				{
					errno=3; break;
				}
			}

			int ty=param[6]&3; // 0-1 2
			int sho=param[6]&8; // show flag
			int nog=param[0];
			int ln=param[1];
			int mr=param[2];

			if (sho)
			{
				rprintf("Type      = %d\n", ty);
				rprintf("length    = %d\n", ln);
				rprintf("Generate  = %d\n", nog);
				rprintf("Mut rate  = %d\%\n", mr);
			}

			for (i=0; i<nog; i++)
			{
				int v,e;
				int co1=0;
				int co2=0;

				if (ty>0)
				{
					co1 = rand()%(ln+1);
					co2 = rand()%(ln+1);
					if (co2<co1) swap(&co1,&co2);
					if (sho)
					{
						if (ty>0) rprintf("CO pt1    = %d\n", co1);
						if (ty>1) rprintf("CO pt2    = %d\n", co2);
					}
				}

				for (e=0; e<ln; e++)
				{
					if (ty==0)
					{
						v=scene[e];    // gene from parent 1
					}
					if (ty==1)
					{
						// single cross over
						if (e<co1)
							v=scene[e]; //straight copy p1
						else
							v=scene[e+ln]; //straight copy p2
					}
					if (ty==2)
					{
						// two cross over
						if (e<co1 || e>=co2)
							v=scene[e]; //straight copy p1
						else
							v=scene[e+ln]; //straight copy p2
					}

					if (rand()%100<mr)
					{
						v += (rand()%(2*param[3]))-param[3];
						if (v<param[4]) v=param[4];
						if (v>param[5]) v=param[5];
					}

					if (ty==1 || ty==2)
						scene[2*ln+(ln*i)+e]=v; // copy
					else
						scene[ln+(ln*i)+e]=v; // copy

				}

			}

			nis = nog*ln;
			for (i=0; i<nis; i++)
			{
				if (ty==1 || ty==2)
					scene[i]=scene[i+2*ln];
				else
					scene[i]=scene[i+ln];
			}
		}
		break;
	case NETWORK:
		// NETWORK  [no inputs],[no outputs],[flgs],[nn ly1],[nn ly2],[nl3], [offset]
		// @! =I1 .. IN  O1 .. OM  W11 ..T1  WNM  .. TN
		{
			int i, j, param[7];
			p=line.text;
			for (i=0; i<7; i++)
			{
				param[i]=0;
				eval_expr(&p, &param[i]);
				if (*p==0 && i>5) break;
				if (*p==',' && i<6)
				{
					p++;
				}
				else if (*p !=0)
				{
					errno=3; break;
				}
			}
			// code here
			int noi=param[0], noo=param[1], flg=param[2], sho=0;
			int nl1=param[3], nl2=param[4], nl3=param[5];
	        int ofset=param[6];

	        sho = (flg&8); // show output
	        flg = flg & 7; // 0, 1, 2, 3  or 4 (sigmoid mode)

			if (noi<=0 || noo<=0)
			{
				// number input & output >0
				rprintfStr("Err:: Input and output must be gt 0\n");
				errno=3;
				break;
			}

			if (nl3!= noo && nl3!=0)
			{
				//layer 3 = number outputs
				rprintfStr("Err:: layer 3 neurons must match output (or be zero)\n");
				errno=3;
				break;
			}

			if (nl3==0 && nl2!=0)
			{
				//layer 3 = number outputs
				rprintfStr("Err:: layer 3 must be non-zero if layer 2 has nodes \n");
				errno=3;
				break;
			}

			int l1o[nl1];
			int l2o[nl2];
			int l3o[nl3];

		    i=ofset;
		    if (nl2==0)
		    	ofset= ofset* (((noi+1)*nl1) + ((nl1+1)*nl3));
		    else
		    	ofset= ofset* (((noi+1)*nl1) + ((nl1+1)*nl2) + ((nl2+1)*nl3));

			if (sho)
			{
				rprintf("NOI   = %d\n", noi);
				rprintf("NOO   = %d\n", noo);
				rprintf("Flags = %d\n", flg);
				rprintf("NO L1 = %d\n", nl1);
				rprintf("NO L2 = %d\n", nl2);
				rprintf("NO L3 = %d\n\n", nl3);

				if (ofset>0) rprintf("Offset = %d (%d)\n\n", i,ofset);
			}

			int t=noi+noo+ofset-1; // index through weights and threshold

			for (i=0; i<nl1; i++)
			{
				if (sho) rprintf("INPUT NEURON = %d\n", i+1);
				int s=0;
				for (j=0; j<noi; j++)
				{
					t=t+1;
					s += scene[j]*scene[t];
					if (sho) rprintf("Input=%d (%d x %d)\n",j,scene[j],scene[t]);
				}
				t++;
				if (sho) rprintf("Th=-(%d)\n",scene[t]);

				s -= scene[t];
				l1o[i]=sigmoid(s,flg);
				if (sho) rprintf("O=%d\n", l1o[i]);
			}

			for (i=0; i<nl2; i++)
			{
				int s=0;
				if (sho) rprintf("HIDDEN NEURON = %d\n", i+1);
				for (j=0; j<nl1; j++)
				{
					t++;
					s += l1o[j]*scene[t];
					printf("Input=%d (%d x %d)\n", j,l1o[j],scene[t]);
				}
				t++;
				if (sho) rprintf("Th=-(%d)\n",scene[t]);

				s -= scene[t];

				l2o[i]=sigmoid(s,flg);
				if (sho) rprintf("O=%d\n", l2o[i]);
			}

			for (i=0; i<nl3; i++)
			{
				int inp, s=0;
				if (sho) rprintf("OUTPUT NEURON = %d\n", i+1);

				if (nl2==0)
				{
					for (j=0; j<nl1; j++)
					{
						t++;
						inp=l1o[j];
						s += inp*scene[t];

						if (sho) rprintf("Input=%d (%d x %d)\n",j,inp,scene[t]);
					}
				}
				else
				{
					for (j=0; j<nl2; j++)
					{
						t++;
						inp=l2o[j];
						s += inp*scene[t];

						if (sho) rprintf("Input=%d (%d x %d)\n",j,inp,scene[t]);
					}
				}


				t++;
				if (sho) rprintf("Th=-(%d)\n",scene[t]);

				s -= scene[t];

				l3o[i]=sigmoid(s,flg);
				if (sho) rprintf("O=%d\n", l3o[i]);

			}

			for (i=0; i<noo; i++)
			{
				if (nl3==0)
					scene[noi+i]=l1o[i]; // if no output layer use layer 1
				else
					scene[noi+i]=l3o[i];

				if (sho) rprintf("O=%d\n", scene[noi+i]);
			}

		}
		break;
	default:
		errno=8; // DEBUG
		tmp=0xCC;
	}		
	return tmp;
}

void basic_clear()
{
	// Set Init pointer to Zero
	rprintfStr("Clear Program \r\n");
	clearln();
}

void basic_zero()
{
	// Set Init pointer to Zero
	int i;	
	BYTE data= 0x00; 				// start of program byte
			
	for (i=0; i<EEPROM_MEM_SZ; i++) 	
	{
		eeprom_write_byte(BASIC_PROG_SPACE+i, data);
	}
	basic_clear();
}

void dump(int sz)
{
	int i;
	
	for (i=0; i<EEPROM_MEM_SZ; i+=sz) 	
	{
		int j;
		char asciis[9];
		rprintf ("%x ", i);
		for (j=0; j<sz;  j++)
		{
			BYTE data = eeprom_read_byte((BYTE*)(BASIC_PROG_SPACE+i+j));
			rprintf ("%x ", data);
			if (sz==8) {if (data>27 && data<127) asciis[j]=data; else asciis[j]='.';}
		}
		if (sz==8) 
		{ 
			asciis[8]='\0';
			rprintfStr (asciis);
		}
		rprintfStr ("\r\n");	
	}
}

void dump_firmware()
{
	int i;
	
	for (i=0; i<64; i+=8) 	
	{
		int j;
		char asciis[9];
		rprintf ("%x ", i);
		for (j=0; j<8;  j++)
		{
			BYTE data = eeprom_read_byte((BYTE*)(FIRMWARE+i+j));
			rprintf ("%x ", data);
			if (data>27 && data<127) asciis[j]=data; else asciis[j]='.';
		}
		asciis[8]='\0';
		rprintfStr (asciis);
		rprintfStr ("\r\n");	
	}
}

void basic_list()
{
	line_t line;
	char buf[MAX_LINE];

	BYTE tmp=0;
	nxtline = 0;	

	rprintfStr("List Program \r\n");

	if (nextchar() != 0xAA ) {
		rprintfStr("No program loaded\r\n");
		return;
	}	
	
	firstline();
	tmp=nextchar();

	while (tmp != 0xCC && nxtline < EEPROM_MEM_SZ )
	{
		line = readln(buf);
		tmp = nextchar();

		if (line.lineno==0) continue;

		/* list code */
	
		rprintf ("%d ", line.lineno); 
		rprintfStr (tokens[line.token]);
		rprintf (" "); 
		
		if (line.token==LET || line.token==GET || line.token==FOR || line.token==LIST || line.token==DATA)
			rprintf ("%c = ", line.var+'A');
			
		if (line.token==PUT)
		{
			if (line.var <30)
			{
				rprintfStr (specials[line.var]); 
			}
			else
			{
				char a, b;
				a='A' + ( (line.var - 30) /10) ;
				b='0' + (line.var % 10) ;
				rprintf ("PORT:%c:%c", a, b);			
			}
			rprintfStr (" = ");
		}

		if (line.token==SERVO || line.token==STEP)
		{
			if (line.var>=32)
				rprintf ("%d=", line.var-32);
			else
				rprintf ("%c=", line.var+'A');
		}
			
		if (line.token==PRINT && line.var==1)
			rprintf ("# ");

		if (line.token==PRINT && line.var==2)
			rprintf ("@ ");
			
		if (line.token==IF)
		{
			// replace ? THEN,  : ELSE (unless :0)
			char *cp2, *p_then=0, *p_else=0, *cp = line.text;
			int l = strlen(cp);
			//if (l>2 && cp[l-2]==',' && cp[l-1]=='0')
			//{
			//	cp[l-2]=' ';  cp[l-1]=' ';
			//}
			//p_then = strstr(cp, ")?  ");
			//p_else = strstr(cp, ":   ");
			//if (p_then != 0)
			//	strncpy(p_then, "THEN",4);
			//
			//if (p_else != 0)
			//	strncpy(p_else,"ELSE",4);

			cp2=cp+l;

			while (cp2>cp)
			{
				if (*cp2==',')
				{
					if (p_else==0)
					{
						p_else=cp2;
						*p_else='\0';
					}
					else if (p_then==0)
					{
						p_then=cp2;
						*p_then='\0';
						break;
					}
				}
				cp2--;
			}

			rprintfStr (cp);
			rprintfStr (" THEN ");
			rprintfStr (p_then+1);
			if (!(*(p_else+1)=='0' && *(p_else+2)==0))
			{
				rprintfStr (" ELSE ");
				rprintfStr (p_else+1);
			}
		}
		else
		if (line.token==NEXT) 
		{
			rprintf ("%c", line.var+'A');
		}
		else
		if (line.token==DATA)
		{
			unsigned char i;
			unsigned char n=(unsigned char)(*(line.text+1));
			rprintf ("%d", (unsigned char)line.text[2]);
			for (i=1; i<n; i++)
				rprintf (",%d", (unsigned char)line.text[2+i]);
		}
		else
		if (line.token==FOR)
		{
			rprintfStr (line.text);
		}
		else
		if (line.token==GOTO || line.token==GOSUB || line.token==PLAY  ) 
			rprintf ("%d", line.value);
		else
			rprintfStr (line.text);


		rprintf ("\r\n");
	}
}

#ifdef AVR
#define MAGIC_RESPONSE	0xEA
#define MAGIC_REQUEST	0xCD
#define VERSION			0x12     /* BIN API VERSION */
#define MAX_INP_BUF 	40
#define PROTOCOL_ERROR	01

void SendResponse(char mt, BYTE d)
{
	PWR_LED1_ON;
	rprintfChar(MAGIC_RESPONSE);
	rprintfChar(mt);
	rprintfChar(d);
	rprintfChar( (mt ^ d) & 0x7F);
	PWR_LED1_OFF;
}

int bin_downloadbasic()
{
	int i;
	int b0=0, b1=0, bytes;
	int cs = 0;

	b0 = GetByte();    
	b1 = GetByte();
		
	bytes = (b1 << 8) | b0;
	cs ^= b0;
	cs ^= b1;
	
	for (i=0; i<bytes; i++)
	{
		b0 = GetByte();
		cs ^= b0;
		eeprom_write_byte(BASIC_PROG_SPACE+i, b0 % 256);	
	}	
	
	b0 = GetByte();
	return (b0 == (cs&0x7f));
}

int bin_read_request()
{
	int mt;
	int b0 = GetByte();
	
	if (b0==MAGIC_REQUEST)
	{
		mt = GetByte();

		if (mt=='l')
		{
			if (bin_downloadbasic())
				return mt;
			else
				return 7; //dowload not ok
		}
	}
	return 4; // not got MAG request
}

void binmode()
{
	int r;
	rprintfStr("start download\r\n");
	RUN_LED2_ON;
	
	r=bin_read_request();
			
	if (r == 'l')
		SendResponse('l', VERSION);
	else
		SendResponse('z', r);
		
	RUN_LED2_OFF;
}
#endif

int outdigit(int n,int c)
{
	char tb[6];
	int i;
	if (c>4) return -1;

	for (i=0; i<c; i++)
	{
		tb[c-i-1]=(n%10)+'0';
		n=n/10;
	}
	tb[c]='\0';
	rprintfStr(tb);
}
int uptime()
{
        outdigit(gHOUR,2);
        rprintfChar(':');
        outdigit(gMIN,2);
        rprintfChar(':');
        outdigit(gSEC,2);
        rprintfChar('-');
        outdigit(gMSEC,3);
}

void testforreset()
{
	if((PINA&03) == 0)  // PF1 on, PF2 off
	{
		delay_ms(10);
		if((PINA&3) == 0)
		{
			rprintf   ("reset\r\n");
			basic_clear();
		}
	}
}

void init()
{
	int i;
	for (i=0; i<16; i++) offset[i]=0;
}

void basic()
{
	int ch;

	rprintfStr("Basic v=$Revision$\r\nCommands: i r l c z q s V R F $\r\n");

	testforreset();

	void init();

	rprintf   ("%d servos connected\r\n", readservos(0));
	rprintf   ("%d lines in memory\r\n", findend());

	sound_init();	// sound output chip on (if available)
	//sample_sound(1); // sound meter on

	while (1)
	{
		rprintfStr(": ");

		ch = GetByte();
		if (ch > 26) {
			rprintfChar(ch);
			if (!(ch== 'G'|| ch=='g')) rprintfCRLF();
		}
		switch (ch)
		{
		case 'i': // input
			basic_load(0);
			break;
		case 'I': // input
			basic_load(1);
			break;
		case   7:  // IR red "stop" button
		case 'r': // run
		    {
		    	int m=gMIN,s=gSEC,ms=gMSEC;
			    basic_run(0);
			    ms=gMSEC-ms;
			    if (ms<0) {ms=ms+1000; s=s-1;}
			    s=gSEC-s;
			    if (s<0)  {s=s+60; m=m-1;}
			    m=gMIN-m;
			    rprintf("Elapsed Time ");
		        outdigit(m,2);
		        rprintfChar(':');
		        outdigit(s,2);
		        rprintfChar('-');
		        outdigit(ms,3);
		        rprintf("\r\n");
		    }
			break;
		case 'R': // run
			basic_run(1);
			break;

		case 'G': // gosub
		case 'g': // gosub
			{
			int ch2,t,gn=0;
			while(1)
			{
				ch2=GetByte();
				if (ch2 <'0' || ch2>'9') break;
				rprintfChar(ch2);
				gn=gn*10+ch2-'0';
			}
			rprintfCRLF();
			t=gotoln(gn);
			if (t>0)
			{
				//rprintf("Go(sub) %d = %d\n",gn,t);
				setline(t);
				basic_start(ch == 'G');
			}
			else
				rprintf("? No such line %d\n",gn);
			}
			break;
		case 'l': // list 
			basic_list();
			break;
		case 'c': // clear 
			basic_clear();
			break;

		case 's': // stand
		case 22:  //*+A
			rprintf("Stand %d\r\n", nos);
			standup(nos);
			break;

		case 'S': // stand
			rprintf("ok\r\n");
			setdh(1);
			standup(nos);
			break;

		case 'o': // output lights
			sample_sound(1); // sound meter on
			break;

		case 'O': // 
			rprintf("PSD off\r\n");
			PSD_off();
			break;

		case 'X': // charge
			rprintf("testing charge\r\n");
			chargemode();
			break;

		case 'C': // zero &clear 
			basic_zero();
			break;
		case 'V': // version
			rprintfStr("v=$Revision$\r\n");
			break;
		case 'd': // dump narrow
			dump(8);
			break;
		case 'D': // dump wide
			dump(24);
			break;
		case 'F': // dump firmware
			dump_firmware();
			break;
		case 'z': // download 
			basic_clear();
			binmode(); // enter binary mode
			rprintf ("%d lines loaded\r\n", findend());
			break;
		case '~':
			dbg=1-dbg;
			if (dbg) rprintf("DEBUG ON\n");
			break;

#ifndef AVR
		case 'Z': // upload  (Unix version only)
			binstore();
			rprintf ("stored\r\n");
			break;
#endif
		case 'q': // query 
			rprintf ("%d servos connected\r\n", readservos(0));
			rprintf ("%d lines stored\r\n", findend());
			rprintf ("Uptime: "); uptime(); rprintf ("\r\n");

			break;

		case '$':
		case 23: //*+1 Demo mode
			rprintf ("DEMO MODE\r\n");

			while (1)
			{
				rprintf (">: ");
				ch = GetByte();
				rprintf ("%d\r\n",ch);

				if (ch == 7)
				{
					rprintf ("EXIT DEMO MODE\r\n");
					break;
				}

				if (ch>=0 && ch<20)
				{
					PerformAction(map[ch]);
				}
			}

			break;
		default:
			rprintfStr("\r\n");
			break;
		}
	}
}
