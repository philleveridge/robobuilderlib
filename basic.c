/*

The ability to create simple actions in an elemntry basic language

basic_load		read a program from the serial port and store in EEPROM
basic_run		run the program in eeprom
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
[LINE no] PRINT LIST [;]
[LINE No] END
[LINE no] IF  EXPR THEN LINE no ELSE Line No
[LINE No] FOR VAR '=' EXPR 'To' EXPR
[LINE No] NEXT
[LINE No] PUT VAR '=' PORTA|B|C:0-7
[LINE No] SCENE LIST
[LINE No] XACT EXPR

Special Commands
PUT 	   enable access to PORTS/SPECIAL REGISTER, IR, ADC, PSD etc ...
SCENE      sends a Scene - 16 Servo Positions, Plus time, no frames
XACT       Call any Experimental action using literal code i.e. XACT 0, would do basic pose, XACT 17 a wave

Alt access (TBD)
LET A=$IR  //get char from IR and transfer to A (also $ADC. $PSD, $X, $Y...)

Example Programs
================

a) simple loop
10 LET A=1
20 PRINT A
30 LET A=A+1
35 WAIT 500
40 IF A<10 THEN 20 
50 END

b) read from console and IR port
10 LET A = $KBD
20 PRINT A
30 LET A = $IR
40 PRINT A
50 GOTO 10

c) Loops
10 FOR A=1 to 5
20 PRINT A
30 NEXT A

d) compound PRINT
10 LET A=(5+3)*(2+1)
20 PRINT "The answer is ";A
30 END

*/

#include <avr/io.h>
#include <avr/eeprom.h> 


// Standard Input/Output functions
#include <stdio.h>

#include <string.h>
#include "Main.h"

//#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "global.h"
#include "Macro.h"
#include "Comm.h"
#include "math.h"
#include "uart.h"
#include "rprintf.h"
#include "majormodes.h"
#include "adc.h"
#include "accelerometer.h"

#include <util/delay.h>

#define EEPROM_MEM_SZ 	256
#define MAX_LINE  		80 
#define MAX_TOKEN 		6

uint8_t EEMEM BASIC_PROG_SPACE[EEPROM_MEM_SZ];  // this is where the tokenised code will be stored

extern void Perform_Action(BYTE action);

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
	WAIT, NEXT
	};
	
const prog_char *tokens[] ={
	"LET", "FOR", "IF","THEN", 
	"ELSE","GOTO","PRINT","GET",
	"PUT", "END", "SCENE", "XACT",
	"WAIT", "NEXT"
};
							
struct basic_line {
    int lineno;
	unsigned char token;
	unsigned char var;
	int value;
	char *text; // rest of line - unproceesed
};

int errno;

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
//

int getToken(char *str, char *tok)
{
	//rprintf("dubug1: ["); rprintfStr(str); rprintf("] \r\n"); 

	int n=0;
	char c;
	while (c=*str)
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
	char c;
	char *sp = *p_line;
	
	char buff[MAX_TOKEN];
	
	t=getToken(*p_line, buff);
	
	//rprintf("dubug2: ["); rprintfStr(buff); rprintf("] %d \r\n", t); 
	
	*p_line += t;

	for (t=0; t<n; t++)
	{
		if (!strcmp(buff, list[t]))
			break;
	}
	
	//rprintf("dubug: m=%d \r\n", t); 

	return (t==n)? -1 : t; // no_match : match
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
		
		if (ch=='"') {qf=!qf;}
			
		if (ch >= 'a' && ch <= 'z' && qf) ch = ch-'a'+'A';  // Uppercase only
		
		if (ch==8) //Bsapce
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
		
void basic_load()
{
	rprintfStr("Enter Program '.' to Finish\r\n");
	
	char line[MAX_LINE];
	int n=0;
	int lc=0;
	//int i=0;
	char *cp;
	
	errno=0;
	
	uint16_t psize=0;
	
	char forbuf[3][20];
	int fb=0;
	
	while (1)
	{
		// for each line entered
		cp=line;	

		if (errno != 0) {
			rprintf ("Error - '" );
			rprintfStr (error_msgs[errno]);
			rprintf ("'\r\n" );
			rprintf ("Pos=%d\r\n", (cp-line));
			errno=0;
		}
			
		n=readLine(line);

		if ( line[0] =='.' && n==1)
		{
			rprintf("\r\n%d lines entered\r\n", lc);
			return;
		}
		
		if (n==0) continue; //ignore blank lines
		
		// convert to token
		
		struct basic_line newline;	
		
		newline.var=0;
		newline.value=0;
		newline.text=0;
			
		newline.lineno = getNum(&cp) ;
		
		if (cp>line+n || *cp  !=' ') 
		{
			errno=1;
			continue;
		}
		cp++;
		if ( (newline.token=token_match(tokens, &cp, sizeof(tokens)))<0)
		{
			errno=2;
			continue;
		}
			
		cp++;
		
		switch (newline.token)
		{
		case LET: 
		case GET: 
		case PUT: 
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

		case PRINT:
			newline.text=cp;
			break;
		case IF:
			// We should check for THEN and ELSE
			// and handle error
			// replace THEN with ? and ELSE with :
			// IF A THEN B ELSE C =>  GOTO (A)?B:C
			newline.text=cp-1;
			*(cp-1)='(';
			char *p_then = strstr(cp, "THEN");
			char *p_else = strstr(cp, "ELSE");
			if (p_then != 0) 
				strncpy(p_then, ")?  ",4);
			else
				errno=3;
				
			if (p_else != 0) 
				strncpy(p_else,":   ",4);
			else
				strcat(cp,":0");
			
			//rprintf("["); rprintfStr(newline.text); rprintf("]\r\n"); //debug
			break;
		case WAIT: 
		case GOTO: 
		case XACT:
			// read line
			newline.value = getNum(&cp);			
			//rprintf ("val=%d\r\n", newline.value); 		//DEBUG
			break;
		case END:
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
		
		// store struct in eeprom
		
		if (errno==0)
		{	
			lc++;
			if (psize==0)
			{
				//write header
				uint8_t data= 0xAA; // start of program byte		
				eeprom_write_byte(BASIC_PROG_SPACE, data);
				psize++;
			}
			eeprom_write_word(BASIC_PROG_SPACE+psize, newline.lineno);	
			psize+=2;
			eeprom_write_byte(BASIC_PROG_SPACE+psize, newline.token);	
			psize++;		
			eeprom_write_byte(BASIC_PROG_SPACE+psize, newline.var);	
			psize++;	
			eeprom_write_word(BASIC_PROG_SPACE+psize, newline.value);	
			psize+=2;
			
			uint8_t l=0;
			if (newline.text != 0) l = strlen(newline.text);
			eeprom_write_byte(BASIC_PROG_SPACE+psize, l);	
			psize++;
			if (l>0)
			{
				eeprom_write_block(newline.text, BASIC_PROG_SPACE+psize, l);			
				psize+=l;
			}

			eeprom_write_byte(BASIC_PROG_SPACE+psize, 0xCC);	// terminator character
		}
	}
}

/*************************************************************************************************************

RUNTIME routines

*************************************************************************************************************/

int variable[26]; // only A-Z at moment

char *specials[] = { "PF1", "MIC", "X", "Y", "Z", "PSD", "VOLT", "IR", "KBD" };

int get_special(char *str, int *res)
{
	int t=token_match(specials, &str, sizeof(specials));
	int v;
	switch(t) {
	case 0: // PF1
		v=0;
		break;
	case 1: // MIC
		v=adc_mic();
		break;
	case 2: // X
		tilt_read(0);
		v=x_value;
		break;	
	case 3: // Y
		tilt_read(0);
		v=y_value;
		break;		
	case 4: // Z
		tilt_read(0);
		v=z_value;
		break;			
	case 5: // Z
		v=adc_psd();
		break;
	case 6: // VOLT
		v=adc_volt();
		break;
	case 7: // IR
		while ((v= irGetByte())<0) ; //wait for IR
		break;
	case 8: // KBD 
		while ((v= uartGetByte())<0) ; // wait for input
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
		n1=n2+n1; break;
	case '-':
		n1=n2-n1; break;
	case '*':
		n1=n2*n1; break;
	case '/':
		n1=n2/n1; break;
	case '>':
		n1=(n1>n2)?1:0; break;		
	case '<':
		n1=(n1<n2)?1:0; break;	
	case '=':
		n1=(n2==n1)?1:0; break;		
	}
	return n1;
}

#define MAX_DEPTH 5

int str_expr(char *str)
{
	char *p=str;
	while (*str != '"')
	{
		str++;
	}
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
		case ':' :
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
			//rprintf("debug - cond test %d, %d, stack=%d,%d,%d ops=%d, %d\r\n", op, sp, stack[sp-3], stack[sp-2], stack[sp-1], ops[op-2], ops[op-1]);

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
	int nl=1;
	int lno=1;

	while (lno != 0)
	{
		lno=eeprom_read_byte(BASIC_PROG_SPACE+nl);					
		if (lno == 0xCC)
		{
			rprintf("Goto line missing ??\r\n");
			return -1;
		}	
		lno += (eeprom_read_byte(BASIC_PROG_SPACE+nl+1)<<8);	
		
		if (lno == gl)
			break;				
		uint8_t b=eeprom_read_word(BASIC_PROG_SPACE+nl+6);	
		nl = nl + 6+ b +1;
	}
	if (lno!=0) 
	{
		return nl;
	}
	return -1;
}


int put_special(char *str, int var)
{
	int t=token_match(specials, &str, sizeof(specials));
	switch(t) {
	case 0: // PF1
		if (variable[var]==0) PF1_LED1_ON; else PF1_LED1_OFF;
		break;
	}
	return 0;
}
		
void basic_run(int dbf)
{
	// Repeat
	//   Get token
	//   Execute action
	//	 Move to next line
	// Loop
	
	uint8_t data = eeprom_read_byte((uint8_t*)(BASIC_PROG_SPACE));

	if (data==0xAA) {
		rprintfStr("Run Program \r\n");
	}
	else
	{
		rprintfStr("No program loaded\r\n");
		return;
	}
	
	//point top at EEPROM
	
	int ptr=1;
	uint8_t tmp=0;
	char *p;
	
	struct basic_line line;
	
	int forptr[3]; int fp=0; // Upto 3 nested for/next
	
	char buf[64];
	line.text=buf;
	int n;
	while (tmp != 0xCC && ptr < EEPROM_MEM_SZ )
	{
		line.lineno=eeprom_read_word(BASIC_PROG_SPACE+ptr);	
		ptr+=2;
		line.token=eeprom_read_byte(BASIC_PROG_SPACE+ptr);	
		ptr++;		
		line.var=eeprom_read_byte(BASIC_PROG_SPACE+ptr);	
		ptr++;	
		line.value=eeprom_read_word(BASIC_PROG_SPACE+ptr);	
		ptr+=2;
		
		uint8_t l=eeprom_read_byte(BASIC_PROG_SPACE+ptr);	

		eeprom_read_block(line.text, BASIC_PROG_SPACE+ptr+1, l);	
		line.text[l]='\0';
		
		ptr += (l+1);
		
		tmp = eeprom_read_byte(BASIC_PROG_SPACE+ptr);	// terminator character ?

		/* execute code */
	
		if (dbf) rprintf (": %d - ", line.lineno); // debug mode
		
		if (uartGetByte() == 27)  {
			rprintf ("User Break on line %d\r\n", line.lineno); 
			return;
		}
		
		switch (line.token)
		{
		case FOR: 
			// remember where next in struction is
			forptr[fp++] = ptr;
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
					rprintf ("Next synatx error\r\n"); 
					break; //need to handle error	
				}			
				if (variable[line.var] <= n) { 
					// if true set ptr=stack; 
					ptr = t_ptr; tmp=0 ;
					//rprintf ("T dbg %d %d %d\r\n", n, variable[line.var], ptr ); 
				}	
				else
				{
					//rprintf ("F dbg %d %d %d\r\n", n, variable[line.var], ptr ); 
					fp--;
				}
			}
			break;
		case GET: 
			n=0;
			if (get_special(line.text, &n)<0)
				rprintf ("no recognised special\r\n"); 
			variable[line.var]=n;	
			break;
		case PUT: 
			put_special(line.text, line.var);
			break;
		case LET: 
			n=0;
			p=line.text;
			if (eval_expr(&p, &n)==NUMBER)
				variable[line.var] = n;
			//else
			//handle error
			//rprintf ("assign %c= %d\r\n", line.var, n); // DEBUG
			break;
		case GOTO: 
			if ((ptr = gotoln(line.value))<0)
				return;			
			tmp=0;
			break;	
		case PRINT: 
			n=0;
			p=line.text;
			while (1)
			{
				switch (eval_expr(&p, &n))
				{
				case NUMBER:
					rprintf ("%d", n);
					break;
				case STRING:
					n=str_expr(p);
					rprintfStrLen(p,0,n);
					p=p+n+1;
					break;
				}
				if (*p=='\0') break; // done
				
				if (*p!=';' && *p!=',') {
					rprintf ("synatx prob= [%d]\r\n", *p); // DEBUG
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
					if ((ptr = gotoln(n))<0)
						return;			
					tmp=0;		
				}
			}
			break;		
		case XACT: 
			Perform_Action (line.value);
			break;
		case WAIT: 
			_delay_ms(line.value);
			break;
		case END: 
			rprintfStr ("End of program\r\n"); 
			return;
		default:
			rprintf ("Invalid command %x\r\n", line.token); // DEBUG
			tmp=0xCC;
		}		
	}
	
}

void basic_clear()
{
	// Set Init pointer to Zero
	rprintfStr("Clear Program \r\n");
			
	uint8_t data= 0x00; // start of program byte		
	eeprom_write_byte(BASIC_PROG_SPACE, data);
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


void basic_list()
{
	// TBD -  Dump EEprom for the Moment
	rprintfStr("List Program \r\n");
	
	int ptr=1;
	uint8_t tmp=0;
	
	struct basic_line line;
	char buf[64];
	
	line.text=buf;
	while (tmp != 0xCC && ptr < EEPROM_MEM_SZ )
	{
		line.lineno=eeprom_read_word(BASIC_PROG_SPACE+ptr);	
		ptr+=2;
		line.token=eeprom_read_byte(BASIC_PROG_SPACE+ptr);	
		ptr++;		
		line.var=eeprom_read_byte(BASIC_PROG_SPACE+ptr);	
		ptr++;	
		line.value=eeprom_read_word(BASIC_PROG_SPACE+ptr);	
		ptr+=2;
		
		uint8_t l=eeprom_read_byte(BASIC_PROG_SPACE+ptr);	

		eeprom_read_block(line.text, BASIC_PROG_SPACE+ptr+1, l);	
		line.text[l]='\0';
		
		ptr += (l+1);
		
		tmp = eeprom_read_byte(BASIC_PROG_SPACE+ptr);	// terminator character ?

		/* list code */
	
		rprintf ("%d ", line.lineno); 
		rprintfStr (tokens[line.token]);
		rprintf (" "); 
		
		if (line.token==LET || line.token==GET || line.token==PUT || line.token==FOR)
			rprintf ("%c = ", line.var+'A');

		if (line.token==NEXT) 
			rprintf ("%c", line.var+'A');
		else
		if (line.token==GOTO) 
			rprintf ("%d", line.value);
		else
			rprintfStr (line.text);
		rprintf ("\r\n");
	}
	
	//dump(); //debug

}

