#include <avr/io.h>
#include "Macro.h"

extern int strcmp(char *, char *);
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

enum  TYPE {INT, BOOL, FUNCTION, FLOAT, STRING, CELL, EMPTY};


struct object {
	int type;
	union { float 	floatpoint; 
	        int 	number; 
			char 	*string;
			void 	*cell;
		};
} tOBJ;

struct cell { 
	struct object head;		
	struct object tail;		
} tCELL; 


struct object car(struct object);

typedef int (*PFI)();
	
typedef struct object (*PFP)(struct object);

struct prim { char *name; PFI func; } prim_tab[] = { "car", car };


/**************************************************************************************************/

#define MEMSZ 100
struct cell memory[MEMSZ];
int mem=0;

struct cell *newCell()
{
	if (mem<MEMSZ-1)
	{
		return &memory[mem++];
	}
	return (struct cell*)0;
}

void delCell(struct cell *p)
{
	//tbd
}

BYTE stringbuffer[1024];
BYTE *stop=stringbuffer;

void copyString(char *a, char *b, int n)
{
	while (n-->=0 && *b!=0)
		*a++ = *b++;
	*a=0; // null terminate
}

char *newString(int n, char *txt)
{
	char  *p;
	if ((stop-stringbuffer)+ n+2 >=1024)
	{
	    printline("alloc failed");
		return (char *)0;
	}
	if (n>=128)
	{
		printline("too big");
		return (char *)0;
	}
	p=(char *)stop;
	*p++ = (n%256)|0x80;  // use bit 7 to indicate in use
	stop += (n+1);
	copyString(p, txt, n);
	return p;
}

void delString(char *p)
{
	*(p-1) = *(p-1) & 0x7f;  // clear bit7
}

void garbageCollect()
{
	//tbd
	// free up string space
	// free up cell space
}

/**************************************************************************************************/

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

	
bool isWhiteSpace(int ch)
{
	return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
}


char *readstring()
{
	char str[20];
	char *p=str;
	int ch = 0;
	int  n=0;
	while (((ch = getch()) > 0) && n<20)
	{
		if (ch=='"')
		{
			*p =0;
			return newString(n, str);
		}
		*p++ = (char)ch;
		n++;
	}
	return (char *)0; //error
}

char *readtoken()
{
	char str[20];
	char *p=str;
	int  n=0;	
	int ch = 0;
	while (((ch = getch()) > 0) && n<20)
	{
		if (isWhiteSpace((char)ch))
		{
			*p =0;
			return newString(n, str);
		}
		*p++ =  (char)ch;
		n++;
	}
	return (char *)0; //error
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


		
/**************************************************************************************************/

struct object eval();

struct cell *evalist()
{
	int ch;

	struct cell *nxt = newCell();	
	struct cell *top = nxt;
	struct cell *prv = 0;

	while ((ch = getch()) > 0)
	{
		if ((char)ch == ')')
		{
			return top;
		}
		ungetch(ch);
		if (prv != 0)
		{
			nxt = newCell();	
			prv->tail.type = CELL;
			prv->tail.cell = nxt;
		}
		nxt->head = eval();
		nxt->tail.type=EMPTY;
		prv=nxt;
	}
	return top;
}
		
struct object eval()
{
	struct object r;
	r.type=EMPTY;
	
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
			r.number=readdigit();
			return r;
		}
		if (ch == '"')
		{
			r.type=STRING;
			r.string=readstring();
			return r;
		}
		if (ch == '\'') //quote shortcut
		{
			qf = true;
		}

		if (ch == '(')
		{
			struct cell *args = evalist();
			//if (!qf)
			//	return callFunction(args);
			//else
			r.type = CELL;
			r.cell = args;
			return r;
		}

		if ( (ch>= 'A' && ch<= 'Z' ) || (ch>= 'a' && ch<= 'z' ))
		{
			int fc=0;
			
			ungetch(ch);
			r.string=readtoken();
			r.type = FUNCTION;
			
			for (fc=0; fc<sizeof(prim_tab); fc++)
			{
				printline(prim_tab[fc].name);
				if (strcmp(prim_tab[fc].name, r.string)==0)
				{
					printline("matched");
				}
			}
			
			return r;
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

void pr(struct object r)
{
	if (r.type == CELL)
	{
		printstr("(");
		struct cell  *c = r.cell;
		pr(c->head);
		if (c->tail.type != EMPTY)
		{
			printstr(".");
			pr(c->tail);
		}
		printstr(")");		
	}
	else if (r.type == INT)
	{
		printint(r.number);
		printline("");
	}
	else if (r.type == STRING)
	{
		printline(r.string);
	}
	else if (r.type == EMPTY)
	{
		printline("NIL");
	}
	else if (r.type == FUNCTION)
	{
		printstr("func: ");
		printline(r.string);
	}
	else	
	{		
		printline("? error - type ");	
	}
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

void prn(struct object x)
{
	pr(x);
	printline("");
	return ;
}

/**************************************************************************************************/

/**********  primitives  */

struct object car(struct object list)
{
	struct object r;
	r.type=EMPTY;
	if (list.type==CELL)
	{
		struct cell *p = list.cell;
		return p->head;
	}
	else
		return r;
}

struct object plus(struct cell top)
{
	struct object r;
	r.type=EMPTY;
//	if (top == null)
//		return 0;

//	if (top.head is int)
//	{
//		return (int)top.head + (int)plus((cell)top.tail);
//	}
//	else
//		return 0;
	return r;
}

/**************************************************************************************************/

void repl()
{
	while (true)
	{
		printstr("> ");
		readline();
		while (ibptr-inputbuffer < ibcnt)
		{
			struct object r = eval();
			pr(r);		
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