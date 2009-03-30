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
[LINE no] IF  EXPR THEN LINE no ELSE Line No
[LINE no] GOTO [Line No]
[LINE no] PRINT LIST
[LINE No] END
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

enum {
	PLUS, MINUS, MULT, DIV, OPAREN, CPAREN, 
	LET, FOR, IF, THEN, ELSE, GOTO, PRINT, SET, END, SCENE, GET, XACT
	};
	
const  prog_char *error_msgs[] = {
	"",
	"Syntax error",
	"Invalid Command",
	"Illegal var",
	};
	
const prog_char *tokens[] ={"LET", 
							"IF",
							"THEN", 
							"ELSE",
							"GOTO",
							"PRINT",
							"SET",
							"END",
							"SCENE",
							"GET",
							"XACT" 
							};
							
const prog_char opers[]  = "+-*\()";

struct basic_line {
    int lineno;
	unsigned char token;
	unsigned char var;
	int value;
	char *text;
	struct basic_line *next;
};

// Read vriable - 
// Simple def - must be A-Z
// More complex later
char getVar(char *line, int *i) 
{
	char c1=*(line+*i);
	char c2=*(line+*i+1);
	if (c1>='A' && c1<= 'Z' && c2 == ' ')
	{
		*i=*i+2;
		return c1;
	}
	else
	return '\0';
}

void basic_load()
{
	rprintf("Enter Program '.' to Finish\r\n");
	// TBD
	// Repeat
	//   Read a line
	//   If a '.' stop
	//   Convert into tokens
	//   Store in Eprom
	// Loop
	
	char line[80];
	int n=0;
	int ln=0;
	int i=0;
	int errno=0;
	
	while (1)
	{
		// for each line entered
		int ch;
		
		if (errno != 0) {
			rprintf ("Error - '" );
			rprintfProgStr (error_msgs[errno]);
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
				break;
				
			if (ch >= 'a' && ch <= 'z') ch = ch-'a'+'A';  // Uppercase only

			line[n++] = ch;
		}
		if ( ch=='.' && n==0)
		{
			rprintf("\r\n%d lines entered\r\n", ln);
			return;
		}
		
		// convert to token
		
		struct basic_line newline;	
			
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
		
		switch (t)
		{
		case 0: // LET command
			// read Variable
			newline.var = getVar(line,&i);
			if (newline.var=='\0')
			{
				errno=3;
			}
			// '='
			// expression
			break;
		case 4: // GOTO command
			// read line
			break;
		default:
			errno=2;
			break;
		}
		
		
		// store struct in eeprom
	}
}

void basic_run()
{
	// TBD
	// Repeat
	//   Get token
	//   Execute action
	//	 Move to next line
	// Loop
	rprintf("Run Program \r\n");
	struct basic_line *top=0;
	
	//point top at EEPROM
	
	while (top !=0)
	{
		/* execute code */
		
		rprintf (": %d\n", top->lineno); // DEBUG
		
		top = top->next;
	}
	
}

void basic_clear()
{
	// TBD
	// Set Init pointer to Zero
	rprintf("Clear Program \r\n");

}

void basic_list()
{
	// TBD
	// 
	rprintf("List Program \r\n");
}

