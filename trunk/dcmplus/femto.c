#include <avr/io.h>
#include "Macro.h"

/**************************************************************************************************/

#define	RXC					7
#define RX_COMPLETE			(1<<RXC)

extern void putByte (BYTE b);

int getByte()
{
	while(!(UCSR1A & RX_COMPLETE)) ;
	return UDR1;
}

/**************************************************************************************************/
#define MAX 100
char inputbuffer[MAX];
int  ibcnt;
char *ibptr;

void readline()
{
	int ch=0;
	ibcnt=0;
	ibptr=&inputbuffer[0];
	
	while (ibcnt<MAX-1) 
	{
		ch=getByte();
		putByte(ch);            //echo input

		if (ch == 13) 
		{
			putByte(10);
		    break;
		}
		if (ch==8 || ch==127)   //Bsapce ?
		{
			if (ibcnt>0) ibcnt--;
		}
		else
			inputbuffer[ibcnt++] = ch;
	}		
	inputbuffer[ibcnt]=0;
}

void ungetch(int c)
{
	if (ibptr>inputbuffer)
	{
		--ibptr;
		*ibptr=c;
	}
}

int getch()
{
	if (*ibptr!=0)
		return *ibptr++;
	return -1;
}

void putch(char c)
{
	putByte(c);
}

void printstr(char *c)
{
	char ch;
	while ((ch=*c++) != 0) putch(ch);
}

void printline(char *c)
{
	printstr(c);
	putch(13);
	putch(10);
}

void printint(int n) //tbd
{
	char tb[10];
	char *cp=tb;
	int n1 = (n<0)?-n:n;
	while (n1>9)
	{
		*cp++ = '0' + (n1%10);
		n1 = n1/10;
	}
	*cp++ = '0' + (n1%10);
	if (n<0) *cp++ = '-';
	while (cp>tb)
	{
		cp--;
		putch(*cp);
	}
}

/**************************************************************************************************/
#define bool  int
#define true  1
#define false 0
#define null  0

enum  TYPE {INT, BOOL, FUNCTION, STRING, CELL};

typedef struct cell {
	void *head; 
	void *tail;
};

struct object {
	int type;
	union { int n; struct cell c; char *s;};
};

int readdigit()
{
	int num = 0;
	int ch = 0;
	while ((ch = getch()) > 0)
	{
		if (!(ch>='0' && ch<='9'))
		{
			ungetch(ch);
			return num;
		}
		num = num * 10 + (ch - '0');
	}
	return num;
}

char * readstring()
{
	char* str = "";
	int ch = 0;
	while ((ch = getch()) > 0)
	{
		if (ch=='"')
			return str;
		str += (char)ch;
	}
	return str;
}

	
bool isWhiteSpace(int ch)
{
	return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
}

bool isSymbol(int ch)
{
	return (ch == '+' || ch == '-' || ch == '_' || ch == '.');
}

bool isLetterorDigit(int ch)
{
	return ((ch >= 'A' && ch <= 'Z' ) ||
	 (ch >= 'a' && ch <= 'z' ) ||
	 (ch >= '0' && ch <= '9' ) );
}

void readwhitespace()
{
	int ch = 0;
	while ((ch = getch()) > 0 && isWhiteSpace((char)ch)) ;
	ungetch(ch);
}

char * readtoken()
{
	char * str = "";
	int ch = 0;
	while ((ch = getch()) > 0)
	{
		if (isWhiteSpace((char)ch))
			return str;
		str += (char)ch;
	}
	return str;
}
	
		
/**************************************************************************************************/

struct object eval();

struct cell evalist()
{
	int ch;
	struct cell top;

	struct cell *nxt = &top;            
	struct cell *prv;

	while ((ch = getch()) > 0)
	{
		if ((char)ch == ')')
		{
			if (prv != 0)
				prv->tail=0;
			return top;
		}
		ungetch(ch);
		//nxt.head = eval();
		//struct cell x;
		//nxt.tail = x;
		//prv=nxt;
		//nxt=(struct cell *)nxt.tail;
	}
	return top;
}
		
struct object eval()
{
	struct object r;
	
	int ch;
	bool qf = false;

	while ((ch = getch()) > 0)
	{
		if (isWhiteSpace((char)ch))
		{
			qf = false;
			readwhitespace();
			continue;
		}

		if (ch >= '0' && ch <= '9')
		{
			ungetch(ch);       
			r.type=INT;
			r.n=readdigit();
			return r;
		}
		if (ch == '"')
		{
			//return (struct object)readstring();
		}
		if (ch == '\'') //quote shortcut
		{
			qf = true;
		}

		if (ch == '(')
		{
			struct cell args = evalist();
			//if (!qf)
			//	return callFunction(args);
			//else
			r.type = CELL;
			r.c = args;
			return r;
		}

		if ( (ch>= 'A' && ch<= 'Z' ) || (ch>= 'a' && ch<= 'z' ))
		{
			ungetch(ch);
			char* t=readtoken();
			/*
			if (!qf)
			{
				object o = env.find(t);
				if (o == null)
				{
					//function f = new function();
					//f.name = t;
					//return f;
				}
				return o;
			}
			else
			{
				return t;
			}
			*/
		}
	}
	return r;
}

void pr(struct cell x)
{
}
/*
struct object pr(struct cell x)
{
	struct cell nxt = x;
	struct object r;
	
	while (nxt != null)
	{
		r = nxt.head;
		if (nxt.head is string)
		{
			printstr(nxt.head);
		}
		else if (nxt.head is int)
		{
			printint(nxt.head);
		}
		else if (nxt.head is function)
		{
			printstr("Function: " + ((function)nxt.head).name);
		}
		else if (nxt.head is cell)
		{
			printstr("(");
			pr((struct cell)nxt.head);
			printstr(")");
		}
		if (nxt.tail is cell)
		{
			nxt = (struct cell)nxt.tail;
		}
		else
		{
			if (nxt.tail != null) 
				printstr("." + (string)nxt.tail);
			nxt = null;
		}
	}
	return r;
}
*/

void prn(struct cell x)
{
	pr(x);
	printline("");
	return ;
}

void repl()
{
	while (true)
	{
		printstr("> ");
		readline();
		while (ibptr-inputbuffer < ibcnt)
		{
			struct object r = eval();

			if (r.type == CELL)
			{
				pr(r.c);
				printline("");
			}
			if (r.type == INT)
			{
				printint(r.n);
				printline("");
			}
			//else
			//	printline((r != null) ? r.ToString() : "? error");
			
		}
	}
}

#define	RUN_LED1_ON			CLR_BIT5(PORTA)     // BLUE
#define	RUN_LED1_OFF		SET_BIT5(PORTA)
#define	RUN_LED2_ON			CLR_BIT6(PORTA)     // GREEN
#define	RUN_LED2_OFF		SET_BIT6(PORTA)

void femto()
{
	printstr("Femto 0.1\r\n");
	RUN_LED1_ON;
	RUN_LED2_ON;	
	UCSR1B= (1<<RXEN)|(1<<TXEN) ; //enable PC read/write Not interupt;	
	repl();
}