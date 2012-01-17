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

#define isdelim(c)  (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' \
                    || c == '^' || c == '(' || c == ')' || c == ',' || c == '='  || c == '$')

#define isalpha(c)  ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')


#define MAXSTACK 10

tOBJ varobj[26];


enum  TYPE {SYM, INT, BOOL, FUNC, FLOAT, STR, CELL, EMPTY};

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

tOP oplist[] = { 
	{"+",    1, PLUS,  2, NULL},
	{"-",    1, MINUS, 2, NULL},
	{"/",    2, DIVD,  2, NULL},
	{"*",    2, MULT,  2, NULL},
	{"(",    5, OBR,   1, NULL},
	{")",    5, CBR,   1, NULL},
	{"AND",  0, LAND,  2, NULL},
	{"OR",   0, LOR,   2, NULL},
	{"SIN",  4, NA,    1, osin},  //function single arg
	{"PI",   4, NA,    0, opi },  //const
	{"MAX",  4, NA,    3, NULL},   //function list arg - tbd
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

	r.type=EMPTY;
	strncpy(exprbuff,s,63);
	e=exprbuff;

	while (lf)
	{
		tk = get_token();
		switch (tk)
		{
		case ALPHA:
			r.type   = STR;
			r.string = tokbuff;
			push(r);
			break;
		case NUMI:
			r.type   = INT;
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
				if (t1.type= INT && (oplist[t1.number].type != OBR) && (oplist[op].level<=oplist[t1.number].level) )
				{
					reduce();
				}
			}
			r.type=INT;
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
        if (r.type == INT)
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
        else if (r.type == BOOL)
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

	if (o1.type==INT && o2.type==INT)
	{
		r.type=INT;
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
		if (o1.type==INT) a=(double)o1.number;

		if (o2.type==FLOAT) b=o2.floatpoint;
		if (o2.type==INT) b=(double)o2.number;

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
		default:
			r.type=EMPTY; //error
			break;
		}
	}
	return r;
}

int set(char *name, tOBJ r)
{
	return -1; //not exist
}

tOBJ get(char *name)
{
	tOBJ r;
	r.type=EMPTY;
	return r; //not found
}

tOBJ osin(tOBJ a)
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=0.0;
	if (a.type==INT)
	{
		r.type = INT;
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
	r.floatpoint=3.1415;
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

	while (1)
	{
		readLine(lines);
		if (lines[0]=='.') break;
		v=eval_oxpr(lines);
		print(v);
		rprintfCRLF();
	}
}

