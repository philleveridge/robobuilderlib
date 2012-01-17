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

//
// The new expression parser
// work in progress
//

#define iswhite(c)   (c == ' ' || c == '\t')
#define isnumdot(c)  ((c >= '0' && c <= '9') || c == '.')
#define isnumber(c)  (c >= '0' && c <= '9')

#define isdelim(c)  (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '<'  || c == '>'  \
                    || c == '^' || c == '(' || c == ')' || c == ',' || c == '='  || c == '$')

#define isalpha(c)  ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')


#define MAXSTACK 10

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
        char *name;
		int level;
		int type;
		int nop;
		tOBJ (*func)(tOBJ);
} tOP, *tOPp;

tOBJ osin(tOBJ a);
tOBJ opi (tOBJ a);
tOBJ get(char *name);
int set(char *name, tOBJ r);

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
	{"AND",  8,  LAND,  2, NULL},
	{"OR",   8,  LOR,   2, NULL},
	{"SIN",  40, NA,    1, osin},  //function single arg
	{"PI",   40, NA,    0, opi },  //const
	{"MAX",  40, NA,    3, NULL},   //function list arg - tbd
};

tOBJ omath(tOBJ o1, tOBJ o2, int op);
tOBJ print(tOBJ r);

/**********************************************************/
/*  stack                                                 */
/**********************************************************/

tOBJ stackobj[MAXSTACK];
int sop=0;

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
	tOBJ r,a,b;
	//
	if (stacksize()>=2)
	{
		b = pop();
		r = pop();
		if (r.type!=INTGR) return;
		if (oplist[r.number].nop==2 && oplist[r.number].func == NULL)
		{
			a = pop();
			a = omath(a, b, oplist[r.number].type);
			push(a);
		}
		if (oplist[r.number].func != NULL)
		{
			b = (*oplist[r.number].func)(b);
			push(b);
		}
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
				r = (*oplist[op].func)(r);
				push(r);
				continue;
			}
			else
			if (oplist[op].type == CBR)
			{
				tOBJ t1 = peek(1);
				while (oplist[t1.number].type != OBR)
				{
					//stackprint();
					reduce();
					t1 = peek(1);
				}
				r=pop();  //temp pop result
				pop();    //pop of OBR
				push(r);  //push back on
				continue;
			}
			else
			if (stacksize()>1)
			{
				tOBJ t1 = peek(1);
				if (t1.type= INTGR && (oplist[t1.number].type != OBR) && (oplist[op].level<=oplist[t1.number].level) )
				{
					reduce();
				}
			}
			r.type=INTGR;
			r.number = op;	
			push(r);
			break;
		case DELI:
			lf=0; 
			continue;
		}
	}
	//stackprint();
	while (stacksize()>1)
	{
		reduce();
	}
	r=pop();
	clear();
	return r;
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
		r.number=variable[*name-'A'];
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
			variable[*name-'A']=r.number;
		if (r.type==FLOAT)
			variable[*name-'A']=(int)r.floatpoint;
		if (r.type==EMPTY)
			variable[*name-'A']=0;
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


tOBJ osin(tOBJ a)
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=0.0;
	if (a.type==INTGR)
	{
		r.type = INTGR;
		r.number = Sin(a.number%256);
	}

	if (a.type==FLOAT)
	{
		r.floatpoint=sin(a.floatpoint);
	}
	return r;
}

tOBJ opi(tOBJ a)
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=3.1415926;
	return r;
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

void testeval()
{
	char lines[128];
	tOBJ v;

	v.type=INTGR; v.number=20;
	set("TEST", v);
	v.type=FLOAT; v.floatpoint=20.0;
	set("FP", v);

	while (1)
	{
		readLine(lines);
		if (lines[0]=='.') break;
		v=eval_oxpr(lines);
		print(v);
		rprintfCRLF();
	}
}


