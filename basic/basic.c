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

extern int strlen	(char *);
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
#include <stdio.h>
#include <string.h>
#include "linux.h"
#endif

#include "edit.h"
#include "express.h"
#include "functions.h"
#include "lists.h"

#define MAX_LINE  		100 
#define MAX_TOKEN 		8

extern int dbg;

/***********************************/

BYTE EEMEM FIRMWARE        	[64];  		// leave blank - used by Robobuilder OS
BYTE EEMEM BASIC_PROG_SPACE	[EEPROM_MEM_SZ];  // this is where the tokenised code will be stored
BYTE EEMEM PERSIST		[256];                     // persistent data store

extern void Perform_Action	(BYTE action);
extern int  getHex		(int d);

extern void SampleMotion	(unsigned char); 
extern void sound_init		();
extern void SendToSoundIC	(BYTE cmd) ;

extern void  blights(int n, int *vals) ; // from adc.c

/***********************************/

extern int chargemode();

void basic_start(int dbf);

/***********************************/

void  lights(int n); 
extern BYTE cpos[];

extern void				sample_sound(int);
extern BYTE				nos;

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
	"SAMPLE","SCALE",  "DATA",   "SET", 	 
        "INSERT","DELETE", "GEN",    "NETWOR", 
  	"SELECT", "!", 	   "ON",     "MAT",
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

#ifdef LINUX
#define HB 10
	char histbuff[HB][MAX_LINE];
	int hbfc=0;
#endif


int readflg=1;

int readLine(char *line)
{
	int ch=0,pch;
	int qf=1;
	int cf=0;
	int lf=0;
	char *start=line;
	char *end=line;
#ifdef LINUX
	int hb2 = hbfc;
#endif
	
	rprintfStr ("> ");
	
	while (1)
	{
		// foreach char entered
		pch=ch;
		if (line>end) {end=line; *end=0;}

		while ((ch = uartGetByte())<0) ;

		if (ch=='\'' && readflg) // comment ignore rest of line
		{
			rprintfChar(ch);
			*line='\0'; 
			while (ch != 13)
			{
				while ((ch = uartGetByte())<0) ;
				rprintfChar(ch);
			}
			rprintf("\r\n");	
			break;		
		}

#ifdef LINUX
		if (ch==20) //recover last line ^U
		{
			if (hb2<=0) hb2=hbfc;
			if (hb2>0) 
			{
				int z, n= line-start;
				for (z=0;z<n;z++) {rprintfChar(8); rprintfChar(32); rprintfChar(8);}
				rprintf("%s", histbuff[hb2-1]);fflush(stdout);
				strcpy(start,histbuff[hb2-1]);
				line = start+strlen(histbuff[hb2-1]);
				hb2--;
			}
			continue;
		}

		if (ch==2) //back ^B
		{
			if (line>start) 
			{
				line--;
				rprintfChar(8);
			}
			continue;
		}

		if (ch==6) //forward ^F
		{
			if (line<end) 
			{
				rprintfChar(*line);
				line++;
			}
			continue;
		}
#else
		if (start==line && ch=='=') //recover last line
		{

			rprintfStr(line);
			while (*line++!=0);
			end=line;
			line--;
			continue;
		}
#endif

		if (lf==1)
		{
			rprintfChar(ch);
			if (ch>=100) { *line++ = (ch/100) +'0'; }
			ch=ch%100;
			*line++ = (ch/10) +'0'; 
			*line++ = (ch%10) +'0';
			lf=0;
			continue;
		}

		if (ch=='`') {lf=1;rprintfChar(ch);continue;}

		if (ch==9) ch=' ';

		if (ch=='~' && start==line)
		{
			dbg=(++dbg)%4;
			rprintfStr ("DEBUG ");
			if (dbg) rprintfProgStr(PSTR("ON")); else rprintfProgStr(PSTR("OFF"));
			rprintfStr ("\n> ");
			continue;
		}
				
		if (ch==13 || (start==line && ch=='.') )
		{
			if (ch=='.') { *end++='.';*end='\0';}
			if (pch==' ') line--; //get rid of trailling space
			//*line='\0'; 
			rprintf("\r\n");	
			break;
		}
#ifdef LINUX
		if (ch==18 && readflg) //^R
		{
			readservos(0);
			if (nos>0) {
				char buf[80];
				sprintf(buf, "MOVE @{%d,%d", nos,cpos[0]); 
				for (int i=1; i<nos; i++) 
					sprintf(buf+strlen(buf),",%d",cpos[i]); 
				sprintf(buf+strlen(buf),"}"); 
				rprintfStr(buf);
				strcpy(line,buf);
				line += strlen(buf);
				*line='\0';
				end=line;
				continue;
			}
		}
#endif

		if (qf && ch==' ' && pch==' ')
			continue;
		
		if (start==line && ch=='?' )
		{
			rprintfProgStr(PSTR("Input mode (. to exit)\r\n"));	
			break;
		}
		
		if (ch=='"') {qf=!qf;}
	
		if (ch >= 'a' && ch <= 'z' && qf) ch = ch-'a'+'A';  // Uppercase only
		
		if (ch==8 || ch==127) //Bsapce ?
		{
			if (line>start && line==end) 
			{
				line--; end--;
				rprintfChar(8);
				rprintfChar(32);
				rprintfChar(8);
				if (*line=='"') qf=!qf;
				*line='\0';
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
#ifdef LINUX
	if (strlen(start)>0)
	{
		strcpy(histbuff[hbfc%HB],start);
		hbfc=(hbfc+1)%HB;
	}
#endif
	return (end-start);
}

int execute(line_t line, int f);
int fnptr[26];
extern WORD psize;

/*************************************************************************************************************

INPUT MODE
 Repeat
   Read a line
   If a '.' stop
   Convert into tokens
   Store in Eprom
 Loop

*************************************************************************************************************/

void basic_load(int tf)
{	
	char line[MAX_LINE];
	int n=0;
	int lc=0;
	char *cp;
					
	struct basic_line newline;	
	
	BasicErr=0;

	rprintfProgStr(PSTR("Input mode '.' to exit\r\n"));
	
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
		}
		else if (*cp=='D' && *(cp+1)=='I' && *(cp+2)=='M')
		{
			newline.token=MATRIX;
		}
		else if ( *cp=='@' || (*cp >= 'A' && *cp <= 'Z' && ( *(cp+1)=='=' || *(cp+1)=='[')) ) {
			newline.token=LET;
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
   			if (*cp=='@' || (*cp>='A' && *cp<='Z' && *(cp+1)=='['))
			{
				newline.var='@';
				newline.text=cp;
				break;
			}
			else
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
				if (to==0) to = strstr(cp," IN ");
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
		case MATRIX:
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
		case ON:
			{
				char *p;
				newline.text=cp;
				p=strstr(cp, " GOSUB");
				if (p !=0)
				{
					*p++=',';

					while(1) {
					   *p=*(p+6);
					   p++;
					   if (*p==0) break;
					}
				}
				else
					BasicErr=3;
			}
			break;
		case PLAY: 
		case GOTO: 
		case GOSUB:
			newline.value = getNum(&cp);	// read line	
			newline.text=cp;
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

RUN

*************************************************************************************************************/
		
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

extern int  timer;
extern int  tline;
extern int  kline;

#ifdef IMAGE
extern int  imready;
extern int  imline;

extern int pbrunning;
extern int pbstep;
extern int pbtime;

extern void pb2();
#endif

extern void push_line(unsigned int n);
extern int  gotoln(int gl);
extern int  gevent;
extern int  gkp;

int breakp=0;

void basic_start(int dbf)
{
	// Repeat
	//   load from memory
	//   Execute action
	// Loop
	char buf[MAX_LINE];
	line_t line;
	int key;	
	int irf=0;
	BYTE tmp=0;

	gtick=0;
	timer=0;
	tline=0;
	kline=0;
#ifdef IMAGE
	imline=0;
#endif
	init_cmdptr();

	while (tmp != 0xCC && nxtline < EEPROM_MEM_SZ )
	{	
		if (BasicErr !=0)
		{
			rprintf("Runtime error %d on line %d\r\n", BasicErr, line.lineno);
			return;
		}


 		if (gkp==27 || (key=uartGetByte()) == 27)  {
			rprintf ("User Break on line %d\r\n", line.lineno); 
			gkp=0;
			return;
		}

		if (key>=0) gkp=key;

		if (irf==0 && kline>0 && key>=0)
		{
			int t;
			//printf("trig %d %d\n", key, kline);
			push_line(nxtline);

			t=gotoln(kline);
			if (t<0)
			{
				BasicErr=6; //invalid line number
				return;
			}
			setline(t);
			gevent=key;
			irf=1;
		}

#ifdef IMAGE
		if (pbrunning==1  && gtick>pbtime)
		{
			pb2();
		}

		if (imline>0 && imready==1)
		{
			int t;
			//printf("image trig %d\n", imline);
			push_line(nxtline);

			t=gotoln(imline);
			if (t<0)
			{
				BasicErr=6; //invalid line number
				return;
			}
			setline(t);

			imready=0;
		}
#endif

		if (irf==0 && timer >0 && gtick>timer)
		{
			//printf("trig %d %d\n", timer, tline);
			int t;
			push_line(nxtline);

			t=gotoln(tline);
			if (t<0)
			{
				BasicErr=6; //invalid line number
				return;
			}
			setline(t);
			gtick=0;
			irf=1;
		}

		line = readln(buf);
		if (line.lineno==0)
		{
			rprintfProgStr(PSTR("End\r\n"));
			return;
		}

		if (line.lineno==breakp)
		{
			rprintfProgStr (PSTR(" ** Break\r\n")); 
			showvars();
			return;
		}
		
		if (dbf) 
		{
			rprintf ("TRACE :: %d - ", line.lineno); 
			rprintfProgStr (tokens[line.token]);

			if (key<0)
			while ((key=uartGetByte())<0) ; // wait for input

			if (key == 27)  
			{
				rprintfProgStr (PSTR(" ** User Break\r\n")); 
				return;
			}

			rprintfCRLF();

			if (key == 'l')  
			{
				listdump();
			}
			else
			if (key == 'v')  
			{
				showvars();
			}
		}

		/* execute code */

 		tmp = execute(line, dbf);
		
		if (dbf && (line.token==LET || line.token==GET || line.token==FOR || line.token==SERVO || line.token==NEXT ))
		{
				rprintf ("%c = %ld\r\n", line.var+'A', getvar(line.var));
		}
		if (tmp == 0) tmp = nextchar();	// terminator character ?

		if (irf=1 && line.token==RETURN) irf=0; // clear it
	}
}


/*************************************************************************************************************

CLEAR / ZERO / DUMP commands

*************************************************************************************************************/

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
		rprintf ("%02x ", i);
		for (j=0; j<sz;  j++)
		{
			BYTE data = eeprom_read_byte((BYTE*)(BASIC_PROG_SPACE+i+j));
			rprintf ("%02x ", data);
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
		rprintf ("%02x ", i);
		for (j=0; j<8;  j++)
		{
			BYTE data = eeprom_read_byte((BYTE*)(FIRMWARE+i+j));
			rprintf ("%02x ", data);
			if (data>27 && data<127) asciis[j]=data; else asciis[j]='.';
		}
		asciis[8]='\0';
		rprintfStr (asciis);
		rprintfCRLF ();	
	}
}

/*************************************************************************************************************

LIST

*************************************************************************************************************/

extern const  prog_char  *specials[];

void basic_list()
{
	line_t line;
	unsigned char buf[MAX_LINE];

	BYTE tmp=0;
	nxtline = 0;	

	rprintfProgStr(PSTR("List Program\r\n"));

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

		if (line.lineno==breakp) rprintfChar('*');
	
		rprintf ("%d ", line.lineno); 
		if (line.token==MATRIX && !strncmp(line.text,"DIM",3))
		{
			//rprintfStr (line.text);
		}
		else
		{
			rprintfProgStr (tokens[line.token]);
			rprintf (" "); 
		}
		
		if (line.token==LET || line.token==GET || line.token==FOR || line.token==LIST || line.token==DATA)
		{
			if (line.var=='@')
				;
			else if (line.var!=32)
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
			rprintf (" THEN ");
			rprintfStr (p_then+1);
			if (!(*(p_else+1)=='0' && *(p_else+2)==0))
			{
				rprintf (" ELSE ");
				rprintfStr (p_else+1);
			}
		}
		else if (line.token==ON)
		{
			char *cp2, *p=0, *cp = line.text;
			int l = strlen(cp);
			cp2=cp+l;
			while (cp2>cp)
			{
				if (*cp2==',')
				{
					p=cp2;
					*p='\0';
				}
				cp2--;
			}
			rprintfStr (cp);
			rprintf (" GOSUB ");
			rprintfStr (p+1);
		}
		else if (line.token==NEXT) 
		{
			rprintf ("%c", line.var+'A');
		}
		else if (line.token==DATA)
		{
			unsigned char i;
			unsigned char n=(unsigned char)(*(line.text+1));
			rprintf ("%d", (unsigned char)line.text[2]);
			for (i=1; i<n; i++)
				rprintf (",%d", (unsigned char)line.text[2+i]);
		}
		else if (line.token==FOR)
		{
			rprintfStr (line.text);
		}
		else if (line.token==GOTO || line.token==GOSUB || line.token==PLAY  ) 
			rprintf ("%d", line.value);
		else
			rprintfStr (line.text);


		rprintfCRLF ();
	}
}

/*************************************************************************************************************

DOWNLOAD BINARY CODE FROM PC

*************************************************************************************************************/

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


void uptime()
{
	rprintf("%02d:%02d:%02d-%03d",gHOUR,gMIN,gSEC,gMSEC);
}

void testforreset()
{

	if((PINA&03) == 0)  // PF1 on, PF2 on 
	{
		delay_ms(10);
		if((PINA&3) == 0) // after 1s still PF1 on, PF2 on 
		{
			rprintfProgStr(PSTR("reset\r\n"));
			basic_clear();
		}
	}
}

extern int base;

void init()
{
	int i;
	for (i=0; i<16; i++) offset[i]=0;

	base=0; // default base
}

extern int nis;
extern int dcmp_mode() ;

/*************************************************************************************************************

COMMAND LINE

*************************************************************************************************************/

#ifndef VER
#define VER "Basic v=$Revision$\r\nCommands: i r l c z q s V R F $\r\n"
#endif

void basic()
{
	int ch,i;

	rprintfProgStr(PSTR(VER));

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
			if (!(ch== 'G'|| ch=='g' || ch=='B')) rprintfCRLF();
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

				rprintf("Elapsed Time %02d:%02d-%03d\n",m,s,ms);
		    }
			break;
		case 'R': // run
			basic_run(1);
			break;
		case 'G': // gosub
		case 'g': // gosub
		case 'B': // set breakpoint
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
			if (ch=='B')
			{
				breakp=gn;
				rprintf("BP=%d\n",gn);
				break;
			}
			if (gn==0) {gn=breakp; breakp=0;} // continue
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
			dbg=(++dbg)%4;
			if (dbg) rprintfProgStr(PSTR("DEBUG ON\n")); else rprintfProgStr(PSTR("DEBUG OFF\n"));
			break;
		case 'Q':
		case 'q': // query 
			rprintf ("%d servos connected\r\n", readservos(0));
			rprintf ("%d lines stored\r\n", findend());
			rprintfProgStr(PSTR("Uptime: ")); uptime(); rprintfCRLF ();
			if (ch=='Q' && nos>0) {
				rprintf("MOVE @{%d,%d", nos,cpos[0]); 
				for (i=1; i<nos; i++) rprintf(",%d",cpos[i]); 
				rprintfProgStr(PSTR("},10,1000"));
				rprintfCRLF();
			}
			break;
		case 'y':
			// customise stand up
			readservos(0);
			rprintf("Init Stand %d\r\n", nos);
			initbpfromcpos();
			rprintf("LIST %d", nos); 
			for (i=0; i<nos; i++) rprintf(",%d",cpos[i]); 
			rprintfCRLF();
			standup(nos);
			break;
		case 'Y':
			rprintf("Reset Stand %d\r\n", nos);
			initbp(nos);
			//
		case 'p': // passive 
			passiveservos(nos);
			break;
		case '$':
		case 23: //*+1 Demo mode
			rprintfProgStr (PSTR("DEMO MODE\r\n"));
			while (1)
			{
				rprintfProgStr (PSTR(">: "));
				ch = GetByte();
				rprintf ("%d\r\n",ch);

				if (ch == 7)
				{
					rprintfProgStr (PSTR("EXIT DEMO MODE\r\n"));
					break;
				}
				if (ch>=0 && ch<20)
				{
					PlayMotion(map[ch]);
				}
			}
			break;
#ifdef AVR
		case 'M':
			dcmp_mode();
			break;
#endif
#ifndef AVR
		case 'Z': // upload  (Unix & windows version only)
			binstore();
			rprintf ("stored\r\n");
			break;
		case 'm': // matrix 
			matrixload(128,"data.txt");
			break;
		case 'M': // matrix 
			matrixstore(nis,"data.txt");
			break;
		case '!': // go lisp 
			readflg=0;
			repl();
			readflg=1;
			break;
#endif
		default:
			rprintfCRLF();
			break;
		}
	}
}
