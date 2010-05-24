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
#define null  (void *)0

enum  TYPE {SYMBOL, INT, BOOL, FUNCTION, FLOAT, STRING, CELL, EMPTY};

typedef struct object {
	int type;
	union { float 	floatpoint; 
	        int 	number; 
			char 	*string;
			void 	*cell;
		};
} tOBJ;

typedef struct cell { 
	struct object head;		
	struct object tail;		
} tCELL; 


typedef tOBJ (*PFP)(tOBJ);

tOBJ car (tOBJ);
tOBJ cdr (tOBJ);
tOBJ plus(tOBJ);
tOBJ pr  (tOBJ);
tOBJ prn (tOBJ);
tOBJ set (tOBJ);
tOBJ env (tOBJ);

struct prim { char *name; PFP func; } prim_tab[] = { 
	{"env",  env},
	{"car",  car},
	{"plus", plus}, 
	{"cdr",  cdr},
	{"pr",   pr},
	{"set",  set},
	{"prn",  prn}
};

#define PRIMSZ (sizeof(prim_tab)/sizeof(struct prim))

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
	//printstr("[");	printstr(p);	printstr("]"); //debug
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
		if (isWhiteSpace((char)ch)) break;
		*p++ =  (char)ch;
		n++;
	}
	*p =0;
	return newString(n, str);
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

tOBJ eval();
tOBJ callFunction(struct cell *);

tCELL *evalist()
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
		
tOBJ eval()
{
	tOBJ r;
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
			tCELL *args = evalist();
			if (!qf)
				return callFunction(args);
			else
			{
				r.type = CELL;
				r.cell = args;
			}
			return r;
		}

		if ( (ch>= 'A' && ch<= 'Z' ) || (ch>= 'a' && ch<= 'z' ))
		{
			ungetch(ch);
			r.string=readtoken();
			r.type = SYMBOL;
			return r;
		}
	}
	return r;
}


tOBJ print(tOBJ r)
{
	if (r.type == CELL)
	{
		struct cell  *c = r.cell;
		printstr("(");	
		pr(c->head);
		
		while (c->tail.type == CELL)
		{
			c=c->tail.cell;
			printstr(" ");
			pr(c->head);
		}
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
	}
	else if (r.type == STRING)
	{
		printstr(r.string);
	}
	else if (r.type == SYMBOL)
	{
		printstr(r.string);
	}
	else if (r.type == BOOL)
	{
		if ( r.number==0)
			printstr("false");
		else
			printstr("true");
	}
	else if (r.type == EMPTY)
	{
		printstr("NIL");
	}
	else if (r.type == FUNCTION)
	{
		printstr("func: ");
		printstr(r.string);	
	}
	else	
	{		
		printline("? error - type ");	
	}
	return r;
}

tOBJ pr(tOBJ x)
{
	print(x);
	return x;
}

tOBJ prn(tOBJ x)
{
	print(x);
	printline("");
	return x;
}

/**************************************************************************************************/

tOBJ callFunction(tCELL *x)
{
	tOBJ r;
	r.type=CELL;
	r.cell=x;
	if (x->head.type == SYMBOL)
	{
		int fn = -1;
		int fc=0;
		for (fc=0; fc<PRIMSZ; fc++)
		{
			if (strcmp(prim_tab[fc].name, x->head.string)==0)
				fn=fc;
		}	
		if (fn>=0)
		{
			// call fucntion
			r = (*prim_tab[fn].func)(x->tail);
		}
		else
		{
			// user define it
			printstr("Not defined?");
		}
	}
	return r;
}

/**********  primitives  *********/

void showenviron();

tOBJ env(tOBJ list)
{
	tOBJ r; r.type=EMPTY;
	showenviron();
	return r;
}

tOBJ car(tOBJ list)   // i.e. (car  (123 456)) => 123
{
	tOBJ r;
	r.type=EMPTY;
	if (list.type==CELL)
	{
		tCELL *p = list.cell;
		tOBJ  x =p->head;
		if (x.type==CELL)
		{
			p = x.cell	;
			return p->head;
		}
	}
	printline("error");
	return r;
}

tOBJ cdr(tOBJ list) // i.e. (cdr  (123 456)) => (456)
{
	tOBJ r;
	r.type=EMPTY;
	if (list.type==CELL)
	{
		tCELL *p = list.cell;
		tOBJ  x =p->head;
		if (x.type==CELL)
		{
			p = x.cell;	
			return p->tail;
		}
	}
	printline("error");
	return r;
}

tOBJ plus(tOBJ list) // i.e. (plus 123 456) => 597
{
	tOBJ r;
	r.type=INT;
	r.number=0;
	
	int n=0;
	if (list.type==CELL)
	{
		tCELL *p = list.cell;
		n = p->head.number;
		tOBJ t=plus(p->tail);
		n = n + t.number; 
	}
	r.number=n;
	return r;
}

#define SYMSZ 10
struct symbs { char *name; tOBJ val; } symb_tab[SYMSZ];
int symCnt=0;


tOBJ get(char *n)
{
	tOBJ r;
	r.type=EMPTY;
	for (int i=0; i<symCnt; i++)
	{
		if (strcmp(n,symb_tab[i].name)==0)
		{
			return symb_tab[i].val;
		}
	}
	return r;
}

int add(char *n, tOBJ v)
{
	int i=0;
	
	printstr("Add "); printstr(n); printstr("-"); printint(v.type); printline(""); 
	
	for (i=0; i<symCnt; i++)
	{
		if (strcmp(n,symb_tab[i].name)==0)
		{
			symb_tab[i].val =v;
			return i;
		}
	}
	if (symCnt<SYMSZ)
	{
		symb_tab[symCnt].name=n;
		symb_tab[symCnt].val=v;
		symCnt++;
	}
	return symCnt;
}


tOBJ set(tOBJ list)   // i.e. (set a 123 ) => 123 and set a 
{
	tOBJ r;
	r.type=EMPTY;
	if (list.type==CELL)
	{
		tCELL *p = list.cell;  // set head = tail
		tOBJ h = p->head;
		if (h.type == SYMBOL)
			add(h.string, p->tail);
		
	}
	return r;
}


/**************************************************************************************************/

void showenviron()
{
	int i;
	printstr("Commands: ");
	printstr(prim_tab[0].name);
	for (i=1; i<PRIMSZ; i++)
	{
		printstr(", ");
		printstr(prim_tab[i].name);
	}
	printline("");	
		
	if (symCnt>0) {
		printstr("Symbols: ");
		printstr(symb_tab[0].name);
		for (i=1; i<SYMSZ; i++)
		{
			printstr(", ");
			printstr(symb_tab[i].name);
		}
		printline("");	
	}
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
			print(r);	
			printline("");	
		}
	}
}

#define	RUN_LED1_ON			CLR_BIT5(PORTA)     // BLUE
#define	RUN_LED2_ON			CLR_BIT6(PORTA)     // GREEN

void femto()
{
	printstr("Femto 0.1\r\n");
	showenviron();
	RUN_LED1_ON;
	RUN_LED2_ON;	
	UCSR1B= (1<<RXEN)|(1<<TXEN) ; //enable PC read/write Not interupt;	
	repl();
}