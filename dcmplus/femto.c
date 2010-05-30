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

void printnumber(int n, int w, char pad) 
{
	char tb[10];
	char *cp=tb;
	int n1 = (n<0)?-n:n;
	while (n1>9)
	{
		*cp++ = '0' + (n1%10);
		n1 = n1/10;
		w--;
	}
	*cp++ = '0' + (n1%10);
	w--;
	if (n<0) *cp++ = '-';
	while (w-- > 0) *cp++ = pad;
	while (cp>tb)
	{
		cp--;
		putch(*cp);
	}
}

void printint(int n) 
{
	printnumber(n, 0, '\0');
}

/**************************************************************************************************/
#define bool  int
#define true  1
#define false 0
#define null  (void *)0

enum  TYPE {SYMBOL, INT, BOOL, FUNCTION, FLOAT, STRING, CELL, EMPTY, ERROR, SPECIAL};

typedef struct object {
	int type;
	union { float 	floatpoint; 
	        int 	number; 
			char 	*string;
			void 	*cell;
			void 	*func;
		};
} tOBJ;

typedef struct cell { 
	tOBJ 			head;		
	struct cell*  	tail;		
} tCELL, *tCELLp; 

typedef tOBJ (*PFP)(tCELLp);

tOBJ car (tCELLp);
tOBJ cdr (tCELLp);
tOBJ plus(tCELLp);
tOBJ pr  (tCELLp);
tOBJ prn (tCELLp);
tOBJ set (tCELLp);
tOBJ setq(tCELLp);
tOBJ env (tCELLp);
tOBJ eq  (tCELLp);
tOBJ iff (tCELLp);
tOBJ call(tCELLp);

struct prim { char *name; PFP func; BYTE f;} prim_tab[] = { 
	{"env",  env, 0},
	{"car",  car, 0},
	{"plus", plus,0}, 
	{"cdr",  cdr, 0},
	{"pr",   pr,  0},
	{"set",  set, 0},
	{"prn",  prn, 0},
	{"eq",   eq,  0},
	{"setq", setq,1},
	{"if",   iff, 1},
	{"eval", call,0}
};

#define PRIMSZ (sizeof(prim_tab)/sizeof(struct prim))

/**************************************************************************************************/

#define MEMSZ 100
tCELL memory[MEMSZ];
int mem=0;
void printtype(tOBJ);

tCELLp newCell()
{
	if (mem<MEMSZ-1)
	{
		return &memory[mem++];
	}
	return null;
}

void delCell(tCELLp p)
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
	stop += (n+2);
	copyString(p, txt, n);
	*stop++ = '$' ; //add a guard
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
	printint(stop-stringbuffer); printline(" bytes");
	BYTE *bot = stringbuffer;
	while (bot<stop)
	{
		printint(bot-stringbuffer); printstr("] <");
		printstr((char *)(bot+1));printline(">");
		bot += ((*bot)&0x7f) + 3; // add no. of bytes + 2 +  guard '$'
	}
	
	printint(mem); printline(" cells");
	for (int i=0; i< mem; i++)
	{
		printint(i); printstr("-"); printtype(memory[i].head); 
		
		printstr("->"); 
		
		if (memory[i].tail != null)
		{
			int n = (memory[i].tail-&memory[0]); ///sizeof(tCELL);
			printint(n);
		}
		else
		{
			printstr("null");
		}
		
		printline("");
	}

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

bool isLetterorDigit(int ch)
{
	return ((ch >= 'A' && ch <= 'Z' ) ||
	 (ch >= 'a' && ch <= 'z' ) ||
	 (ch >= '0' && ch <= '9' ) );
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

tOBJ get(char *n);

tOBJ readtoken(bool quoteflag)
{
	char str[20];
	char *p=str;
	int  n=0;	
	int ch = 0;
	while (((ch = getch()) > 0) && n<20)
	{
		if (!isLetterorDigit(ch)) 
		{
			ungetch(ch);
			break;
		}
		*p++ =  (char)ch;
		n++;
	}
	*p =0;

	// SYMBOL - FUNCTION - BOOL - EMPTY 
	tOBJ r;
	r.type=EMPTY;
	
	if (strcmp(str, "true")==0)
	{
		r.type = BOOL;
		r.number = 1;
	}
	else if (strcmp(str, "false")==0)
	{
		r.type = BOOL;
		r.number = 0;
	}
	else
	{
		if (quoteflag==true)
		{
			r.string = newString(n, str);
			r.type   = SYMBOL;
			return r;
		}
	
		int fn = -1;
		int fc=0;
		for (fc=0; fc<PRIMSZ; fc++)
		{
			if (strcmp(prim_tab[fc].name, str)==0)
				fn=fc;
		}	
		
		if (fn>=0)
		{
			//  fucntion	
			r.type = (prim_tab[fn].f==1)?SPECIAL:FUNCTION;
			r.func = prim_tab[fn].func;
		}
		else
		{
			// user define it?		
			r = get(str);
			
			if (r.type==EMPTY)
			{
				printstr("Not defined?");
				printline(str);
			}
			
		}
	}		
	return r;
}

bool isSymbol(int ch)
{
	return (ch == '+' || ch == '-' || ch == '_' || ch == '.');
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

tCELL *evalist(bool qf)
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
			prv->tail = nxt;
		}
		nxt->head = eval(qf);
		nxt->tail = null;
		prv=nxt;
		
		if (top->head.type == SPECIAL)
			qf=true;
	}
	return top;
}
	

tOBJ eval(bool qf)
{
	tOBJ r;
	r.type=EMPTY;
	
	int ch;
	
	bool fl = qf;

	while ((ch = getch()) > 0)
	{
		if (isWhiteSpace((char)ch))
		{
			qf = fl;
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
			tCELL *args = evalist(qf);
			if (!qf )
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
			r=readtoken(qf);			
			return r;
		}
	}
	return r;
}

void printtype(tOBJ r)
{
	if (r.type == CELL)
	{
		printstr("CELL#");	
		if (r.cell != null)
		{
			int n = ((tCELLp)(r.cell)-&memory[0]); ///sizeof(tCELL);
			printint(n);
		}
		else
		{
			printstr("null");
		}
	}
	else if (r.type == INT)
	{
		printint(r.number);
	}
	else if (r.type == STRING)
	{
		putch('"');
		printstr(r.string);
		putch('"');
	}
	else if (r.type == SYMBOL)
	{
		printstr(r.string);
	}
	else if (r.type == BOOL)
	{
		if ( r.number==0)
			printstr("False");
		else
			printstr("True");
	}
	else if (r.type == EMPTY)
	{
		printstr("NIL");
	}
	else if (r.type == FUNCTION || r.type == SPECIAL)
	{
		printstr("FUNCTION");	
	}
	else	
	{		
		printline("? error - type ");	
	}
	return;
}

tOBJ print(tOBJ r)
{
	if (r.type == CELL)
	{
		struct cell  *c = r.cell;
		printstr("(");	
		print(c->head);
		
		while (c->tail != null)
		{
			c=c->tail;
			printstr(" ");
			print(c->head);
		}
		//if (c->tail != EMPTY)
		//{
		//	printstr(".");
		//	print(c->tail);
		//}
		printstr(")");
	}
	else
	{
		printtype(r);
	}
	return r;
}

/**************************************************************************************************/
tOBJ get(char *n);

tOBJ callFunction(tCELL *x)
{
	tOBJ r;
	r.type=CELL;
	r.cell=x;
	
	if (x->head.type == FUNCTION || x->head.type == SPECIAL)
	{
		// call function	
		PFP f = (PFP)x->head.func;
		r = (*f)(x->tail);
	}
	else
	{
		printtype(x->head);
		printline(" :: Error - must be function");
		r.type=EMPTY;
	}
	return r;
}

/**********  primitives  *********/

void showenviron();

tOBJ env(tCELLp list)
{
	tOBJ r; r.type=EMPTY;
	showenviron();
	return r;
}

tOBJ car(tCELLp p)   // i.e. (car  '(123 456)) => 123
{
	tOBJ r;
	r.type=EMPTY;

	if (p->head.type==CELL)
	{
		p = p->head.cell;
		return p->head;
	}
	return r;
}

tOBJ cdr(tCELLp p) // i.e. (cdr  '(123 456)) => (456)
{
	tOBJ r;
	r.type=EMPTY;

	if (p->head.type==CELL)
	{
		p = p->head.cell;	
		r.type = CELL;
		r.cell = p->tail;
	}
	return r;
}

tOBJ cadr(tCELLp p) // i.e. (cdr  '(123 456)) => 456
{
	tOBJ r = car (cdr(p).cell);
	return r;
}

tOBJ plus(tCELLp p) // i.e. (plus 123 456) => 597
{
	tOBJ r;
	r.type=INT;
	r.number=0;
	
	int n=0;
	if (p->head.type == INT)
	{
		n = p->head.number + plus(p->tail).number;
	}
	r.number=n;
	return r;
}


tOBJ pr(tCELLp p)  // i.e. (pr "abc" 123) => abc123
{
	tOBJ r;
	r.type=EMPTY;
	
	print(p->head);
	if (p->tail==null)
		return p->head;
	r=pr(p->tail);

	return r;
}

tOBJ prn(tCELLp p)
{
	tOBJ r= pr(p);
	printline("");
	return r;
}


tOBJ eq(tCELLp p)  // i.e. (eq 1 1) => true, (eq 1 2) => nil
{
	tOBJ r, a, b;
	r.type=BOOL;
	r.number=0;  // assume false
	
	a = p->head;
	b = p->tail->head;
	
	if (a.type == b.type)
	{
		if (a.type == STRING || a.type == SYMBOL)
		{
			r.number = (strcmp(a.string,b.string)==0)?1:0;
		}
		if (a.type == INT || a.type == BOOL)
		{
			r.number = (a.number==b.number)?1:0;
		}
		if (a.type == FUNCTION || a.type == SPECIAL)
		{
			r.number = (a.func==b.func)?1:0;
		}
		if (a.type == EMPTY)
		{
			r.number = 1;
		}
	}
	return r;
}

tOBJ call(tCELLp p);
tOBJ callobj(tOBJ h) ;


tOBJ iff(tCELLp p)  // i.e. (if (eq 1 2) (prn "One") true (prn "Two")) => Two
{
	tOBJ r;
	r.type=EMPTY;
	
	if (p == null) return r;
	
	//printstr("test ");printtype(p->head); 	
	r = callobj(p->head);
	
	if ((r.type == BOOL && r.number==1) || (r.type != EMPTY && r.type != ERROR && r.type != BOOL )) 
	{	
		tCELLp x = p->tail;
		if (x != null)
		{
			//printstr(" eval :: ");printtype(x->head); 	
			r = callobj(x->head);

		}
		return r;
	}
	else
	{
		p=p->tail;
		r=iff(p->tail);
		return r;		
	}
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
	
	//printstr("Add "); printstr(n); printstr("-"); printint(v.type); printline(""); 	//DEBUG
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


tOBJ setq(tCELLp p)   // i.e. (setq a 123 ) => 123 and set a 
{
	tOBJ r;
	r.type=EMPTY;

	tOBJ h = p->head;
	if (h.type == SYMBOL)
	{
		tOBJ t = (p->tail)->head;
		if (t.type==CELL)
		{
			t = callFunction(t.cell);
		}
		add(h.string, t);
		return t;
	}
	else
	{
		printtype(h);
		printline(" - Must be a symbol?");
	}
	return r;
}

tOBJ set(tCELLp p)   // i.e. (set 'a 123 'b 245 ) => 123 and set a 
{
	tOBJ r;
	r.type=EMPTY;

	tOBJ h = p->head;
	if (h.type == SYMBOL)
	{
		add(h.string, (p->tail)->head);
		r= (p->tail)->head;
	}
	else
	{
		printtype(h);
		printline(" - Must be a symbol?");
	}
	return r;
}


tOBJ callobj(tOBJ h)   // i.e. (eval '(plus 2 3)) -> 7 // (set 'a '(prn "hello")) (eval a) => "hello"
{
	tOBJ r ; r.type=EMPTY;
	
	if (h.type != CELL)
	{
		return h;
	}
	
	tCELLp p = (tCELLp)(h.cell);
	h=p->head;
	
	if (h.type==SYMBOL)
	{
		int fn = -1;
		int fc=0;
		for (fc=0; fc<PRIMSZ; fc++)
		{
			if (strcmp(prim_tab[fc].name, h.string)==0)
				fn=fc;
		}	
		
		if (fn>=0)
		{
			//  function	
			PFP f = (PFP)prim_tab[fn].func;
			r = (*f)(p->tail);
		}
		else
		{
			printline("Undefined ?");
		}
	}
	else
	{
		printtype(h);
		printline(" :: Not a symbol ?");
	}
	return r;
}

tOBJ call(tCELLp p)   // i.e. (eval '(plus 2 3)) -> 7 // (set 'a '(prn "hello")) (eval a) => "hello"
{
	return callobj(p->head);
}


/**************************************************************************************************/

extern volatile BYTE   gSEC;
extern volatile BYTE   gMIN;

void printtime(char * s)
{
	printstr (s); printnumber(gMIN,2,'0'); printstr(":"); printnumber(gSEC,2,'0'); 
}

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
		for (i=1; i<symCnt; i++)
		{
			printstr(", ");
			printstr(symb_tab[i].name);
		}
		printline("");	
	}
	
	printtime("Time: ");	printline("");	
	
	garbageCollect();
}

tOBJ evalstring(char *txt)
{
	ibcnt=0;
	while ((inputbuffer[ibcnt++] = *txt++ ) != 0) ;
	inputbuffer[ibcnt] = '\0';
	ibptr=inputbuffer;
	return eval(false);
}

void repl()
{
	while (true)
	{
		printstr("> ");
		readline();
		while (ibptr-inputbuffer < ibcnt)
		{
			struct object r = eval(false);
			print(r);	
			printline("");	
		}
	}
}

void testf()
{
   evalstring("(set 'a 1)");
   evalstring("(if true (prn \"hi\")))");
}

void initialise()
{
	printline("Femto 0.1");
	//testf();  //DEBUGING
	showenviron();						
}

#define	RUN_LED1_ON			CLR_BIT5(PORTA)     // BLUE
#define	RUN_LED2_ON			CLR_BIT6(PORTA)     // GREEN

void femto()
{
	initialise();

	RUN_LED1_ON;
	RUN_LED2_ON;	
	UCSR1B= (1<<RXEN)|(1<<TXEN) ; //enable PC read/write Not interupt;	
	repl();
}