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
#include "wckmotion.h"

// Standard Input/Output functions

#include "main.h"
#include "global.h"
#include "macro.h"

#include "adc.h"
#include "ir.h"
#include "accelerometer.h"

//#define Sqrt sqrt

int Sqrt(long x)
{
   double f=sqrt((double)x);
   return (int)f;
}

extern int strlen	(char *);
extern void strcat	(char *, char *);
extern void strcpy	(char *, char *);
extern void strncpy	(char *, char *, int);
extern char *strstr	(char *, char *);
extern int rand();
int dbg=0;

int remote=0;

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
#include "lists.h"

#define MAX_LINE  		100 
#define MAX_TOKEN 		8
#define MAX_FOR_NEST	6
#define MAX_GOSUB_NEST	5
#define MAX_FOR_EXPR	20 
#define SCENESZ 		128

extern int dbg;

/***********************************/

BYTE EEMEM FIRMWARE        	[64];  		// leave blank - used by Robobuilder OS
BYTE EEMEM BASIC_PROG_SPACE	[EEPROM_MEM_SZ];  // this is where the tokenised code will be stored
BYTE EEMEM PERSIST		[256];                     // persistent data store

extern void Perform_Action	(BYTE action);
extern int	getHex		(int d);
extern int	delay_ms	(int d);
extern void SampleMotion	(unsigned char); 
extern void sound_init		();
extern void SendToSoundIC	(BYTE cmd) ;

extern void  blights(int n, int *vals) ; // from adc.c

/***********************************/

extern int chargemode();

void basic_start(int dbf);

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

const prog_char tokens[NOTOKENS][7] ={
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
	"GEN",  "NETWOR", "SELECT", "!"
};

extern const prog_char *specials[];
extern const prog_char map[];


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

int BasicErr;
int fmflg;


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
		rprintfProgStr(PSTR("Motion range:  0 - 18\r\n"));
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

int token_match(prog_char list[][7], char **p_line, int n)
{
	int t;
	int t1;
	char buff[MAX_TOKEN];
	
	t1=getToken(*p_line, buff);  // this could buffer over run - needs fixing
	
	for (t=0; t<n; t++)
	{
		if (!strcmp_P(buff, list[t]))
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
			rprintfProgStr(PSTR("Program entry mode (. to exit)\r\n"));	
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
	
	BasicErr=0;

	rprintfProgStr(PSTR("Enter Program '.' to Finish\r\n"));
	
	while (1)
	{
		// for each line entered

		if (BasicErr > 0 && BasicErr<6) 
		{
			rprintfProgStr (PSTR("Error - '" ));
			rprintfStr (error_msgs[BasicErr]);
			rprintfProgStr (PSTR("'\r\n") );
			rprintf ("Pos=%d [%c]\r\n", (cp-line), line[cp-line]);
			BasicErr=0;
		}
		else if (BasicErr!=0)
		{
			rprintf ("Panic %d\r\n", BasicErr);
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
				rprintf ("Delete - %d\r\n", newline.lineno);
				deleteln(newline.lineno);
				continue;
			}
			cp++;
		}

		while (*cp == ' ') 
			cp++;

		if (*cp == '!') {
			newline.token=EXTEND;
			cp++;
			//printf ("extended\n");
		}
		else
		{
			//printf ("normal\n");
			if ( (newline.token=token_match(tokens, &cp, NOTOKENS))<0)           // sizeof(tokens)/))<0)
			{
				BasicErr=2;
				continue;
			}
		}
			
		while (*cp == ' ') cp++;
		
		switch (newline.token)
		{
		case LET: 
		case FOR:
			// read Variable		
			if ((newline.var = getVar(&cp))<0)
			{
				BasicErr=3;
			}
			// '='
			if (getNext(&cp) != '=')
			{
				BasicErr=1;
			}
			// expression
			if (newline.token==FOR)
			{
				char *to = strstr(cp," TO ");
				if (to==0) BasicErr=1;
			}
			//else
				newline.text=cp;
			break;
		case POKE: 
		    //poke address, byte
			newline.var = getNum(&cp);
			if (getNext(&cp) != ',')
			{
				BasicErr=1;
			}			
			newline.text=cp;
			break;
		case PUT: 
			//if ((newline.var = token_match(specials, &cp, sizeof(specials)))<0)
			if ((newline.var = token_match(specials, &cp, NOSPECS))<0)
			{
				BasicErr=3;
			}
			
			if (newline.var == sPORT)
			{
				char pn, pb;
				if (getNext(&cp) != ':')
				{
					BasicErr=3;
				}	

				pn = getNext(&cp) ;
				
				if (pn<'A' || pn >'G') 
				{
					BasicErr=3;
				}
				
				if (getNext(&cp) != ':')
				{
					BasicErr=3;
				}	
				
				pb = getNext(&cp) ;
				
				if ((pb<'0' || pb >'8'))
				{
					BasicErr=3;
				}
				
				//Now have PORT:$pn:$pb
				
				newline.var = 30 + (pn-'A')*10 + (pb-'0') ;
			}
			else
			{
				BasicErr=1;
			}
			// '='
			if (getNext(&cp) != '=')
			{
				BasicErr=1;
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
					BasicErr=3;
					break;
				}
				newline.var += 32;
			}
			// '='
			if (getNext(&cp) != '=')
			{
				BasicErr=1;
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
		case EXTEND:
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
				BasicErr=3;
			}
			// '='
			if (getNext(&cp) != '=')
			{
				BasicErr=1;
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
		case IF:
			// A THEN B ELSE C =>  A,B[,C]
			{
				char *p_then,*p_else;
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
					BasicErr=3;

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
				BasicErr=3;
			}
			break;
		default:
			BasicErr=2;
			break;
		}			
		
		if (BasicErr==0)
		{	
			if (newline.lineno==0)
			{
				if (    //newline.token==DATA || 
					    newline.token==FOR || newline.token==GOTO
						|| newline.token==NEXT || newline.token==RETURN
						|| newline.token==IF || newline.token==GOSUB)
				{
					rprintfProgStr(PSTR("Command not available in immediate mode\r\n"));
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

int scene[SCENESZ];	  // generic array
int nis=0;        // number of item isn array

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

 
void quicksort(int list[],int m,int n)
{
   int key,i,j,k;
   if( m < n)
   {
      k = (m+n)/2;
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




static int  forptr[MAX_FOR_NEST];   // Upto 3 nested for/next
static char nxtptr[MAX_FOR_NEST][MAX_FOR_EXPR];   // Upto 3 nested for/next
static int fp; 

static int gosub[MAX_GOSUB_NEST];  // 3 nested gosubs
static int gp;  
		
void basic_run(int dbf)
{
	fmflg=0;

	nxtline=0;

	if (nextchar()!=0xAA) {
		rprintfProgStr(PSTR("No program loaded\r\n"));
		return;
	}

	//point top at EEPROM
	rprintfProgStr(PSTR("Run Program \r\n"));

	listinit ();

	firstline();

	basic_start(dbf);
}

char buf[MAX_LINE];

void basic_start(int dbf)
{
	// Repeat
	//   load from memory
	//   Execute action
	// Loop

	BYTE tmp=0;
	line_t line;
	
	BasicErr=0;

	fp=0; // Upto 3 nested for/next

	gp=0;  // 3 nested gosubs

	cs=0; // reset line number cache

	while (tmp != 0xCC && nxtline < EEPROM_MEM_SZ )
	{
		int tc;
		if (BasicErr !=0)
		{
			rprintf("Runtime error %d on line %d\r\n", BasicErr, line.lineno);
			return;
		}

		line = readln(buf);
		if (line.lineno==0)
		{
			rprintfProgStr(PSTR("End\r\n"));
			return;
		}
		
		tc = nextchar();	// terminator character ?
		
		if (dbf) {
			int kp;
			rprintf ("TRACE :: %d - ", line.lineno); 
			rprintfProgStr (tokens[line.token]);;
			while ((kp=uartGetByte())<0) ; // wait for input
			if (kp == 27)  
			{
				rprintfProgStr (PSTR(" ** User Break\r\n")); 
				return;
			}

			rprintfCRLF();

			if (kp == 'l')  
			{
				listdump();
			}

			if (kp == 'v')  
			{
				showvars();
			}
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
				rprintf ("%c = %ld\r\n", line.var+'A', variable[line.var]);
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
	long n;

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
				BasicErr=1;
			else
			{
				strncpy(nxtptr[fp],to+4, MAX_FOR_EXPR);
				*to='\0'; // null terminate expr1
				if (eval_expr(&p, &n)==NUMBER)
					variable[line.var] = n;		
			}

			//if (dbg) {
			//	rprintf("DBG: for %d-> %d\n", fp, forptr[fp]);
			//}

			fp++;
		}
		break;
	case NEXT: 
		// increment var and check condition
		if (fp>0)
		{
			int t_ptr=forptr[fp-1];

			// increment var
			variable[line.var] = variable[line.var] + (long)1;

			//if (dbg) {
			//	rprintf("DBG: NEXT %d-> %d  %d %ld\n", fp-1, t_ptr, line.var, variable[line.var]);
			//}

			// test against expr2 i.e var<=expr2
			n=0;
			p=nxtptr[fp-1];
			if (eval_expr(&p, &n)!=NUMBER)
			{
				BasicErr=1;
				break; //need to handle error	
			}

			//if (dbg) {
			//	rprintf("DBG: VAR %c  %ld<= %ld\n", line.var+'A', variable[line.var], n);
			//}
			
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
 			else if (eval_expr(&p, &n)==NUMBER)
			{
				// put result into address line.var
				BYTE addr=line.var;
                if (f)
				   eeprom_write_byte(FIRMWARE+addr, n);
                else
				   eeprom_write_byte(PERSIST+addr, n);                             
			}
			else
				BasicErr=1;
        }
		break;
	case PUT: 
		n=0;
		p=line.text;
		if (eval_expr(&p, &n)==NUMBER)
			put_special(line.var, n);
		else
			BasicErr=1;
		break;
	case LET: 
		n=0;
		p=line.text;
		if (eval_expr(&p, &n)==NUMBER)
			variable[line.var] = n;
		else
			BasicErr=1;
		break;
	case SERVO:
		{
			long v=line.var;
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
				BasicErr=3; return 0xcc;	
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
				//rprintfNum (10, 6, 1,' ', n);
				rprintf ("%ld", n); //COMPLEX FORM
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
				if (listsize(arrayname)>0)
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
						int k,sz; long w=0;
						if (*p==':')
						{
							p++;
							if (eval_expr(&p, &w) != NUMBER)
							{
								BasicErr=3; break;
							}
						}

						sz=listsize(arrayname);
						if (w<=0)  w=sz;
						if (w>sz) w=sz;
						for (k=0; k<=(sz/w); k++)
						{
							if (k*w<sz) rprintf ("%d", listread(arrayname, k*w));
							for (n=1; (n<w) && ((k*w+n)<listsize(arrayname)); n++) {rprintf (",%d",listread(arrayname, k*w+n));}
							if (k*w<sz) rprintfCRLF();
						}
					}
				}
				else
					rprintfStr("Null");
			}

			if (*p=='\0') break; // done

			if (*p!=';' && *p!=',') {
				BasicErr=4; //rprintf ("synatx prob= [%d]\r\n", *p); // DEBUG
				break;
			}
			if (*p==';' && *(p+1)=='\0') // last one
				break;
			p++;
		}
		// check for last ; (no crlf)
		if (*p!=';')	
			rprintfCRLF (); 			
		break;	
	case IF:		
		n=0;
		p=line.text;

		if (eval_expr(&p, &n)==NUMBER)
		{
			if (*p++ != ',')
			{
				BasicErr=5; break;
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
					BasicErr=5;
				else
					setline(t);
				tmp=1;
			}
		}
		break;		
	case MOVE: 
		// MOVE @A,C,D
		// Move to position @A, in C steps, taking D ms
		// MOVE @A
		// No args - send servo positions synchronously
		// with args (No Frames / Time in Ms) - use MotionBuffer
		// MOVE A,B,C,D
		// Move to position @![a,a+b],C,D
		p=line.text;
			
		if (p!=0 && *p != 0)
		{	
			int j, st=0;
			long tm=0,fm=0, nb=0;
			BYTE pos[32];

			switch (eval_expr(&p, &fm))
			{
			case ARRAY:
				listreadc(arrayname);
				if (*p=='\0')
				{
					if (nis>0)
						wckSyncPosSend(nis-1, speed, scene, 0);
					return tmp;
				}
				break;
			case NUMBER:
				st=fm;
				if (*p==',')
				{
					p++;
					if (eval_expr(&p, &nb) != NUMBER)
					{
						BasicErr=1;
						return 0;
					}
				}
				if (st>=nis || st+nb>nis)
				{
					BasicErr=5;
					return 0;
				}
				break;
			}

			if (*p++ != ',') { BasicErr=1; break;}
			eval_expr(&p, &fm);
			if (*p++ != ',') { BasicErr=1; break;}
			eval_expr(&p, &tm);

			if (nb==0) nb=nis-st;

			for (j=0; j<nb; j++)
			{
				if (scene[j+st]>=0 && scene[j+st]<=254)
					pos[j] = scene[j+st];
				else
				{
					BasicErr=5;
					return 0;
				}

				if (j<16)
				{
					pos[j] += offset[j];
				}

				if (pos[j]<0)
					pos[j] = 0;

				if (pos[j]>254)
					pos[j] = 254;

				if (dbg) rprintf("DBG:: MOVE %d=%d\n", j, pos[j]);
			}

			if (!dbg) PlayPose(tm, fm, speed, pos, (fmflg==0)?nb:0);
			fmflg=1;
		}
		//
		break;
	case I2CO: 	//IC2O Addr,@{5,1,1,1,1}
	{
		BYTE ob[20];
		long i, addr=0;
		p=line.text;
		if (eval_expr(&p, &addr)==NUMBER)
		{
			if (*p++ != ',') { BasicErr=1; break;}
			if (eval_expr(&p, &n) != ARRAY)
			{
				BasicErr=1;
				return 0;
			}
			listreadc(arrayname);
			for (i=0; i<nis; i++)
			  ob[i]=scene[i];
			I2C_write(addr, nis, ob);
		}
		break;
	}
	case I2CI: 	//IC2I Addr,n[,@{}]
		{
		int i; long ibc=0, addr=0;
		p=line.text;			
		if (eval_expr(&p, &addr)==NUMBER)
		{
			BYTE ob[20];
			BYTE ib[20];
			
			if  (*p++ != ',')
			{
				BasicErr=1;
				return 0;
			}
			if (eval_expr(&p, &ibc)==NUMBER)
			{
				if  (*p == ',')
				{
					if (eval_expr(&p, &n) != ARRAY)
					{
						BasicErr=1;
						return 0;
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
		//new lists
		list_eval((line.var==32)?'!' : ('A' + line.var), line.text, 0);
		break;
	case DATA: 
		list_eval((line.var==32)?'!' : ('A' + line.var), line.text, 1);
		break;		
	case XACT:
	case RUN:
		n=0;
		p=line.text;
		if (eval_expr(&p, &n) != NUMBER)
		{
			BasicErr=1;
			return 0;
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
				BasicErr=1;
				return 0;
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
				if (arrayname=='!')
				{
					for (i=0; i<16 && i<nis; i++)
					{
						offset[i]=scene[i];
					}
				}else{
					//
					int sz=listsize(arrayname);
					for (i=0; i<16 && i<sz; i++)
					{
						offset[i]=listread(arrayname,i);
					}
				}
			}
			else
				BasicErr=1;
		}
		break;
	case OUT: 
		{
		long l=1;
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
			while (l>0) { rprintfChar(n); l--;}
			break;
		case ARRAY:
			for (l=0; l<listsize(arrayname); l++)
				rprintfChar(listread(arrayname,l));
			break;
		}
		}
		break;
	case STEP: 
		//STEP servo=from,to[,inc][,dlay]
		{
			long sf, st, si=5, sp, sn, sw=75, cnt=0, sd=8;
			long v=line.var;
			if (v>=0 && v<=31)
				v=variable[v];
			else
				v=v-32;
			n=0;
			p=line.text;
			if (eval_expr(&p, &sf)!=NUMBER)
			{
				BasicErr=3; return 0;
			}
			if (*p++ != ',')
			{
				BasicErr=2; return 0;
			}
			if (eval_expr(&p, &st)!=NUMBER)
			{
				BasicErr=3; return 0;
			}
			if (*p==',')
			{
				p++;
				if (eval_expr(&p, &si)!=NUMBER)
				{
					BasicErr=3; return 0;
				}
				sd=si+1;
				if (*p==',')
				{
					p++;
					if (eval_expr(&p, &sd)!=NUMBER)
					{
						BasicErr=3; return 0;
					}
					if (*p==',')
					{
						p++;
						if (eval_expr(&p, &sw)!=NUMBER)
						{
							BasicErr=3; return 0;
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
			BasicErr=3; break;
		}
		if (*p==',')
		{
			//LIGHTS A,@{5,5,10,15,20,25}
			long tn;
			p++;
			if (eval_expr(&p, &tn)!=ARRAY)
			{
				BasicErr=3; return 0;
			}

			listreadc(arrayname);

			MIC_SAMPLING=0;
			if (nis==5)
				blights(n, scene);
			else
			{
				BasicErr=3; // must be 5 elements
				return 0;
			}
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
			BasicErr=3; break;
		}
		if (n<0) PP_mtype=0;
		PP_mtype=n%4;
	case SPEED: 
		p=line.text;
		if (eval_expr(&p, &n)!=NUMBER)
		{
			BasicErr=3; break;
		}
		if (speed<0) n=0;
		speed=n%4;
		break;
	case END: 
		rprintfProgStr (PSTR("End of program\r\n")); 
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
			BasicErr=7;
		}
		break;
	case DELETE:
	case SELECT:
		// i.e. DELETE 5
		// or   DELETE *
		//      DELETE 5,7
		//      DELETE 5,*
		{
			long n2;
			char an='!';
			p=line.text;

			if (*p=='@')
			{
				p++;
				an=*p;
				if (an != '!' && ( an<'A' || an>'Z')  || *(p+1) != ',')
				{
					BasicErr=3; break;
				}
				p++;				
				p++;
			}
			if (*p=='*')
			{
				p++;
				listdelete(an, 0,0,2); 
				break;
			}
			if (eval_expr(&p, &n)!=NUMBER)
			{
				BasicErr=3; break;
			}
			if (n<0 || n>=128)
			{
				BasicErr=3; break;
			}

			n2=1;
			if (*p==',')
			{
				p++;
				if (*p=='*')
				{
					p++;
					n2=0;
				}
				else
				if (eval_expr(&p, &n2)!=NUMBER)
				{
					BasicErr=3; break;
				}
			}
			else
				n2=n;

			listdelete(an, n, n2, line.token==DELETE);
		}
		break;
	case SET:
	case INSERT:
		// i.e. SET [@A,]I,V
		// current array ![I]=V
		{
			int ind=0;
			char an='!';
			p=line.text;

			if (*p=='@')
			{
				p++;
				an=*p;
				if (an != '!' && ( an<'A' || an>'Z')  || *(p+1) != ',')
				{
					BasicErr=3; break;
				}
				p++;
				p++;
			}

			if (eval_expr(&p, &n)!=NUMBER)
			{
				BasicErr=3; break;
			}
			ind=n;
			if (*p++ != ',' || n<0 || n>=128)
			{
				BasicErr=3; break;
			}
			if (eval_expr(&p, &n)!=NUMBER)
			{
				BasicErr=3; break;
			}

			listset(an, ind, n, line.token==INSERT);
		}
		break;
	case SORT: 
		if (line.var==1)
		{
			//Length of gen, no of generation, no to surive
			// e.g.
			// PRINT @{12,1,1,1,2,2,2,3,3,3,1,5,2}
			// SORT #3,3,3

			long i, param[3],sho=0,lgn,nog,nts;
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
					BasicErr=3; break;
				}
			}

			sho=0; //1;
			lgn=param[0];
			nog=param[1];
			nts=param[2];

			if (sho)
			{
				rprintf("length    = %d\n", lgn);
				rprintf("N of Gen  = %d\n", nog);
				rprintf("N to Save = %d\n", nts);
				if (nts<1 || nts>nog)
				{
					BasicErr=3; break;
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
				BasicErr=1;
				break;
			}
			quicksort(scene,0,nis-1);
		}
		break;
	case SAMPLE: 
		//SAMPLE n,d
		// trigger level =n, max delay= d, (4ms per sample)
		{
			long n=4, d=2000; // defaults
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
					BasicErr=1;
			}
		}
		break;
	case SCALE: 
		{
			int i,s;
			p=line.text;
			if (eval_expr(&p, &n) != ARRAY)
			{
				BasicErr=1;
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
				BasicErr=1;
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
				rprintfProgStr (PSTR("Invalid sample size must be one of 8,16,32 or 64\n"));
				BasicErr=1;
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
			int ch=0,c=0;
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
		// GEN 4,16,5,2,0,254,0
		{
			int i, param[7],ty,sho,nog,ln,mr,mm;
			long t;
			p=line.text;
			for (i=0; i<7; i++)
			{
				eval_expr(&p, &t);
				param[i]=(int)t;
				if (*p==',' && i<6)
				{
					p++;
				}
				else if (*p !=0)
				{
					BasicErr=3; break;
				}
			}

			ty=param[6]&3; // 0-1 2
			sho=param[6]&8; // show flag
			nog=param[0];
			ln=param[1];
			mr=param[2];
			mm=param[3];

			if (mr>0 && mm==0)
			{
				rprintfProgStr(PSTR("Error - mutation range must be >0\n"));
				BasicErr=4; 	if (!remote) return 0;break;
			}

			if (sho)
			{
				rprintf("Type      = %d\n", ty);
				rprintf("length    = %d\n", ln);
				rprintf("Generate  = %d\n", nog);
				rprintf("Mut rate  = %d\n", mr);
				rprintf("Mut range = +/-%d\n", mm);
			}

			for (i=0; i<nog; i++)
			{
				int v=0,e;
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
						v = v + (rand()%(2*mm))-mm;
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
			int i, j, param[7],noi,noo,flg,sho,nl1,nl2,nl3,ofset,t;
			long t2;
#ifdef WIN32

			int l1o[20];
			int l2o[20];
			int l3o[20];
#endif
			p=line.text;
			for (i=0; i<7; i++)
			{
				param[i]=0;
				eval_expr(&p, &t2);
				param[i]=(int)t2;
				if (*p==0 && i>5) break;
				if (*p==',' && i<6)
				{
					p++;
				}
				else if (*p !=0)
				{
					BasicErr=3; break;
				}
			}
			// code here
			noi=param[0];
			noo=param[1];
			flg=param[2];
			sho=0;
			nl1=param[3];
			nl2=param[4];
			nl3=param[5];
	        ofset=param[6];

	        sho = (flg&8); // show output
	        flg = flg & 7; // 0, 1, 2, 3  or 4 (sigmoid mode)

			if (noi<=0 || noo<=0)
			{
				// number input & output >0
				rprintfProgStr(PSTR("Err:: Input and output must be gt 0\n"));
				BasicErr=3;
				break;
			}

			if (nl3!= noo && nl3!=0)
			{
				//layer 3 = number outputs
				rprintfProgStr(PSTR("Err:: layer 3 neurons must match output (or be zero)\n"));
				BasicErr=3;
				break;
			}

			if (nl3==0 && nl2!=0)
			{
				//layer 3 = number outputs
				rprintfProgStr(("Err:: layer 3 must be non-zero if layer 2 has nodes \n"));
				BasicErr=3;
				break;
			}

#ifndef WIN32

			int l1o[nl1];
			int l2o[nl2];
			int l3o[nl3];
#endif

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

			t=noi+noo+ofset-1; // index through weights and threshold

			for (i=0; i<nl1; i++)
			{
				int s=0;
				if (sho) rprintf("INPUT NEURON = %d\n", i+1);
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
					rprintf("Input=%d (%d x %d)\n", j,l1o[j],scene[t]);
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
	case EXTEND:
#ifdef AVR
		rprintfProgStr(PSTR("? Cmd not available\r\n"));
#else
		extend(line.text);
#endif
		break;
	default:
		BasicErr=8; // DEBUG
		tmp=0xCC;
	}		
	return tmp;
}

void basic_clear()
{
	// Set Init pointer to Zero
	rprintfProgStr(PSTR("Clear Program \r\n"));
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
		rprintfCRLF ();	
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
		rprintfCRLF ();	
	}
}

extern const  prog_char  *specials[];


void basic_list()
{
	line_t line;
	unsigned char buf[MAX_LINE];

	BYTE tmp=0;
	nxtline = 0;	

	rprintfProgStr(PSTR("List Program \r\n"));

	if (nextchar() != 0xAA ) {
		rprintfProgStr(PSTR("No program loaded\r\n"));
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
		rprintfProgStr (tokens[line.token]);
		rprintf (" "); 
		
		if (line.token==LET || line.token==GET || line.token==FOR || line.token==LIST || line.token==DATA)
		{
			if (line.var!=32)
				rprintf ("%c = ", line.var+'A');
			else 
				rprintfProgStr (PSTR("! = "));
		}
			
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


		rprintfCRLF ();
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
	rprintfProgStr(PSTR("start download\r\n"));
	RUN_LED2_ON;
	
	r=bin_read_request();
			
	if (r == 'l')
		SendResponse('l', VERSION);
	else
		SendResponse('z', r);
		
	RUN_LED2_OFF;
}
#endif

void outdigit(int n,int c)
{
	char tb[6];
	int i;
	if (c>4) return;

	for (i=0; i<c; i++)
	{
		tb[c-i-1]=(n%10)+'0';
		n=n/10;
	}
	tb[c]='\0';
	rprintfStr(tb);
}

void uptime()
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
			rprintfProgStr(PSTR("reset\r\n"));
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

	rprintfProgStr(PSTR("Basic v=$Revision$\r\nCommands: i r l c z q s V R F $\r\n"));

	testforreset();

	init();

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
			    rprintfProgStr(PSTR("Elapsed Time "));
		        outdigit(m,2);
		        rprintfChar(':');
		        outdigit(s,2);
		        rprintfChar('-');
		        outdigit(ms,3);
		        rprintfCRLF();
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
		case 'L': // list 
			basic_list();
			listdump();
			showvars();
			break;
		case 'c': // clear 
			listinit();
			basic_clear();
			break;
		case 's': // stand
		case 22:  //*+A
			rprintf("Stand %d\r\n", nos);
			standup(nos);
			break;
		case 'S': // stand
			rprintfProgStr(PSTR("ok\r\n"));
			setdh(1);
			standup(nos);
			break;
		case 'o': // output lights
			sample_sound(1); // sound meter on
			break;
		case 'O': // 
			rprintfProgStr(PSTR("PSD off\r\n"));
			PSD_off();
			break;
		case 'X': // charge
			rprintfProgStr(PSTR("testing charge\r\n"));
			chargemode();
			break;
		case 'C': // zero &clear 
			basic_zero();
			break;
		case 'V': // version
			rprintfProgStr(PSTR("v=$Revision$\r\n"));
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
			if (dbg) rprintfProgStr(PSTR("DEBUG ON\n"));
			break;
		case 'q': // query 
			rprintf ("%d servos connected\r\n", readservos(0));
			rprintf ("%d lines stored\r\n", findend());
			rprintfProgStr(PSTR("Uptime: ")); uptime(); rprintfCRLF ();
			break;
		case '$':
		case 23: //*+1 Demo mode
			rprintfProgStr (PSTR("DEMO MODE\r\n"));
			while (1)
			{
				rprintf (">: ");
				ch = GetByte();
				rprintf ("%d\r\n",ch);

				if (ch == 7)
				{
					rprintfProgStr (PSTR("EXIT DEMO MODE\r\n"));
					break;
				}

				if (ch>=0 && ch<20)
				{
					PerformAction(map[ch]);
				}
			}
			break;
#ifndef AVR
		case 'Z': // upload  (Unix & windows version only)
			binstore();
			rprintf ("stored\r\n");
			break;
		case 'm': // matrix 
			matrixload(128);
			break;
		case 'M': // matrix 
			matrixstore(nis);
			break;
		case 't': // test
			testeval();
			break;
#endif
		default:
			rprintfCRLF();
			break;
		}
	}
}
