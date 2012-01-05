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

#define isalpha(c)  ((c >= 'A' && c <= 'Z') || c == '_')

tOBJ varobj[26];

char exprbuff[64];
char *e;
char tokbuff[25];
char *tb;

enum  TOKTYP {NUMI, NUMF, DELI, ALPHA};

int get_token() 
{
	int ty=DELI;
	tb=tokbuff;
    while (iswhite(*e))     // space or tab
    {
        e++;
    }

	if (isnumdot(*e))
	{
		int r=NUMI;
		while (isnumdot(*e))
		{
			if (*e=='.') r=NUMF;
			*tb++=*e++;
		}
		*tb=0;
		return r;
	}

	if (isalpha(*e))
	{
		*tb++=*e++;
		while (!isdelim(*e))
		{
			*tb++=*e++;
		}
		*tb=0;
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
		break;
	case NUMI:
		break;
	case NUMF:
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


