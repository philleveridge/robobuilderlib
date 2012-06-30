#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "win.h"
#endif

#ifdef LINUX
#include <math.h>
#include "linux.h"

#define NULL (void *)0
#endif

#include "express.h"
#include "functions.h"
#include "lists.h"
//
// The new expression parser
// work in progress
//

extern void sigcatch();
extern int  matrixload(int n);
extern int  matrixstore(int n);
extern void loadimage(char *ifile, int x, int *f);
extern void filterimage(char *ifile, int *data, int x, int a, int b, int c, int d, int e, int f);

#define iswhite(c)   (c == ' ' || c == '\t')
#define isnumdot(c)  ((c >= '0' && c <= '9') || c == '.')
#define isnumber(c)  (c >= '0' && c <= '9')

#define isdelim(c)  (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '<'  || c == '>'  \
                    || c == '^' || c == '(' || c == ')' || c == ',' || c == '='  || c == '$')

#define isalpha(c)  ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')


#define MAXSTACK 50

char varname[256];
tOBJ varobj[50];
int nov=0;


char exprbuff[64];
char *e;
char tokbuff[5];
int  tb;

int    tnum;
double tfloat;

typedef struct ops {
        char	*name;
		int		level;
		unsigned char type;
		int		nop;
		void	(*func)();
} tOP, *tOPp;

void osin ();
void opsd ();
void omax ();
void oprint ();

tOBJ get(char *name);
int  set(char *name, tOBJ r);

tOP oplist[] = { 
	{"+",    10, PLUS,  2, NULL},
	{"-",    10, MINUS, 2, NULL},
	{"/",    20, DIVD,  2, NULL},
	{"*",    20, MULT,  2, NULL},
	{"<",    5,  LT,    2, NULL},
	{">",    5,  GT,    2, NULL},
	{"=",    5,  EQL,   2, NULL},
	{"<>",   5,  NEQ,   2, NULL},
	{"(",    50, OBR,   1, NULL},
	{")",    50, CBR,   1, NULL},
	{",",    50, COMMA, 1, NULL},
	{"AND",  8,  LAND,  2, NULL},
	{"OR",   8,  LOR,   2, NULL},
	{"SIN",  40, NA,    1, osin},  //function single arg
	{"PSD",  40, NA,    0, opsd},  //const
	{"MAX",  40, NA,    2, omax},   //function two args
};

tOBJ omath(tOBJ o1, tOBJ o2, int op);
tOBJ print(tOBJ r);

/**********************************************************/
/*  strings                                              */
/**********************************************************/

char strings[256], *estr=&strings[0];
char *newstring(char *s)
{
	char *r=estr;
	if (estr-&strings[0]>254)
		return 0;
	strcpy(estr, s);
	estr=estr+strlen(s);
	*estr++='\0';
	return r;
}

/**********************************************************/
/*  stack                                                 */
/**********************************************************/

tOBJ stackobj[MAXSTACK];
int sop=0;

unsigned char stackop[MAXSTACK];
int oop=0;

int push(tOBJ a)
{
	if (sop<MAXSTACK-1)
	{
		stackobj[sop++] = a;
		return 1;
	}
	return 0;
}

tOBJ pop()
{
	tOBJ e;
	e.type=EMPTY;

	if (sop>0)
	{
		e=stackobj[sop-1];
		sop--;
	}
	return e;
}

tOBJ peek(int n)
{
	tOBJ e;
	e.type=EMPTY;

	if (sop-1-n >= 0)
	{
		return stackobj[sop-1-n];
	}
	return e;
}

void clear()
{
	sop=0;
}

void stackprint()
{
	int i;
	rprintfStr ("Stack\n");
	for (i=0;i<sop;i++)
	{
		rprintf ("%d ", i);
		print(stackobj[i]);
		rprintfCRLF();
	}
}

int stacksize()
{
	return sop;
}

/**********************************************************/

int getOP(char *str)
{
	int i=0;
	while (i<sizeof(oplist)/sizeof(tOP))
	{
		//printf ("%d %s\n", i,oplist[i].name);
		if (!strcmp(oplist[i].name,str))
			return i;
		i++;
	}
	return -1;
}

int get_token() 
{
	int ty=DELI;
	tb=0;
    while (iswhite(*e))     // space or tab
    {
        e++;
    }

    // check for end of expression
    if (*e == '\0')
    {
        return DELI;
    }

	if (isnumdot(*e))
	{
		int r=NUMI;
		double dn=0.1;
		tnum=0;
		while (isnumdot(*e))
		{
			if (*e=='.') 
			{
				r=NUMF;
				tfloat=(double)tnum;
			}
			else
			{	
				if (r==NUMI)
					tnum  = tnum*10 + (*e-'0');
				else
				{
					tfloat= tfloat + dn * (*e-'0'); 
					dn=dn/10;
				}
			}
			e++;
		}
		return r;
	}

	if (isdelim(*e))
	{
		if (*e=='<' && *(e+1)=='>')
		{
			tokbuff[tb++]=*e++;
		}

		tokbuff[tb++]=*e++;
		tokbuff[tb]=0;

		if (getOP(tokbuff)<0)
			return DELI;
		else
			return OPR;
	}

	if (isalpha(*e))
	{
		tokbuff[tb++]=toupper(*e++);
		while (isalpha(*e) || isnumber(*e))
		{
			tokbuff[tb]=toupper(*e++);
			tb=tb+1;
			if (tb>4) tb=4;
		}
		tokbuff[tb]=0;

		if (getOP(tokbuff)<0)
			return ALPHA;
		else
			return OPR;
	}

	return ty;
}


void reduce()
{
	tOBJ a,b;
	int i;

	if (oop<=0)
		return; //nothing to do

	i=stackop[--oop];

	if (oplist[i].nop==2 && oplist[i].func == NULL)
	{
		b = pop();
		a = pop();
		a = omath(a, b, oplist[i].type);
		push(a);
	}
	
	if (oplist[i].nop==1 && oplist[i].func == NULL)
	{
		//
	}

	if (oplist[i].func != NULL)
	{
		if(oplist[i].nop>stacksize())
		{
			rprintf("Insuffcient parameters\n");
		}
		else
			(*oplist[i].func)();
	}
}

tOBJ eval_oxpr(char *s)
{
	tOBJ r;
	int op=NA;
	int lf=1;
	int tk=0;
	int tmp;

	r.type=EMPTY;
	strncpy(exprbuff,s,63);
	e=exprbuff;

	while (lf)
	{
		tk = get_token();
		switch (tk)
		{
		case ALPHA:
			r = get(tokbuff);
			push(r);
			break;
		case NUMI:
			r.type   = INTGR;
			r.number = tnum;
			push(r);
			break;
		case NUMF:
			r.type   = FLOAT;
			r.floatpoint = tfloat;
			push(r);
			break;
		case OPR:
			op = getOP(tokbuff);
			if (oplist[op].nop==0 && oplist[op].func != NULL)
			{
				(*oplist[op].func)();
				continue;
			}
			else
			if (oop>0 && (oplist[op].type == CBR || oplist[op].type==COMMA))
			{
				int i=stackop[oop-1];
				while (oplist[i].type != OBR && oplist[i].type != COMMA)
				{
					//stackprint();
					reduce();
					i=stackop[oop-1];
				}
				continue;
			}
			else
			if (oop>0)
			{
				int i = stackop[oop-1];
				if ((oplist[i].type != OBR) && (oplist[op].level<=oplist[i].level) )
				{
					reduce();
				}
			}
			stackop[oop++]=op;
			break;
		case DELI:
			lf=0; 
			continue;
		}
	}

	while (oop>0)
	{
		reduce();
	}

	if (stacksize()>1)
	{
		printf("Suntax error - too many on stack\n");
		stackprint();
		clear();
	}
	return pop();
}

void printtype(tOBJ r)
{
    if (r.type == INTGR)
    {
            rprintf("%d", r.number);
    }
	else if (r.type == FLOAT)
    {
            rprintf("%f", r.floatpoint);
    }
    else if (r.type == STR)
    {
            rprintfStr(r.string);
    }
    else if (r.type == SYM)
    {
            rprintfStr(r.string);
    }
    else if (r.type == BOOLN)
    {
        if ( r.number==0)
                rprintfStr("False");
        else
                rprintfStr("True");
    }
    else if (r.type == EMPTY)
    {
        rprintfStr("NIL");
    }
    else if (r.type == FUNC)
    {
        rprintfStr("FUNCTION");   
    }
    else    
    {               
        rprintfStr("? error - type\n");   
    }
    return;
}

tOBJ omath(tOBJ o1, tOBJ o2, int op)
{
	tOBJ r;
	r.type=EMPTY;

	// print(o1); rprintfStr(" "); print(o2); rprintf(" %d\n", op); //debug info

	if (o1.type==INTGR && o2.type==INTGR)
	{
		r.type=INTGR;
		switch (op)
		{
		case PLUS:
			r.number = o1.number + o2.number; break;
		case MINUS:
			r.number = o1.number - o2.number; break;
		case MULT:
			r.number = o1.number * o2.number; break;
		case DIVD:
			r.number = o1.number / o2.number; break;
		case LAND:
			r.number = o1.number && o2.number; break;
		case LOR:
			r.number = o1.number || o2.number; break;
		case EQL:
			r.number = o1.number == o2.number; break;
		case NEQ:
			r.number = o1.number != o2.number; break;
		case LT:
			r.number = o1.number < o2.number; break;
		case GT:
			r.number = o1.number > o2.number; break;
		default:
			r.type=EMPTY; //error
			break;
		}
	}

	if (o1.type==FLOAT || o2.type==FLOAT)
	{
		double a=0.0,b=0.0;

		r.type=FLOAT;
		if (o1.type==FLOAT) a=o1.floatpoint;
		if (o1.type==INTGR) a=(double)o1.number;

		if (o2.type==FLOAT) b=o2.floatpoint;
		if (o2.type==INTGR) b=(double)o2.number;

		switch (op)
		{
		case PLUS:
			r.floatpoint = a + b; break;
		case MINUS:
			r.floatpoint = a - b; break;
		case MULT:
			r.floatpoint = a * b; break;
		case DIVD:
			r.floatpoint = a / b; break;
		case LT:
			r.type=INTGR; r.number = a < b; break;
		case EQL:
			r.type=INTGR; r.number = a == b; break;
		case NEQ:
			r.type=INTGR; r.number = a != b; break;
		case GT:
			r.type=INTGR; r.number = a > b; break;
		default:
			r.type=EMPTY; //error
			break;
		}
	}
	return r;
}

tOBJ get(char *name)
{
	int i=0;
	char *p=varname;
	tOBJ r;
	r.type=EMPTY;

	if (strlen(name)==1 && isalpha(*name)) //backwards compat
	{
		r.type=INTGR;
		r.number=getvar(*name-'A');
		return r;
	}

	while (i<nov)
	{
		if (strcmp(p,name)==0)
		{
			return varobj[i];
		}
		p=p+strlen(p)+1;
		i++;
	}
	return r;
}

int set(char *name, tOBJ r)
{
	int n = 0;
	char *p=varname;

	if (strlen(name)==1 && isalpha(*name)) //backwards compat
	{
		if (r.type==INTGR)
			setvar(*name-'A',r.number);
		if (r.type==FLOAT)
			setvar(*name-'A',(long)r.floatpoint);
		if (r.type==EMPTY)
			setvar(*name-'A',0);
		return 1;
	}

	while (n<nov)
	{
		if (strcmp(p,name)==0)
		{
			varobj[n]=r;
			return 1;
		}
		p=p+strlen(p)+1;
		n++;
	}
	if (nov<50)
	{
		varobj[nov++]=r;
		strcpy(p,name);
		p=p+strlen(p)+1;
		*p='\0';
		return 0; //added
	}
	else
		return -1;
}


void osin()
{
	tOBJ r,a;
	r.type=FLOAT;
	r.floatpoint=0.0;
	a=pop();
	if (a.type==INTGR)
	{
		r.type = INTGR;
		r.number = Sin(a.number%256);
	}

	if (a.type==FLOAT)
	{
		r.floatpoint=sin(a.floatpoint);
	}
	push(r);
	return ;
}

void opsd()
{
	tOBJ r;
	r.type=INTGR;
	r.number=50;
	push(r);
	return;
}

void oprint()
{
	int i=stacksize();
	while (i>0)
	{
		print(peek(--i));
		rprintfStr(" ");
	}
	rprintfCRLF();
	clear();
	return;
}


void omax()
{
	tOBJ r,a;
	r=pop();
	a=pop();

	if (r.type==INTGR && a.type==INTGR)
	{
		if (a.number>r.number) r=a;
	}
		
	if (r.type==FLOAT && a.type==FLOAT)
	{
		if (a.floatpoint>r.floatpoint) r=a;
	}

	push(r);
	return;
}

tOBJ print(tOBJ r)
{
    if (r.type == CELL)
    {
        struct cell  *c = r.cell;
        rprintfStr("(");  
        print(c->head);
                
        while (c->tail != (void *)0)
        {
                c=c->tail;
                rprintfStr(" ");
                print(c->head);
        }
        rprintfStr(")");
    }
    else
    {
		printtype(r);
    }
    return r;
}

int get_str_token(char *s)
{
	int tk = get_token();
	
	if (s==(char *)0 && tk==ALPHA)
		return 1;

	if (tk==ALPHA && !strcmp(tokbuff,s))
		return 1;
	return 0;
}

int get_integer()
{
	int tk = get_token();
	
	if (tk==NUMI)
		return tnum;;
	return 0;
}

int get_opr_token(unsigned char op)
{
	int tk = get_token();
	if (tk==OPR && oplist[getOP(tokbuff)].type==op)
		return 1;
	return 0;
}

static int  intf=1;

extern int   nis;

extern int   gotoln(int gl);
extern void  setline(WORD p);
extern int   BasicErr;

void extend(char *x)
{
	tOBJ v;
	if (intf) {
		intf=0; v.type=FLOAT; v.floatpoint=3.1415926; set("PI", v);
	}
		
	v.type=EMPTY;

	e=x;
	if (get_str_token("LET")==1)
	{
		if (get_str_token(NULL)==1)
		{	
			char var[5];
			strncpy(var,tokbuff, 5);
			if (get_opr_token(EQL)==1)
			{
				v=eval_oxpr(e);
				set(var,v);
				return;
			}	
		}
	}

	e=x;
	if (get_str_token("PRIN")==1)
	{
		while (*e!='\0')
		{
			v=eval_oxpr(e);
			print(v);
			if (*e!=';') 
			{
				rprintfCRLF();
			}
			else
				e++;
		}
		return;
	}

	e=x;
	if (get_str_token("MAT")==1)
	{
		if (get_str_token("LOAD")==1)
		{
			if (*e != '\0')
				v=eval_oxpr(e);
			else
				v.number=128;
			matrixload(v.number);
		}
		if (!strcmp(tokbuff,"STOR"))
		{
			if (*e != '\0')
				v=eval_oxpr(e);
			else
				v.number=nis;
			matrixstore(v.number);
		}

		return;
	}

	e=x;
	if (get_str_token("IMAG")==1)
	{
		if (get_str_token("LOAD")==1)
		{
			if (*e != '\0')
				v=eval_oxpr(e);
			else
				v.number=8;
			loadimage("test.jpg", (int)(v.number), &scene[0]);
			nis=v.number*v.number;

		}
		if (!strcmp(tokbuff,"FILT"))
		{
			//!IMAGE FILTER 4;1;10;10;100;100;100

			int args[6];
			int sz;

			v=eval_oxpr(e);
			sz=v.number;
			if (*e++ != ';')
				return;

			for (int i=0; i<6; i++)
			{
				v=eval_oxpr(e);
				args[i]=v.number;
				if (*e == ';' || (i==5 && *e=='\0'))
				{
					e++;
				}
				else
				{
        				rprintfStr("incorrect number args (6)\n"); 
					return;
				}
			}			
			filterimage("test.jpg",&scene[0],sz,args[0],args[1],args[2],args[3],args[4],args[5]);
			nis=sz*sz;
		}

		return;
	}


	e=x;
	if (get_str_token("EXIT")==1)
	{
		sigcatch();
	}
/*
	e=x;
	if (get_str_token("IF")==1)
	{
			int n,t;
			v=eval_oxpr(e);
			if (get_str_token("THEN")==1)
			{
				n=get_integer();
				if (v.type==NUMI && v.number==0 && get_str_token("ELSE")==1)
				{
					n=get_integer();
				}
				t = gotoln(n);
				if (t<0)
					BasicErr=5;
				else
					setline(t);

				return;
			}
	}
*/
	rprintfStr("No match ?\n");
}

void testeval()
{
	while (1)
	{
		readLine(exprbuff);
		if (exprbuff[0]=='.') break;

		extend(exprbuff);

	}
}


