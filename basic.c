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
[LINE No] SET PORTA|B|C:0-7|ADC|PSD|IR|SERVO '=' EXPR
[LINE No] GET VAR '=' PORTA|B|C:0-7|ADC|PSD|IR|SERVO
[LINE No] SCENE LIST
[LINE No] XACT EXPR

Special Commands
SET | GET  enable access to PORTS/SPECIAL REGISTER, IR, ADC, PSD etc ...
SCENE      sends a Scene - 16 Servo Positions, Plus time, no frames
XACT       Call any Experimental action using literal code i.e. XACT 0, would do basic pose, XACT 17 a wave


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

#define EEPROM_MEM_SZ 64

uint8_t EEMEM BASIC_PROG_SPACE[EEPROM_MEM_SZ];  // this is where the tokenised code will be stored

extern void Perform_Action(BYTE action);

const  prog_char *error_msgs[] = {
	"",
	"Syntax error",
	"Invalid Command",
	"Illegal var",
	"Bad number",
	};
	
/*
struct tok {
	char *token;
	char val;
	};
	
const struct tok tab = { 
	{ "LET",   LET },
	{ "IF",    IF },
	{ "THEN",  THEN },
	{ "ELSE",  ELSE },
	{ "GOTO",  GOTO },
	{ "PRINT", PRINT },
	{ "SET",   SET },
	{ "END",   END },
	{ "SCENE", SCENE },
	{ "GET",   GET },
	{ "XACT" , XACT }
	};

*/
enum {
	LET=0, FOR, IF, THEN, ELSE, GOTO, PRINT, SET, END, SCENE, GET, XACT, WAIT,
	PLUS, MINUS, MULT, DIV, OPAREN, CPAREN
	};
	
const prog_char *tokens[] ={"LET", 
							"FOR", 
							"IF",
							"THEN", 
							"ELSE",
							"GOTO",
							"PRINT",
							"SET",
							"END",
							"SCENE",
							"GET",
							"XACT",
							"WAIT"
							};
							
const prog_char opers[]  = "+-*\()";

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
		
		rprintf("Line No=%d\r\n", newline.lineno); //DEBUG
		
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
		
		rprintf ("token="); 				//DEBUG
		rprintfStrLen(line,p,i-p);			//DEBUG
		rprintf(" [%d]\r\n", t);			//DEBUG
		
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
		case WAIT: 
		case GOTO: 
		case XACT:
			// read line
			newline.value = getNum(line,&i);			
			//rprintf ("val=%d\r\n", newline.value); 		//DEBUG
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

enum {STRING, NUMBER, ERROR } ;

unsigned char eval_expr(char *str, int *ptr, int *res)
{
	char c=*(str+*ptr);
	
	int n1=0;
	int n2=0;
	char op='\0';
	
	while (c != '\0')
	{
		if (c>='0' && c<='9')
		{
			n1 = n1*10 + c - '0';
		}
		
		if (c>='A' && c<='Z')
		{
			n1=variable[c-'A'];
		}
		switch (c)
		{
		case '(':
			break;
		case ')':
			break;
		case '+' :
		case '-' :
		case '*' :
		case '/' :
		case '>' :
		case '<' :
		case '=' :
			op=c;
			n2=n1;
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
		
		*ptr = *ptr+1;	
		c=*(str+*ptr);
	}
	if (op != '\0')
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
	}
	*res = n1;
	return NUMBER;
}

void basic_run()
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
	
		rprintf (": %d - ", line.lineno); // DEBUG
		
		switch (line.token)
		{
		case LET: 
			i=0; n=0;
			if (eval_expr(line.text, &i, &n)==NUMBER)
				variable[line.var-'A'] = n;
			//else
			//handle error
			//rprintf ("assign %c= %d\r\n", line.var, n); // DEBUG
			break;
		case GOTO: 
			rprintf ("goto line = %d\r\n", line.value); // DEBUG
			// scan from top until match line no or hit end
			int nl=1;
			int lno=1;
			while (lno != 0)
			{
				// rprintf ("%d", lno); // DEBUG
				lno=eeprom_read_byte(BASIC_PROG_SPACE+nl);	
				
				if (lno == 0xCC)
				{
					rprintf("Goto line missing ??\r\n");
					return;
				}
				
				lno += (eeprom_read_byte(BASIC_PROG_SPACE+nl+1)<<8);	
				//rprintf("test %d=%d\r\n", lno, line.value);
				
				if (lno == line.value)
					break;
				
				uint8_t b=eeprom_read_word(BASIC_PROG_SPACE+nl+6);	

				nl = nl + 6+ b +1;
			}
			if (lno!=0) 
			{
				//rprintf("Line match %d\n", lno); // DEBUG
				ptr=nl; // this is where happens
				tmp=0;
			}			
			break;	
		case PRINT: 
			//rprintf ("xact lit = %d\r\n", line.value); // DEBUG
			i=0; n=0;
			if (eval_expr(line.text, &i, &n)==NUMBER)
				rprintf ("%d\r\n", n);
			break;	
		case XACT: 
			//rprintf ("xact lit = %d\r\n", line.value); // DEBUG
			Perform_Action (line.value);
			break;
		case WAIT: 
			//rprintf ("wait lit = %d\r\n", line.value); // DEBUG
			_delay_ms(line.value);
			break;
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

