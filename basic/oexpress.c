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

tOBJ varobj[26];

char exprbuff[64];
char *e;
char tokbuff[5];
int  tb;

int    tnum;
double tfloat;

enum  TOKTYP {NUMI, NUMF, DELI, ALPHA};

int get_token() 
{
	int ty=DELI;
	tb=0;
    while (iswhite(*e))     // space or tab
    {
        e++;
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
		return ALPHA;
	}

	return ty;
}

tOBJ eval_oxpr(char *s)
{
	tOBJ r;
	r.type=EMPTY;
	strncpy(exprbuff,s,63);
	e=exprbuff;
	switch (get_token())
	{
	case ALPHA:
		r.type   = STR;
		r.string = tokbuff;
		break;
	case NUMI:
		r.type   = INT;
		r.number = tnum;
		break;
	case NUMF:
		r.type   = FLOAT;
		r.floatpoint = tfloat;
		break;
	case DELI:
		break;
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


