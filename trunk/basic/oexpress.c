#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include "win.h"
#endif

#ifdef LINUX
#include "linux.h"
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
                    || c == '^' || c == '(' || c == ')' || c == ',' || c == '=')

#define isalpha(c)  ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')

#define isspecial(c)  ( c == '+' || c == '-' || c == '*'  || c == '/')


tOBJ varobj[26];

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
		void *func;
} tOP, *tOPp;

tOP oplist[] = { 
	{"+",    1, PLUS,  NULL},
	{"-",    1, MINUS, NULL},
	{"/",    2, DIVD,  NULL},
	{"*",    2, MULT,  NULL},
	{"AND",  0, LAND,  NULL},
	{"OR",   0, LOR,   NULL},
	{"SIN",  2, NA,    NULL},  //function single arg
	{"MAX",  2, NA,    NULL},  //function list arg
};

tOBJ omath(tOBJ o1, tOBJ o2, int op);

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

	if (isspecial(*e))
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

tOBJ eval_oxpr(char *s)
{
	tOBJ r,a,b;
	int op=NA;

	r.type=EMPTY;
	strncpy(exprbuff,s,63);
	e=exprbuff;

	switch (get_token())
	{
	case ALPHA:
		a.type   = STR;
		a.string = tokbuff;
		break;
	case NUMI:
		a.type   = INT;
		a.number = tnum;
		break;
	case NUMF:
		a.type   = FLOAT;
		a.floatpoint = tfloat;
		break;
	case OPR:
	case DELI:
		return r; //error return null
	}

	while (1)
	{
		switch (get_token())
		{
		case OPR:
			op = getOP(tokbuff);
			op = oplist[op].type;
			break;
		case ALPHA:
		case NUMI:
		case NUMF:
			return r; //error return null
		case DELI:
			if (*e==0)
				return a; //single argument
			else
				return r; //error return null
		}

		switch (get_token())
		{
		case NUMI:
			b.type   = INT;
			b.number = tnum;
			break;
		case NUMF:
			b.type   = FLOAT;
			b.floatpoint = tfloat;
			break;
		case ALPHA:
		case OPR:
		case DELI:
			return r; //error return null
		}

		r = omath(a, b, op);
		a = r;
		r.type=EMPTY;
	}
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
			printf("::  ");
			gets(lines);
			if (lines[0]==0 || lines[0]=='.')
				break;
			v=eval_oxpr(lines);
			print(v);
			rprintfCRLF();
		}
}


