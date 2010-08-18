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
#else
#include "win.h"
#endif

// Standard Input/Output functions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Main.h"
#include "global.h"
#include "Macro.h"

#include "adc.h"
#include "ir.h"
#include "accelerometer.h"
#include "edit.h"

#define MAX_LINE  		150 
#define MAX_TOKEN 		8
#define MAX_FOR_NEST	3
#define MAX_GOSUB_NEST	3
#define MAX_FOR_EXPR	20 
#define MAX_DEPTH 		5

/***********************************/

uint8_t EEMEM FIRMWARE        [64];  			// leave blank - used by Robobuilder OS
uint8_t EEMEM BASIC_PROG_SPACE[EEPROM_MEM_SZ];  // this is where the tokenised code will be stored

extern void Perform_Action	(BYTE action);
extern int	getHex			(int d);
extern int	delay_ms		(int d);
extern void SampleMotion	(unsigned char); 
extern void sound_init		();
extern void SendToSoundIC	(BYTE cmd) ;

/***********************************/

extern BYTE				sData[];
extern int 				sDcnt;
extern void				sample_sound(int);
extern BYTE				nos;
extern volatile BYTE	MIC_SAMPLING;
extern volatile WORD	gtick;

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
	
enum {
	LET=0, FOR, IF, THEN, 
	ELSE, GOTO, PRINT, GET, 
	PUT, END, LIST, XACT, 
	WAIT, NEXT, SERVO, MOVE,
	GOSUB, RETURN, POKE, STAND,
	PLAY, OUT, OFFSET, RUN, I2CO, I2CI,
	STEP, SPEED, MTYPE, LIGHTS
	};

#define NOTOKENS 30

const prog_char *tokens[] ={
	"LET", "FOR", "IF","THEN", 
	"ELSE","GOTO","PRINT","GET",
	"PUT", "END", "LIST", "XACT",
	"WAIT", "NEXT", "SERVO", "MOVE",
	"GOSUB", "RETURN", "POKE", "STAND",
	"PLAY", "OUT", "OFFSET", "RUN",
	"I2CO", "I2CI", "STEP", "SPEED", 
	"MTYPE", "LIGHTS"
};

#define NOSPECS 20

char *specials[] = { "PF", "MIC", "X", "Y", "Z", "PSD", "VOLT", "IR", "KBD", "RND", "SERVO", "TICK", 
		"PORT", "ROM", "TYPE", "ABS", "MAPIR", "KIR", "FIND", "CVB2I" };

enum { 	sPF1=0, sMIC, sGX, sGY, sGZ, sPSD, sVOLT, sIR, 
		sKBD, sRND, sSERVO, sTICK, sPORT, sROM, sTYPE, sABS,
		sIR2ACT, sKIR, sFIND, sCVB2I};
							

int errno;
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
	char *start=line;
	
	rprintfStr ("> ");
	
	while (1)
	{
		// foreach char entered
		pch=ch;

		while ((ch = uartGetByte())<0) ;
			
		rprintfChar(ch);	
				
		if (ch==13 || (start==line && ch=='.') )
		{
			if (ch=='.') *line++='.';
			if (pch==' ') line--; //get rid of trailling space
			*line='\0'; 
			rprintf("\r\n");	
			break;
		}

		if (qf & ch==' ' && pch==' ')
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
			if (line>start) line--;
		}
		else
			*line++ = ch;
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
extern uint16_t psize;

void basic_load()
{	
	char line[MAX_LINE];
	int n=0;
	int lc=0;
	char *cp;

	char forbuf[MAX_FOR_NEST][MAX_FOR_EXPR];
	int fb=0;
					
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
		case GET: 
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
				else
				{
					*to='\0'; // null terminate expr1
					newline.text=cp;
					strcpy(forbuf[fb++],to+4); // store expr2
				}
			}
			else
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
		    if (*cp=='#') { newline.var=1; cp++;}
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
			newline.text=cp;
			break;
		case LIST:
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
			newline.text=cp;
			break;
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
		case PLAY: 
		case STAND: 
		case WAIT: 
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
			if (fb>0) {
				newline.text=forbuf[--fb];  //
				fnptr[newline.var]=psize;   //line pointer
			} else {
				// next without FOR
				errno=5;
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
				fmflg=0;
				execute(newline,0); // immediate execute
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

int variable[26]; // only A-Z at moment
int scene[32];	  // generic array
int nis=0;        // number of item isn array
int  eval_list(char *p);
unsigned char eval_expr(char **str, int *res);
enum {STRING, NUMBER, ARRAY, ERROR, CONDITION } ;
int speed=2;
int mtype=2;

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
	
	volatile uint8_t *port;
	uint8_t mask;
	
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

unsigned char map[] = {0, 7, 6, 4, 8, 5, 2, 17, 3, 0, 9, 1, 10, 11, 12, 13, 14, 15, 16, 18, 19};

int get_special(char **str, int *res)
{
	char *p=*str;
	int t=token_match(specials, str, NOSPECS);
	int v=0;
	
	switch(t) {
	case sPF1:
		v=0;
		break;
	case sMIC:
		MIC_SAMPLING=1; // on by default, but make sure
		{
		int lc;
		for (lc=0; lc<SDATASZ; lc++) 
		{
			v += sData[lc];  // sum the buffer
			sData[lc]=0;     // and clear
		}
		}
		break;
	case sTICK:
		v=gtick;
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
	case sPSD:
		Get_AD_PSD();
		v = gDistance;
		break;
	case sVOLT:
		Get_VOLTAGE();
		v = gVOLTAGE;
		break;
	case sTYPE:
		v=nos;
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
	case sRND:
		v=rand();
		break;
	case sIR2ACT: //$IR2ACT(10) -> x
		v=0;
		if (getArg(str,&v))
		{
			v=map[v];
		}
		break;
	case sABS: // $ABS(x)
		v=0; 
		if (getArg(str,&v))
		{
			v = v<0?-v:v;
		}
		break;
	case sCVB2I: // $CVB2I(x)  255 -> -1
		v=0; 
		if (getArg(str,&v))
		{
			v = cbyte(v);
		}
		break;
	case sROM: // ROM(x)
		if (getArg(str,&v))
		{
			v = eeprom_read_byte((uint8_t*)(FIRMWARE+v));
		}
		break;
	case sSERVO: // SERVO(nn)
		// get position of servo id=nn
		if (getArg(str,&v))
		{
			v = wckPosRead(v);
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
				t=  (**str + '0');
				(*str)++;
				}
		}				
		*res=get_bit(v, t);         //need to read port with PINA etc 
		return (*str-p); // not finished yet
	default:
		return -1;
	}
	*res=v;
	t=strlen(specials[t]);
	return t;
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
				for (i=0;i<32; i++)
				{
					scene[i] = offset[i];
				}
				nis=32;
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
            {
				char tmpA[100];
				n1 = **str-'A';
				if (n1<0 || n1 >25)
				{
					break;
				}
				n1 = variable[n1];
				readtext(n1, tmpA);				
				eval_list(tmpA);
				//
				(*str)++;
			}
			if (**str == '[')
			{
				(*str)++;
				eval_expr(str, &tmp);
				n1 = scene[tmp];
				(*str)++;
				break;
			}
			if (**str == '+' || **str == '-')
			{
				//add array
				int i,tnis;
				int tempB[32];
				char o = **str;
				(*str)++;
				for (i=0;i<32; i++)
				{
					tempB[i]=scene[i];
				}
				tnis=nis;
				if (eval_expr(str,res)==ARRAY)
				{
					if (tnis>nis) nis=tnis;
					for (i=0;i<32; i++)
					{
						if (o == '+') scene[i] = tempB[i] + scene[i]; else scene[i] = tempB[i] - scene[i];
					}
				}
			}
			return ARRAY;
			break;
		case '$':
			//special var?
			get_special(str, &n1);
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
	int p = findln(gl);
	if (p<3 || getlineno(p) != gl ) return -1; // no such line
	return p;
}


int put_special(int var, int n)
{
	if (var>= 30)
	{
		char a,b;
		a='A' + (var-30)/10;
		b='0' + (var % 10);
		set_bit((var-30)/10, (var % 10), n);
	}
	else
		switch(var) {
		case sPF1:
			if (n) PF1_LED1_ON; else PF1_LED1_OFF;
			break;
		case sPSD:
			if (n) PSD_ON; else PSD_OFF;
			break;
		default:
			rprintf ("? special %d is read only\r\n", var);
			break;
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

int forptr[MAX_FOR_NEST];   // Upto 3 nested for/next
int fp; 
int gosub[MAX_GOSUB_NEST];  // 3 nested gosubs
int gp;  
		
void basic_run(int dbf)
{
	// Repeat
	//   load from memory
	//   Execute action
	// Loop

	uint8_t tmp=0;	
	line_t line;
	char buf[MAX_LINE];
	
	fp=0; // Upto 3 nested for/next
	gp=0;  // 3 nested gosubs
	
	fmflg=0;
	
	nxtline=0;

	if (nextchar()!=0xAA) {
		rprintfStr("No program loaded\r\n");
		return;
	}
	
	//point top at EEPROM
	rprintfStr("Run Program \r\n");	
	errno=0;
	firstline();
	
	while (tmp != 0xCC && nxtline < EEPROM_MEM_SZ )
	{
		int tc;
		if (errno !=0)
		{
			rprintf("Runtime error %d on line %d\r\n", errno, line.lineno);
			return;
		}

		line = readln(buf);
		
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
		if (tmp == 0) tmp=tc;
	}
}

int execute(line_t line, int dbf)
{
	//   Get token
	//   Execute action
	//	 Move to next line
	uint8_t tmp=0;
	char *p;			
	int n;

	switch (line.token)
	{
	case FOR: 
		// remember where next in struction is
 		forptr[fp++] = nxtline;
		// eval expr1 of line.text = "expr1 TO expr2"
		// i.e var=expr1
		n=0;
		p=line.text;
		if (eval_expr(&p, &n)==NUMBER)
			variable[line.var] = n;					
		break;
	case NEXT: 
		// increment var and check condiiton
		if (fp>0) {
			int t_ptr=forptr[fp-1];
			// increment var
			variable[line.var]++;
			// test against expr2 i.e var<=expr2
			n=0;
			p=line.text;
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
	case GET: 
		n=0;
		if (get_special(&(line.text), &n)<0)
			errno=2;
		variable[line.var]=n;	
		break;
	case POKE:
 		n=0;
		p=line.text;
		if (eval_expr(&p, &n)==NUMBER)
		{
			// put result into address line.var
			uint8_t addr=line.var;
			eeprom_write_byte(FIRMWARE+addr, n);
		}
		else
			errno=1;
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
		standup(line.value);
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
					rprintf ("%d", scene[0]);
					for (n=1; n<nis; n++) {rprintf (",%d",scene[n]);}
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
			if (n != 0)
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
		// MOVE @A,500,10
		// MOVE @A
		// No args - send servo positions syncronously
		// with args (No Frames / Time in Ms) - use MotionBuffer
		p=line.text;
			
		if (p!=0 && *p != 0)
		{	
			int j, fm=0, tm=0;
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
				if (j<16) scene[j] += offset[j];

			}
			{
				BYTE pos[32];
				for (j=0; j<nis; j++)
				{
					if (scene[j]>=0 && scene[j]<=254) 
						pos[j] = scene[j];
					if (scene[j]<0) 
						pos[j] = 0;
					if (scene[j]>254) 
						pos[j] = 254;
				}
				PlayPose(tm, fm, 4, pos, (fmflg==0)?nis:0);
			}
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
		variable[line.var] = lastline+8;
		break;		
	case XACT:	case RUN:			
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
		delay_ms(line.value);
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
				for (i=0; i<32; i++)
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
				for (i=16; i<32; i++)
				{
					offset[i]=0;
				}
			}
			else if (eval_expr(&p, &n)==ARRAY)
			{
				for (i=0; i<nis; i++)
				{
					offset[i]=scene[i];
				}
				for (i=nis; i<32; i++)
				{
					offset[i]=0;
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
		if (eval_expr(&p, &n)==NUMBER)
		{
			if (*p==',')
			{
				p++;
				eval_expr(&p, &l);
			}
			while (l-->0) rprintfChar(n);
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
		MIC_SAMPLING=0;
		lights(n);
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
	uint8_t data= 0xFF; 				// start of program byte	
			
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
			uint8_t data = eeprom_read_byte((uint8_t*)(BASIC_PROG_SPACE+i+j));
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
			uint8_t data = eeprom_read_byte((uint8_t*)(FIRMWARE+i+j));
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

	uint8_t tmp=0;
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
		
		if (line.token==LET || line.token==GET || line.token==FOR || line.token==LIST)
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
			
		if (line.token==IF)
		{
			// replace ? THEN,  : ELSE (unless :0)
			char *p_then, *p_else, *cp = line.text;
			int l = strlen(cp);
			if (l>2 && cp[l-2]==':' && cp[l-1]=='0')
			{
				cp[l-2]=' ';  cp[l-1]=' ';
			}
			p_then = strstr(cp, ")?  ");
			p_else = strstr(cp, ":   ");
			if (p_then != 0) 
				strncpy(p_then, "THEN",4);
				
			if (p_else != 0) 
				strncpy(p_else,"ELSE",4);

			rprintfStr (cp+1);
		}
		else
		if (line.token==NEXT) 
		{
			rprintf ("%c", line.var+'A');
		}
		else
		if (line.token==GOTO || line.token==GOSUB || line.token==WAIT  || line.token==STAND  || line.token==PLAY  ) 
			rprintf ("%d", line.value);
		else
			rprintfStr (line.text);

		if (line.token==FOR)
		{
			char temp[100];

			//last bit is stored with next !
			//printf (" (%d) ", fnptr[line.var]);
			int p = findnext(nxtline, line.var);
			readtext(p+8, temp);
			rprintfStr (" TO ");
			rprintfStr(temp);
		}
		rprintf ("\r\n");
	}
}

#ifdef AVR
#define MAGIC_RESPONSE	0xEA
#define MAGIC_REQUEST	0xCD
#define VERSION			0x12     /* BIN API VERSION */
#define MAX_INP_BUF 	40
#define PROTOCOL_ERROR	01

void SendResponse(char mt, uint8_t d)
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

void basic()
{
	int ch;

	rprintfStr("Basic v=$Revision$\r\nCommands: i r l c z q\r\n");
	rprintf   ("%d servos connected\r\n", readservos());
	rprintf   ("%d lines in memory\r\n", findend());
	sound_init();	// sound output chip on (if available)
	sample_sound(1); // sound meter on



	while (1)
	{
		rprintfStr(": ");

		ch = GetByte();			
		if (ch > 26) {
			rprintfChar(ch);rprintfCRLF();
		}
		switch (ch)
		{
		case 'i': // input
			basic_load();
			break;
		case   7:  // IR red "stop" button
		case 'r': // run
			gtick=0;
			basic_run(0);
			rprintf("Elapsed Time %dms\r\n", gtick);
			break;
		case 'R': // run
			basic_run(1);
			break;
		case 'l': // list 
			basic_list();
			break;
		case 'c': // clear 
			basic_clear();
			break;
		case 'C': // zero &clear 
			basic_zero();
			break;
		case 'd': // dump 
			dump(8);
			break;
		case 'D': // dump 
			dump(24);
			break;
		case 'F': // dump 
			dump_firmware();
			break;
		case 'z': // download 
			basic_clear();
			binmode(); // enter binary mode
			rprintf ("%d lines loaded\r\n", findend());
			break;
		case 'q': // query 
			rprintf ("%d servos connected\r\n", readservos());
			rprintf ("%d lines stored\r\n", findend());
			break;
		default:
			rprintfStr("\r\n");
			break;
		}
	}
}
