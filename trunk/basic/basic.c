/*

The ability to create simple actions in an elemntry basic language

basic_load		read a program from the serial port and store in EEPROM
basic_run		run the program in eeprom (option debuf flag for testing)
basic_clear		clear eeprom program(s)
basic_list 		show contents of eeprom

Language Spec:
VAR    A-Z  INTEGER
OPER   +-*\()=<>
CMD    LET:FOR:NEXT:GOTO:IF:THEN:ELSE:PRINT:END:SET
STRING " ... "
EXPR1  VAR | LITERAL
EXPR2  EXPR1 | STRING
LIST   EXPR2 [,EXPR2]
EXPR   EXPR1 OPER EXPR1  

SYNTAX:
[LINE no] LET  VAR '=' EXPR 
[LINE no] GOTO [Line No]
[LINE no] PRINT [#] LIST [;]
[LINE No] END
[LINE no] IF  EXPR THEN LINE no ELSE Line No
[LINE No] FOR VAR '=' EXPR 'To' EXPR
[LINE No] NEXT
[LINE No] XACT EXPR
[LINE No] WAIT EXPR
[Line No] GOSUB [Line No]
[Line No] RETURN
[Line No] SERVO VAR '=' EXPR | '@'
[LINE No] SCENE N, LIST(of N items)
[LINE No] MOVE LIST
--------------------- UNDER TEST -----------------------

[LINE No] PUT [Special] = [Expr]
[LINE No] LET A=$ROM:addr

--------------------------TBD --------------------------
[LINE No] POKE val, Address


Rbas Cmd		 Description
==============   ==============
XACT       		 Call any Experimental action using literal code i.e. XACT 0, would do basic pose, XACT 17 a wave
PUT PF1=1	   	 access to PORTS/SPECIAL REGISTER, This would set PF1_led1 on
SERVO ID=POS     set servo id to positon POS / @
LET A=$SERVO:id  let A get position of servo id 
SCENE            set up a Scene - 16 Servo Positions
MOVE             sends a loaded Scene  (time ms, no frames)


Special register access ($)
LET A=$IR  		 //get char from IR and transfer to A (also $ADC. $PSD, $X, $Y...)
LET A=$PORT:A:6  //Read Bit 6 of Port A
LET A=$ROM:10    // read byte 10 of ROM

POKE 10,A         // Put A into Byte 10
PUT PORT:A:8 = 3 //set DDR of Port A = 3 (PIN0,PIN1 readable)
PUT PORT:A:2 = 1 //set Port A bit 1 =1 (assuming writeable)

Example Programs are now available from examples folder
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

#define DPAUSE {rprintf(">");while (uartGetByte()<0); rprintf("\r\n");}

#define EEPROM_MEM_SZ 	1024
#define MAX_LINE  		100 
#define MAX_TOKEN 		8
#define MAX_FOR_NEST	3
#define MAX_GOSUB_NEST	3
#define MAX_FOR_EXPR	20 
#define MAX_DEPTH 		5

/***********************************/

#define C 		16
#define PGAIN 	(unsigned char)20
#define DGAIN 	(unsigned char)30
#define IGAIN 	(unsigned char)0
#define S 		(2+3*C)

/***********************************/

uint8_t EEMEM FIRMWARE[64];  					// leave blank - used by Robobuilder OS
uint8_t EEMEM BASIC_PROG_SPACE[EEPROM_MEM_SZ];  // this is where the tokenised code will be stored

extern void Perform_Action(BYTE action);
extern int getHex(int d);
extern int delay_ms(int d);
extern void SampleMotion(unsigned char); 

/***********************************/


extern  BYTE nos;

//wait for byte
int  GetByte()
{
	int b;
	
	RUN_LED1_OFF;
	while ((b=uartGetByte())<0) ;
	RUN_LED1_ON;
	
	return b;
}

const  prog_char *error_msgs[] = {
	"",
	"Syntax error",
	"Invalid Command",
	"Illegal var",
	"Bad number",
	"Next without for",
	};
	
enum {
	LET=0, FOR, IF, THEN, 
	ELSE, GOTO, PRINT, GET, 
	PUT, END, SCENE, XACT, 
	WAIT, NEXT, SERVO, MOVE,
	GOSUB, RETURN, POKE, STAND
	};

#define NOTOKENS 20

const prog_char *tokens[] ={
	"LET", "FOR", "IF","THEN", 
	"ELSE","GOTO","PRINT","GET",
	"PUT", "END", "SCENE", "XACT",
	"WAIT", "NEXT", "SERVO", "MOVE",
	"GOSUB", "RETURN", "POKE", "STAND"
};

char *specials[] = { "PF", "MIC", "X", "Y", "Z", "PSD", "VOLT", "IR", "KBD", "RND", "SERVO", "TICK", 
		"PORT", "ROM", "TYPE" };

enum { 	sPF1=0, sMIC, sGX, sGY, sGZ, sPSD, sVOLT, sIR, sKBD, sRND, sSERVO, sTICK, sPORT, sROM, sTYPE};
							

int errno;

void PerformAction (BYTE Action)
{	
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
	*/
		SampleMotion(Action); 
		//ptime(); 
		rprintf("Do Motion %x\r\n", Action);
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
		if (!(c>= 'A' && c <='Z') )  //must be alpha
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
	int ch;
	int qf=1;
	char *start=line;
	
	rprintfStr ("> ");
	
	while (1)
	{
		// foreach char entered

		while ((ch = uartGetByte())<0) ;
			
		rprintfChar(ch);	
				
		if (ch==13 || (start==line && ch=='.') )
		{
			if (ch=='.') *line++='.';
			*line='\0'; 
			rprintf("\r\n");	
			break;
		}
		
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

int execute(line_t line, int dbf);

void basic_load()
{	
	char line[MAX_LINE];
	int n=0;
	int lc=0;
	char *cp;
	
	uint16_t psize=0;
	
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
				errno=1;
				continue;
			}
			cp++;
		}

		if ( (newline.token=token_match((char **)tokens, &cp, NOTOKENS))==255)           // sizeof(tokens)/))<0)
		{
			errno=2;
			continue;
		}
			
		if (*cp == ' ') cp++;
		
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
			// read servo ID		
			if ((newline.var = getNum(&cp))<0)
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
		case PRINT:
		    if (*cp=='#') { newline.var=1; cp++;}
		case MOVE:
		case SCENE:
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
			
			//rprintf("["); rprintfStr(newline.text); rprintf("]\r\n"); //debug
			}
			break;
		case STAND: 
		case WAIT: 
		case GOTO: 
		case XACT:
		case GOSUB:
			// read line
			newline.value = getNum(&cp);			
			//rprintf ("val=%d\r\n", newline.value); 		//DEBUG
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
				newline.text=forbuf[--fb];
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

int get_special(char *str, int *res)
{
	char *p=str;
	int t=token_match(specials, &str, sizeof(specials));
	int v=0;
	
	//if (t==sPSD || t==sVOLT || t==sMIC)
	//{
	//	adc_test(0);
	//}
	
	switch(t) {
	case sPF1:
		v=0;
		break;
	case sMIC:
		v=adc_mic();
		break;
	case sGX:
		tilt_read(0);
		v=x_value;
		break;	
	case sGY:
		tilt_read(0);
		v=y_value;
		break;		
	case sGZ:
		tilt_read(0);
		v=z_value;
		break;			
	case sPSD:
		v=adc_psd();
		break;
	case sVOLT:
		v=adc_volt();
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
	case sRND:
		v=rand();
		break;	
	case sSERVO: // SERVO:nn
		// get position of servo id=nn
		v=0;
		if (*str==':') {
			str++;
			v=getNum(&str);
			v = wckPosRead(v); 			// get pos of servo id=v
			*res=v;
			return (str-p);
		}
		break;	
	case sPORT: // PORT:A:n
		// get position of servo id=nn
		v=0;
		if (*str==':') {
			str++;
			}
		if (*str>='A' && *str<='G' ) {
			v= (*str-'A');
			str++;
			}	
		t=8;
		if (*str==':') {   // Optional Bit specficied
			str++;
			if (*str>='0' && *str<='7' ) {
				t=  (*str+ '0');
				str++;
				}
		}				
		*res=get_bit(v, t);         //need to read port with PINA etc 
		return (str-p); // not finished yet
	case sROM: // ROM(x)
		{
		int i=0; // x
		if (*str=='(') 
		{   //(Addr)
			str++;
			i=getNum(&str);
			if (*str==')')
			{
				v = eeprom_read_byte((uint8_t*)(FIRMWARE+i));
			}
		}
		}
		break;
	default:
		return -1;
	}
	*res=v;
	t=strlen(specials[t]);
	return t;
}

enum {STRING, NUMBER, ERROR, CONDITION } ;

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

/*

*/

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
		case '$':
			//special var?
			if ((tmp=get_special(*str, &n1))>0) 
			{
				*str += tmp;
			}
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
	if (p==0) return -1; // no such line

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

int forptr[MAX_FOR_NEST];   // Upto 3 nested for/next
int fp; 
int gosub[MAX_GOSUB_NEST];  // 3 nested gosubs
int gp;  
BYTE scene[32];
int nis=0;

int execute(line_t line, int dbf);
		
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
			rprintf("Runtime error %d\r\n", errno);
			return;
		}

		line = readln(buf);
		
		tc = nextchar();	// terminator character ?
		
		if (uartGetByte() == 27)  {
			rprintf ("User Break on line %d\r\n", line.lineno); 
			return;
		}

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

	int tm;
	int fm;
	uint8_t tmp=0;
	char *p;		
	
	int n;
	
	if (dbf) rprintf (": %d - ", line.lineno); // debug mode

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
				setline(t_ptr); tmp=0 ;
			}	
			else
			{
				fp--;
			}
		}
		break;
	case GET: 
		n=0;
		if (get_special(line.text, &n)<0)
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
		n=0;
		p=line.text;
		if (*p=='@') // set passive mode
		{
			// set pass servo id=line.var
			wckSetPassive(line.var);
		}
		else
		{
			eval_expr(&p, &n);
			if (n>00 && n<=254)
			{
				// set pos servo id=line.var, n
				// char SpeedLevel
				wckPosSend(line.var, 2, n);
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
			tmp=0;
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
				int t = gotoln(line.value);
				if (t<0)
					errno=5; 	
				else
					setline(t);
				tmp=0;		
			}
		}
		break;		
	case MOVE: 
		// No args - send servo positions syncronously
		// with args (No Frames / Time in Ms) - use MotionBuffer
		fm=0; tm=0;
		p=line.text;
		if (p!=0 && *p != 0)
		{
			eval_expr(&p, &fm);
			if (*p++ != ',') { errno=1; break;}
			eval_expr(&p, &tm);

			//playpose(fm,tm,scene);
			PlayPose(tm, fm, scene, nis);
		}
		else
		{
			if (nis>0) 
				wckSyncPosSend(nis-1, 4, scene, 0);
			else 
				errno=1;
		}
		//
		break;
	case SCENE: 
		// read in 'n' servo position values and store
		{
		int i;
		p=line.text;
		eval_expr(&p, &nis);
		if (*p++ != ',') { errno=6;break; }
		for (i=0;i<nis;i++)
		{
			n=0;
			eval_expr(&p, &n);
			if (i!=(nis-1) && *p++ != ',') { errno=6;break; }
			scene[i]=n;
		}
		}
		//
		break;
			
	case XACT: 
		PerformAction(line.value);
		break;
	case WAIT: 
		delay_ms(line.value);
		break;
	case END: 
		rprintfStr ("End of program\r\n"); 
		return 0xCC;
	case GOSUB: 
		//rprintfStr ("gosub\r\n");  //debug
		{
			int t;
			gosub[gp++]=nxtline;
			t=gotoln(line.value);
			if (t<0)
				return 0xCC;	// this needs an error message		
			setline(t);
			tmp=0;	
		}
		break;
	case RETURN: 
		//rprintfStr ("return\r\n"); //debug
		if (gp>0) {
			setline(gosub[--gp]);
			tmp=0;
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
	int i;	
	uint8_t data= 0xFF; 				// start of program byte	
	rprintfStr("Clear Program \r\n");
			
	for (i=0; i<EEPROM_MEM_SZ; i++) 	
	{
		eeprom_write_byte(BASIC_PROG_SPACE+i, data);
	}

	clearln();
	
	rprintf("Cleared %d bytes \r\n", EEPROM_MEM_SZ);
}


void dump()
{
	int i;
	
	for (i=0; i<EEPROM_MEM_SZ; i+=8) 	
	{
		int j;
		char asciis[9];
		rprintf ("%x ", i);
		for (j=0; j<8;  j++)
		{
			uint8_t data = eeprom_read_byte((uint8_t*)(BASIC_PROG_SPACE+i+j));
			rprintf ("%x ", data);
			if (data>27 && data<127) asciis[j]=data; else asciis[j]='.';
		}
		asciis[8]='\0';
		rprintfStr (asciis);
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

	while (tmp != 0xCC && nxtline < EEPROM_MEM_SZ )
	{
		line = readln(buf);
		tmp = nextchar();

		/* list code */
	
		rprintf ("%d ", line.lineno); 
		rprintfStr (tokens[line.token]);
		rprintf (" "); 
		
		if (line.token==LET || line.token==GET || line.token==FOR)
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

		if (line.token==SERVO)
			rprintf ("%d = ", line.var);
			
		if (line.token==PRINT && line.var==1)
			rprintf ("# ");
			
		if (line.token==NEXT) 
			rprintf ("%c", line.var+'A');
		else
		if (line.token==GOTO || line.token==WAIT ) 
			rprintf ("%d", line.value);
		else
			rprintfStr (line.text);
		rprintf ("\r\n");
	}
}

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
				return -1;
		}
	}
	return -1;
}

void binmode()
{
	int r;
	
	RUN_LED2_ON;

	r=bin_read_request();
			
	if (r == 'l')
		SendResponse('l', VERSION);
	else
		SendResponse('z', PROTOCOL_ERROR);
		
	RUN_LED2_OFF;
}

void basic()
{
	int ch;
	rprintfStr("Basic v=$Revision$\r\nCommands: i r l c z q\r\n");
	readservos();

	while (1)
	{
		rprintfStr(": ");
		ch = GetByte();			
		rprintfChar(ch);rprintfChar(10);rprintfChar(13);	
		switch (ch)
		{
		case 'i': // input
			basic_load();
			break;
		case 'r': // run
			basic_run(0);
			break;
		case 'l': // list 
			basic_list();
			break;
		case 'c': // clear 
			basic_clear();
			break;
		case 'd': // dump 
			dump();
			break;
		case 'z': // download 
			rprintfStr("start download\r\n");
			binmode(); // enter binary mode
			break;
		case 'q': // download 
			readservos();
			break;
		default:
			rprintfStr("??\r\n");
			break;
		}
	}
}

#ifndef AVR
main()
{
	basic();
}
#endif
