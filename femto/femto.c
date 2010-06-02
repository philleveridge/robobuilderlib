#include <avr/io.h>
#include "Macro.h"

#define	RUN_LED1_ON		CLR_BIT5(PORTA)     // BLUE
#define	RUN_LED2_ON		CLR_BIT6(PORTA)     // GREEN

#define	RXC				7
#define RX_COMPLETE		(1<<RXC)
#define HEADER			0xFF 
#define TIME_OUT		100
#define NULL			'\0'

int dbg=0;
#define DEBUG(a)   if(dbg) {a;}

extern int 		strcmp(char *, char *);

extern volatile WORD   gticks;
extern volatile WORD   g10MSEC;
extern volatile WORD   g10Mtimer;
extern volatile BYTE   gSEC;
extern volatile BYTE   gMIN;
extern void     delay_ms(int ms);

/**************************************************************************************************/

extern void putByte (BYTE b);
extern void putWck  (BYTE b);

int getByte()
{
	while(!(UCSR1A & RX_COMPLETE)) ;
	return UDR1;
}

int wckGetByte(WORD timeout)
{
	g10Mtimer = timeout;
	
	while(!(UCSR0A&(1<<RXC)) ){ 	// test for received character
		if (g10Mtimer==0) return -1;
	}
	return UDR0;
}

/******************************************************************************/
/* Function that sends Operation Command Packet(4 Byte) to wCK module */
/* Input : Data1, Data2 */
/* Output : None */
/******************************************************************************/
void wckSendOperCommand(char Data1, char Data2)
{
	char CheckSum;
	CheckSum = (Data1^Data2)&0x7f;
	putWck(HEADER);
	putWck(Data1);
	putWck(Data2);
	putWck(CheckSum);
}

void wckSendSetCommand(char Data1, char Data2, char Data3, char Data4)
{
	char CheckSum;
	CheckSum = (Data1^Data2^Data3^Data4)&0x7f;
	putWck(HEADER);
	putWck(Data1);
	putWck(Data2);
	putWck(Data3);
	putWck(Data4);
	putWck(CheckSum);
}

void wckSyncPosSend(BYTE LastID, BYTE SpeedLevel, BYTE *TargetArray, BYTE Index)
{
	int i;
	char CheckSum;
	i = 0;
	CheckSum = 0;
	putWck(HEADER);
	putWck((SpeedLevel<<5)|0x1f);
	putWck(LastID+1);
	while(1) 
	{
		if(i>LastID) 
			break;
		putWck(TargetArray[Index*(LastID+1)+i]);
		CheckSum = CheckSum ^ TargetArray[Index*(LastID+1)+i];
		i++;
	}
	CheckSum = CheckSum & 0x7f;
	putWck(CheckSum);
}

/**************************************************************************************************/

char *errmessage[] = {
	"not enough items",	
	"must be number",
	"must be list",
	"incorrect parameters passed",
	"must be a symbol",
	"must be a function"
};

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

		if (ch == 13) 
		{
			putByte(13); putByte(10);
		    break;
		}
		if (ch==8 || ch==127)   //Bsapce ?
		{
			if (ibcnt>0) 
			{
				putByte(ch);
				ibcnt--;
			}	
		}
		else
		{
			putByte(ch);            //echo input
			inputbuffer[ibcnt++] = ch;
		}
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

enum  TYPE {SYMBOL, INT, BOOL, FUNCTION, SPECIAL, FLOAT, STRING, CELL, EMPTY, ERROR};

typedef struct object {
	int   type;
	bool  q;
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

tOBJ time  (tCELLp);
tOBJ sleep (tCELLp);
tOBJ whle  (tCELLp);

tOBJ car  (tCELLp);
tOBJ cdr  (tCELLp);
tOBJ cons (tCELLp);
tOBJ list (tCELLp);

tOBJ plus(tCELLp);
tOBJ pr  (tCELLp);
tOBJ prn (tCELLp);
tOBJ set (tCELLp);
tOBJ setq(tCELLp);
tOBJ env (tCELLp);
tOBJ eq  (tCELLp);
tOBJ iff (tCELLp);
tOBJ call(tCELLp);
tOBJ progn(tCELLp);
tOBJ prog(tCELLp);
tOBJ len (tCELLp);
tOBJ cclone (tCELLp);

tOBJ sendServo   (tCELLp);
tOBJ getServo    (tCELLp);
tOBJ passiveServo(tCELLp);
tOBJ synchServo  (tCELLp);
tOBJ readIR      (tCELLp);
tOBJ wckwriteIO  (tCELLp); //(n c0 c1)

struct prim { char *name; PFP func; int sf;} prim_tab[] = { 
	{"env",  env, 0},
	{"time", time,1},
	{"sleep",sleep,0},
	{"car",  car, 0},
	{"plus", plus,0}, 	
	{"cons", cons,0},
	{"list", list,0}, 
	{"length", len,0}, 
	
	{"cdr",  cdr, 0},
	{"pr",   pr,  0},
	{"set",  set, 0},
	{"setq", setq,1},
	{"prn",  prn, 0},
	{"eq",   eq,  0},
	{"if",   iff, 1},
	{"eval", call,0},
	{"do1",  prog,0},	
	{"do",   progn,0},	
	{"clone",cclone,0},

	{"sendServo",   sendServo, 0},
	{"getServo",    getServo,  0},
	{"passServo",   passiveServo,0},
	{"syncServo",   synchServo,0},
	{"wckwriteIO",  wckwriteIO,0},
	{"readIR",      readIR,    0},

	{"while",whle,1}
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
	
	if (dbg==0) return;
	
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
		
		if (memory[i].tail != null)
		{
			int n = (memory[i].tail-&memory[0]); ///sizeof(tCELL);
			printstr("->#"); printint(n);
		}
		else
		{
			printstr("->null");
		}
		
		printline("");
	}

}

/**************************************************************************************************/

int readdigit()
{
	int num = 0;
	int ch  = 0;
	int s   = 1;
	switch(ch=getch())
	{
		case '-': s=-1; break;
		case '+': s=1; break;
		default: ungetch(ch); break;
	}
	
	while ((ch = getch()) > 0)
	{
		if (!(ch>='0' && ch<='9'))
		{
			ungetch(ch);
			return num*s;
		}
		num = num * 10 + (ch - '0');
	}
	return num*s;
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

tOBJ readtoken()
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
	r.q = false;
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
			r.type = prim_tab[fn].sf==0?FUNCTION:SPECIAL;
			r.func = prim_tab[fn].func;
		}
		else
		{
			r.string = newString(n, str);
			r.type   = SYMBOL;
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

tOBJ parse();
tOBJ callFunction(struct cell *);

tCELL *parselist()
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
		nxt->head = parse();
		nxt->tail = null;
		prv=nxt;
	}
	return top;
}
	

tOBJ parse()
{
	tOBJ r;
	r.type=EMPTY;
	r.q = false;
	
	int ch;
	bool qf=0;
	
	while ((ch = getch()) > 0)
	{
		if (isWhiteSpace((char)ch))
		{
			qf = false;
			readwhitespace();
			continue;
		}

		if (ch=='+' || ch=='-' || (ch >= '0' && ch <= '9'))
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
			tCELL *args = parselist();
			r.q = qf;
			r.type = CELL;
			r.cell = args;

			return r;
		}

		if ( (ch>= 'A' && ch<= 'Z' ) || (ch>= 'a' && ch<= 'z' ))
		{
			ungetch(ch);
			r=readtoken();		
			r.q = qf;
			return r;
		}
	}
	return r;
}

void printtype(tOBJ r)
{

	if (r.type == CELL)
	{
		if (r.q == true) printstr("'");
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
		printstr(r.string);
	}
	else if (r.type == SYMBOL)
	{
		if (r.q == true) printstr("'");
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
	else if (r.type == ERROR)
	{
		printstr("Error [");printint(r.number);printstr("] :: ");
		if (r.number<sizeof(errmessage)) printstr(errmessage[r.number]);
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
		if (r.q == true) printstr("'");
		printstr("(");	
		print(c->head);
		
		while (c->tail != null)
		{
			c=c->tail;
			printstr(" ");
			print(c->head);
		}
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


/**********  primitives  *********/

void showenviron();

tOBJ throw(int n)
{
	tOBJ r; r.type=ERROR; r.number=n;
	return r;
}

tOBJ env(tCELLp list)
{
	tOBJ r; r.type=EMPTY;
	if (list != null && list->head.type==INT) dbg = list->head.number;
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

tOBJ cadr(tCELLp p) // i.e. (cadr  '(123 456)) => 456
{
	tOBJ r = car (cdr(p).cell);
	return r;
}

tOBJ len(tCELLp p) // i.e. (len  '(123 456)) => 2
{
	tOBJ r; r.type=INT;
	if (p->head.type != CELL)
		r=throw(2);
	else
	{
		int n=0;
		p=p->head.cell;
		while (p != null)
		{
			n++;
			p=p->tail;
		}
		r.number=n;
	}
	return r;
}

tOBJ cons(tCELLp p) // i.e. (cons  123 '(456)) => (123 456)
{
	tOBJ r ; r.type=EMPTY;
	tOBJ a = p->head;
	tOBJ b = p->tail->head;
	if (a.type ==CELL || b.type != CELL)
	{
		r.type=EMPTY; //error
	}
	else
	{
		tCELLp n = newCell();
		n->head=a;
		n->tail = (tCELLp)(b.cell);
		r.type = CELL;
		r.cell = n;
	}
	return r;
}

tOBJ list(tCELLp p) // i.e. (list 123 456) => (123 456)
{
	tOBJ r ; r.type=CELL; 
	r.cell = p;
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
		if (a.type == FUNCTION || a.type == SPECIAL )
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
	
	r = callobj(p->head);
	
	if ((r.type == BOOL && r.number==1) || (r.type != EMPTY && r.type != ERROR && r.type != BOOL )) 
	{	
		tCELLp x = p->tail;
		if (x != null)
		{
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


tOBJ set(tCELLp p)   // i.e. (set 'a 123 'b 245 ) => 123 and set a 
{
	tOBJ r;
	r.type=EMPTY;

	tOBJ h = p->head;
	if (h.type == SYMBOL)
	{
		add(h.string, (p->tail)->head);
		r= (p->tail)->head;
		
		if (p->tail->tail != null)
		{
			return set(p->tail->tail);
		}
	}
	else
	{
		DEBUG(printtype(h);)
		r=throw(4);
	}
	return r;
}

tOBJ setq(tCELLp p)   // i.e. (set 'a 123 'b 245 ) => 123 and set a 
{
	tOBJ r;
	r.type=EMPTY;

	tOBJ h = p->head;
	
	if (h.type == SYMBOL)
	{
		add(h.string, callobj((p->tail)->head));
		r= (p->tail)->head;
		
		if (p->tail->tail != null)
		{
			return setq(p->tail->tail);
		}
	}
	else
	{
		DEBUG(printtype(h);)
		r=throw(4);
	}
	return r;
}


tCELLp clone(tCELLp p)  //(clone 1 2 '(3))-> (1 2 (3))
{
	tCELLp n;	
	tCELLp t=null;
	tCELLp r=null;
	
	if (p==null) return null;
	
	while (p != null)
	{
		n = newCell();
		if (r == null) r=n;
		if (t != null) t->tail=n;
		n->head = p->head;
		if (p->head.type == CELL)
		{
			n->head.cell = clone((tCELLp)(p->head.cell));
		}
		p=p->tail;
		t=n;
	}
	return r;
}

tOBJ cclone(tCELLp p)
{
	tOBJ r; r.type=CELL;
	r.cell=clone(p);
	return r;
}

tOBJ oclone(tOBJ o)
{
	if (o.type == CELL)
		return cclone(o.cell);
	return o;
}

tOBJ callobj(tOBJ h)   // i.e. (eval '(plus 2 3)) -> 7 // (set 'a '(prn "hello")) (eval a) => "hello"
{
	tOBJ r ; r.type=EMPTY;
	
	DEBUG(printstr("DEBUG:: ["); printtype(h); printline("]"))
	
	if (h.q == true)
	{
		DEBUG(printline("QUOTE"))
		h.q=false; //unquote
		return h;
	}
	
	if (h.type != CELL )
	{
		if (h.type == SYMBOL)
		{
			// user define it?		
			r = get(h.string);
			
			if (r.type==EMPTY)
			{
				printstr("Not defined? :: ");
				printline(h.string);
			}
			else
				return r;
		}
		return h;
	}
	
	tCELLp p = (tCELLp)(h.cell);
	h=p->head;
	
	// first element in a lis must be a function
	
	if (h.type==FUNCTION || h.type == SPECIAL)
	{
		int i;
		//  function	
		PFP f = (PFP)h.func;
		
		tCELLp tp =p->tail;
		
		// eval tail
		int ts=0;
		tOBJ temp[50];
				
		if (h.type == FUNCTION)
		{
			while ((p=p->tail) != null && ts<49)
			{ 
				temp[ts] = p->head;  //?memcpy???
				
				p->head = callobj (p->head);
				ts++;
			}
		}
		r= (*f)(tp);
		
		p=tp;

		for (i=0;i<ts; i++)
		{
			//DEBUG(printint(i);printstr("-"); printint(temp[i].type);printtype(temp[i]);printline("");)
			p->head = temp[i];
			p=p->tail;
		}
	}
	else
	{
		DEBUG(printtype(h);)
		return throw(5);
	}
	return r;
}

tOBJ prog(tCELLp p)   // i.e. (do1 (prn "hello") (prn "world") -> "hello")
{
	tOBJ r; r.type=EMPTY;
	if (p != null)
		r= p->head;
	return r;
}

tOBJ progn(tCELLp p)   // i.e. (do (prn "hello") (prn "world") -> "world")
{
	tOBJ r; r.type=EMPTY;
	while (p != null)
	{
		r=p->head;
		p=p->tail;
	}
	return r;
}

tOBJ whle(tCELLp p)  
{
	// (while (eq -1 (readIR)) (do (prn "loop - IR=" (readIR)) (sleep 100)))
	tOBJ cond = p->head;
	tOBJ fn   = p->tail->head;
	tOBJ r; r.type=EMPTY;
	
	tOBJ t = callobj(cond);
		
	while ((t.type == BOOL && t.number==1) || (t.type != EMPTY && t.type != ERROR && t.type != BOOL )) 
	{
		r = callobj(fn);
		t = callobj(cond);
	}
	
	return r;
}

tOBJ time(tCELLp p)   // i.e. (time (sleep 500))  => 500
{
	tOBJ r; r.type=INT;
	gticks = 0;
	callobj(p->head);
	r.number=gticks;
	return r;
}

tOBJ sleep(tCELLp p)   // i.e.(sleep 500) 
{
	tOBJ r; r.type=EMPTY;
	
	if (p->head.type == INT)
	{
		delay_ms(p->head.number);
	}
	return r;
}

//define ir.c
extern int irGetByte(void);

tOBJ readIR(tCELLp p)   
{
	tOBJ r; r.type = INT;
	r.number = irGetByte();
	return r;
}

tOBJ call(tCELLp p)   // i.e. (eval '(plus 2 3)) -> 7 // (set 'a '(prn "hello")) (eval a) => "hello"
{
	tOBJ r = callobj(p->head);
	if (p->tail != null)
		return call(p->tail);
	return r;
}

/**************************************************************************************************/

tOBJ getServo(tCELLp p)   // i.e. (getServo 15)
{
	tOBJ r; r.type=INT;
	if (p->head.type==INT && p->head.number>=0 && p->head.number<=31)
	{
		wckSendOperCommand(0xa0|(p->head.number), NULL);
		int b1 = wckGetByte(TIME_OUT);
		int b2 = wckGetByte(TIME_OUT);
		
		if (b1<0 || b2<0)
			r.type = EMPTY; //timeout
		else
			r.number = b2;
	}
	else
		r.type=EMPTY;
	return r;
}

tOBJ wckwriteIO(tCELLp p)   // i.e. (wckwriteIO 15 true true)
{
	tOBJ r; r.type=INT;
	if (p->head.type==INT && p->head.number>=0 && p->head.number<=31)
	{
		int id = p->head.number;
		p=p->tail;
		if (p->head.type != BOOL) return throw(3);
		int b1 = p->head.number;
		p=p->tail;
		if (p->head.type != BOOL) return throw(3);
		int b2 = p->head.number;
		
        wckSendSetCommand((7 << 5) | (id % 31), 0x64, ((b1) ? 1 : 0) | ((b2) ? 2 : 0), 0);
		
		b1 = wckGetByte(TIME_OUT);
		b2 = wckGetByte(TIME_OUT);
		
		if (b1<0 || b2<0)
			r.type = EMPTY; //timeout
		else
			r.number = b2;		
	}
	else
		r=throw(3);
	return r;
}

tOBJ passiveServo(tCELLp p)   // i.e. (passiveServo 15)
{
	tOBJ r; r.type=INT;
	if (p->head.type==INT && p->head.number>=0 && p->head.number<=31)
	{
		wckSendOperCommand(0xc0|(p->head.number), 0x10);
		int b1 = wckGetByte(TIME_OUT);
		int b2 = wckGetByte(TIME_OUT);
		
		if (b1<0 || b2<0)
			r.type = EMPTY; //timeout
		else
			r.number = b2;
	}
	else
		r.type=EMPTY;
	return r;
}
tOBJ sendServo(tCELLp p)   // i.e. (sendServo 12 50 4)
{
	tOBJ r; r.type=INT;
	int sid = p->head.number;
	p=p->tail;
	int pos = p->head.number;
	p=p->tail;
	int tor = p->head.number;
	if (sid<0 || sid>30 || pos<0 || pos>254 ||tor<0 || tor>4) 	
	{
		r=throw(3); // incorrect aparams
	}
	else	
	{
		wckSendOperCommand((tor<<5)|sid, pos);

		int b1 = wckGetByte(TIME_OUT);
		int b2 = wckGetByte(TIME_OUT);	

		if (b1<0 || b2<0)
			r.type = EMPTY; //timeout
		else
			r.number = (b1<<8|b2);
	}
	return r;
}

tOBJ synchServo(tCELLp p)   // i.e. (sycnServo lastid torq '(positions))
{
	tOBJ r;	r.type=EMPTY;
	BYTE lastid = (p->head.number)&0x1F;
	p=p->tail;
	BYTE tor    = (p->head.number)&0x03;
	p=p->tail;
	if (p->head.type == CELL)
	{
		BYTE pos[lastid+2];
		p=(tCELLp)(p->head.cell);
		
		for (int i=0; i<lastid+1; i++)
		{
			int n;
			if (p == null)
			{
				return throw(0);
			}
			if (p->head.type==INT)
			{
				n=p->head.number;							
				DEBUG(printint(n); printstr("-");)				
				pos[i] = n;
			}
			else
			{
				return throw(1);
			}

			p=p->tail;
		}
		wckSyncPosSend(lastid, tor, pos, 0); // uncomment if works!
	}
	else
	{
		return throw(2);
	}
	return r;
}

/**************************************************************************************************/

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

tOBJ eval()
{
	tOBJ r = parse();
	return callobj(r);
}

tOBJ evalstring(char *txt)
{
	ibcnt=0;
	while ((inputbuffer[ibcnt++] = *txt++ ) != 0) ;
	inputbuffer[ibcnt] = '\0';
	ibptr=inputbuffer;
	return eval();
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

void testf()
{
   evalstring("(set 'a 1)");
   evalstring("(if (eq a 1) (prn \"one\") true (prn \"hi\")))");
}

void initialise()
{
	printline("Femto $Rev: 152 $");
	RUN_LED1_ON;
	RUN_LED2_ON;	
	UCSR1B= (1<<RXEN)|(1<<TXEN) ; //enable PC read/write Not interupt;	
	UCSR0B= (1<<RXEN)|(1<<TXEN) ; //enable wck read/write Not interupt;	
	
	DEBUG(testf())  //DEBUGING
		
	showenviron();						
}


void femto()
{
	initialise();
	repl();
}