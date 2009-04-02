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
[LINE no] PRINT LIST
[LINE No] END
[LINE no] IF  EXPR THEN LINE no ELSE Line No
[LINE No] FOR VAR '=' EXPR 'To' EXPR
[LINE No] NEXT
[LINE No] PUT VAR '=' PORTA|B|C:0-7
[LINE No] GET VAR '=' PORTA|B|C:0-7|ADC|PSD|IR|SERVO
[LINE No] SCENE LIST
[LINE No] XACT EXPR

Special Commands
SET | GET  enable access to PORTS/SPECIAL REGISTER, IR, ADC, PSD etc ...
SCENE      sends a Scene - 16 Servo Positions, Plus time, no frames
XACT       Call any Experimental action using literal code i.e. XACT 0, would do basic pose, XACT 17 a wave


Example Programs
================

10 LET A=1
20 PRINT A
30 LET A=A+1
35 WAIT 500
40 IF A<10 THEN 20 ELSE 50
50 END

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

#include <util/delay.h>

#define EEPROM_MEM_SZ 256

uint8_t EEMEM BASIC_PROG_SPACE[EEPROM_MEM_SZ];  // this is where the tokenised code will be stored

extern void Perform_Action(BYTE action);

const  prog_char *error_msgs[] = {
	"",
	"Syntax error",
	"Invalid Command",
	"Illegal var",
	"Bad number",
	};
	
enum {
	LET=0, FOR, IF, THEN, 
	ELSE, GOTO, PRINT, GET, 
	PUT, END, SCENE, XACT, 
	WAIT
	};
	
const prog_char *tokens[] ={
	"LET", "FOR", "IF","THEN", 
	"ELSE","GOTO","PRINT","GET",
	"PUT", "END", "SCENE", "XACT",
	"WAIT"
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
char getNext(char *line, int *i) 
{
	char c1=*(line+*i);
	
	while (c1 == ' ' )
	{
		*i=*i+1;
		c1=*(line+*i);
	}
	if (c1 != '\0') *i=*i+1;
	return c1;
}

// Read variable - 
// Simple def - must be A-Z
// More complex later
char getVar(char *line, int *i) 
{
	char c1=getNext(line, i);;
	
	if (c1>='A' && c1<= 'Z' )
	{
		return c1;
	}
	else
	return '\0';
}

// Read number  - 
// Simple def - must be 0-9
// More complex later (%/& etc)
int getNum(char *line, int *i) 
{
	int num=0;
	char c1=getNext(line, i);
	
	while (c1>='0' && c1<= '9' )
	{
		num = num*10 + c1-'0';
		c1=*(line+*i);	
		*i=*i+1;	
	}
	*i=*i-1;	
	return num;
}

void basic_load()
{
	rprintf("Enter Program '.' to Finish\r\n");
	// Repeat
	//   Read a line
	//   If a '.' stop
	//   Convert into tokens
	//   Store in Eprom
	// Loop
	
	char line[80];
	int n=0;
	int ln=0;
	int lc=0;
	int i=0;
	
	errno=0;
	
	uint16_t psize=0;
	
	while (1)
	{
		// for each line entered
		int ch;
		
		if (errno != 0) {
			rprintf ("Error - '" );
			rprintfStr (error_msgs[errno]);
			rprintf ("'\r\n" );
			rprintf ("Pos=%d\r\n", i);
			errno=0;
		}
		
		n=0;	
	
		while (1)
		{
			// foreach char entered

			while ((ch = uartGetByte())<0) ;
				
			rprintf("%c", ch);	
					
			if (ch==13 || (n==0 && ch=='.') )
			{
				line[n]='\0';
				rprintf("\r\n");	
				break;
			}
				
			if (ch >= 'a' && ch <= 'z') ch = ch-'a'+'A';  // Uppercase only
			
			if (ch==8) //Bsapce
			{
				if (n>0) n--;
			}
			else
				line[n++] = ch;
		}
		if ( ch=='.' && n==0)
		{
			rprintf("\r\n%d lines entered\r\n", lc);
			return;
		}
		
		// convert to token
		
		struct basic_line newline;	
		
		newline.var=0;
		newline.lineno=0;
		newline.token=0;
		newline.var=0;
		newline.value=0;
		newline.text=0;	
			
		i=0;
		if (i>=n) break;
		if (line[i]>='0' && line[i] <= '9') // test for optional line
		{
			ln=0;
			// read in digit up to a space
			while (line[i] != ' ')
			{
				if (line[i]>= '0' && line[i] <= '9')  
					ln=ln*10 + line[i]-'0';
				else 
				{
					errno=1; // Syntax error
					break;
				}
				i++;
			}
		}
		
		newline.lineno = ln++;
		
		// rprintf("Line No=%d\r\n", newline.lineno); //DEBUG
		
		if (i>=n || errno != 0) continue;	

		int p=++i; // remember where we are
		
		while (line[i] != ' ')
		{
			if (!(line[i]>= 'A' && line[i] <='Z') ) 
			{
				errno=1; // Syntax error
				break;
			}
			i++;
		}	
		line[i]='\0';
		if (i>=n || errno != 0) continue;	
		
		//Look up token
		int t;
		
		for (t=0; t<sizeof(tokens); t++)
		{
			if (!strcmp(&line[p], tokens[t]))
				break;
		}
		
		//rprintf ("token="); 				//DEBUG
		//rprintfStrLen(line,p,i-p);			//DEBUG
		//rprintf(" [%d]\r\n", t);			//DEBUG
		
		if (t==sizeof(tokens))
		{
			errno=2;
			continue;
		}
		
		newline.token=t;
		i++;
		
		switch (t)
		{
		case LET: 
		case GET: 
		case PUT: 
			// read Variable
			newline.var = getVar(line,&i);
			if (newline.var=='\0')
			{
				errno=3;
			}
			// '='
			if (getNext(line,&i) != '=')
			{
				errno=1;
			}
			// expression
			newline.text=line+i;
			//rprintfStr(newline.text);			//DEBUG

			break;
		case PRINT:
			newline.text=line+i;
			break;
		case IF:
			newline.text=line+i;
			// We should check for THEN and ELSE
			// and handle error
			// replace THEN with ? and ELSE with :
			// IF A THEN B ELSE C =>  GOTO (A)?B:C
			char *p_then = strstr(line+i, "THEN");
			char *p_else = strstr(line+1, "ELSE");
			if (p_then != 0) 
				strncpy(p_then,"?   ",4);
			else
				errno=3;
				
			if (p_else != 0) 
				strncpy(p_else,":   ",4);
			else
				strcat(line+1,":0");
			
			rprintfStr(newline.text);			//DEBUG		
			break;
		case WAIT: 
		case GOTO: 
		case XACT:
			// read line
			newline.value = getNum(line,&i);			
			//rprintf ("val=%d\r\n", newline.value); 		//DEBUG
			break;
		case END:
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


int variable[26];

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
		n1=(n2>n1)?1:0; break;		
	case '<':
		n1=(n2<n1)?1:0; break;	
	case '=':
		n1=(n2==n1)?1:0; break;		
	}
	return n1;
}

#define MAX_DEPTH 5

unsigned char eval_expr(char *str, int *ptr, int *res)
{
	char c;
	
	int n1=0;
	int stack[MAX_DEPTH]; 
	char ops[MAX_DEPTH];
	
	int sp=0;
	int op=0;
	int tmp=0;
	
	while (str[*ptr] != '\0')
	{
		if ((c = str[(*ptr)++])==')')
			break;

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
			eval_expr(str, ptr, &tmp);
			n1 = tmp;
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
			break;
		case ' ':
			break;
		default:
			return ERROR;
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
			}
			else
			{
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
	rprintf ("goto line = %d\r\n", gl); // DEBUG
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
		
void basic_run(int dbf)
{
	// Repeat
	//   Get token
	//   Execute action
	//	 Move to next line
	// Loop
	
	uint8_t data = eeprom_read_byte((uint8_t*)(BASIC_PROG_SPACE));

	if (data==0xAA) {
		rprintf("Run Program \r\n");
	}
	else
	{
		rprintf("No program loaded\r\n");
		return;
	}
	
	//point top at EEPROM
	
	int ptr=1;
	uint8_t tmp=0;
	
	struct basic_line line;
	
	char buf[64];
	line.text=buf;
	int n;
	int i;	
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
		
		switch (line.token)
		{
		case GET: 
		case PUT: 
			break;
		case LET: 
			i=0; n=0;
			if (eval_expr(line.text, &i, &n)==NUMBER)
				variable[line.var-'A'] = n;
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
			//rprintf ("xact lit = %d\r\n", line.value); // DEBUG
			i=0; n=0;
			if (eval_expr(line.text, &i, &n)==NUMBER)
				rprintf ("%d\r\n", n);
			break;	
		case IF:		
			i=0; n=0;
			if (eval_expr(line.text, &i, &n)==NUMBER)
			{
				rprintf ("%d\r\n", n);
				if (n != 0)
				{
					if ((ptr = gotoln(n))<0)
						return;			
					tmp=0;		
				}
			}
			break;		
		case XACT: 
			//rprintf ("xact lit = %d\r\n", line.value); // DEBUG
			Perform_Action (line.value);
			break;
		case WAIT: 
			//rprintf ("wait lit = %d\r\n", line.value); // DEBUG
			_delay_ms(line.value);
			break;
		case END: 
			rprintf ("End of program\r\n"); 
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
	rprintf("Clear Program \r\n");
			
	uint8_t data= 0x00; // start of program byte		
	eeprom_write_byte(BASIC_PROG_SPACE, data);

}

void basic_list()
{
	// TBD -  Dump EEprom for the Moment
	int i;
	
	rprintf("List Program \r\n");
	
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
		rprintf ("\r\n");	
	}
}

