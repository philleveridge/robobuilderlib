#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include "win.h"
#endif

#ifdef LINUX
#include "linux.h"
#endif

#ifdef AVR
#include <avr/eeprom.h> 
#include <avr/pgmspace.h>

#include "main.h"
#include "global.h"
#include "macro.h"
#include "wckmotion.h"
#include "rprintf.h"
#endif

#include "edit.h"
#include "express.h"
#include "functions.h"
#include "lists.h"

extern int				nis;
extern int				BasicErr;
extern int				scene[];	  // generic array
extern unsigned char	cpos[];
extern BYTE				sData[];
extern int 				sDcnt;

#define SCENESZ 128
extern BYTE				nos;
extern volatile BYTE	MIC_SAMPLING;
extern volatile BYTE    MIC_NOS;

extern int dbg;

const char *o 	= "+-*/><gl=n&%|:?";
const int mp[]  = {10,10,20,20,5,5,5,5,5,5,5,0,0,0,0};

const  prog_char  specials[37][7] = {

		"VOLT",  "IR",   "KBD",  "RND",  "SERVO", 
		"TICK",  "PORT", "ROM",  "TYPE", "ABS", 
		"MAPIR", "KIR",  "FIND", "CVB2I","NE", 
		"NS",    "MAX",  "SUM",  "MIN",  "NORM", 
		"SQRT",  "SIN",  "COS",  "IMAX", "HAM", 
		"RANGE", "SIG",  "DSIG",  "STAND", "ZEROS",
		"MIC",   "X",    "Y",    "Z",    "PSD", 
		"GREY",  "TURTLE"
};


long variable[26]; // only A-Z at moment

extern void readtext(int ln, unsigned char *b);
extern char *strchr(const char *, int);

char arrayname;

int preci(char s)
{
	char *p = strchr(o,s);
	if (p==0) return -1;
	return mp[(p-o)];
}

void showvars()
{
	int i,j=0;

	for (i=0; i<24; )
	{ 
		for (j=0; j<6; j++)
		{
			rprintf ("%c = %ld  ", 'A'+i, variable[i]);
			i++;
		}
		rprintfCRLF();
	}
	rprintf ("%c = %ld  ", 'A'+i, variable[i++]);
	rprintf ("%c = %ld  ", 'A'+i, variable[i]);
	rprintfCRLF();
}

static long stack[MAX_DEPTH]; 
static char ops[MAX_DEPTH];	
static int sp=0;
static int op=0;

void dumpstack()
{
	int sc;
	if (dbg) {
		rprintf("Ops %d\n", op); 
		for (sc=0;sc<op;sc++) rprintf("%d - [%c]\n", sc,ops[sc]);
		rprintf("stack %d\n", sp); 
		for (sc=0;sc<sp;sc++) rprintf("%d - %d\n", sc,stack[sc]);
	}
	return;
}

void epush(long n)
{
	if (sp<MAX_DEPTH-1)
		stack[sp++] = n;
	else
	{
		rprintfProgStr(PSTR("eval stack error\n"));
		op=0;sp=0;
	}
}

long epop()
{
	if (sp>0)
		return stack[--sp];
	else
	{
		rprintfProgStr(PSTR("eval stack error\n"));
		op=0;sp=0;
		return 0;
	}
}

unsigned char eval_expr(char **str, long *res)
{
	char c;
	
	long n1=0;
	int tsp=sp;
	int top=op;
	long tmp=0;
	int done=0;
	int i;

#ifdef PARSE
	if (**str=='!')
	{
		tOBJ v;
		(*str)++;
		v=eval_oxpr(*str);
		if (v.type==INTGR)
			n1=v.number;
		else if (v.type==FLOAT)
			n1=(int)v.floatpoint;
		else n1=0;
		*res=n1;
		while (**str!='\0') (*str)++;
		return NUMBER;
	}
#endif


	if (dbg) {
		rprintf("Eval =%s\n", *str); 
		dumpstack(); // debug
	}
	
	while (**str != '\0' && !done)
	{
		if ((c = **str)==')')
			break;
		
		(*str)++;

		if (c=='A' && **str=='N' && *(*str+1)=='D')
		{
			c='&';
			(*str)++;
			(*str)++;
		}

		if (c=='O' && **str=='R')
		{
			c='|';
			(*str)++;
		}

		if (c=='M' && **str=='O'  && *(*str+1)=='D')
		{
			c='%';
			(*str)++;
			(*str)++;
		}

		if (c>='0' && c<='9')
		{
			n1 = n1*10 + c - '0';
		}
		else
		if (c>='A' && c<='Z')
		{
			n1 = variable[c-'A'];
		}
		else
		switch (c)
		{
		case '(':
			eval_expr(str, &tmp);
			n1 = tmp;
			(*str)++;
			break;
		case '?' :
		case '+' :
		case '-' :
		case '*' :
		case '/' :
		case '>' :
		case '<' :
		case '=' :
		case '&' :
		case '|' :
		case '%' :
		case ':' :
			if (c=='>' && **str=='=') {c='g'; (*str)++;}
			if (c=='<' && **str=='=') {c='l'; (*str)++;}
			if (c=='<' && **str=='>') {c='n'; (*str)++;}



			if (op>top && (preci(c)<=preci(ops[op-1])))
			{
				n1 = math(n1,stack[sp-1],ops[op-1]);
				sp--;
				op--;
			}

			if (sp<MAX_DEPTH-1 && op<MAX_DEPTH-1)
			{
				ops[op++]=c;
				stack[sp++] = n1;
			}
			else
			{
				rprintf("eval stack err %d,%d\n", top,tsp);
				op=0;sp=0;
				 *res=0; 
				return NUMBER;
			}
			n1=0;

			break;
		case '"':
			return STRING;
		case ' ':
			break; //ignore sp
		case '@':
			if (**str=='#')
            {
				arrayname = '!';
				(*str)++;
				for (i=0;i<16; i++)
				{
					scene[i] = offset[i];
				}
				nis=16;
			}
			else
            if (**str=='{')
            {
                // literal
                int cnt;
				arrayname = '!';

                (*str)++;
                cnt=list_eval('!',*str,0);
                *str = *str+cnt;
                if (**str!='}')
                {
					break;
                }
				(*str)++;
            }
			else
            if (**str=='!')
            {
				//use current array
				arrayname = **str;
				(*str)++;
			}
			else
            if (**str=='<')
            {
				//use sound array
				for (i=0; (i<MIC_NOS && i<SDATASZ && i<SCENESZ); i++) 
				{
					scene[i] = sData[i];     // and clear
				}
				nis=MIC_NOS;
				arrayname = '!';
				(*str)++;
			}
			else
            if (**str=='?')
            {
				//use servo pos array
				arrayname = '!';
				readservos(nos);

				for (i=0; i<nos; i++) 
				{
					scene[i] = cpos[i];     // and clear
				}
				nis=nos;
				(*str)++;
			}
            else
            {
				i = (**str-'A');
				if (i<0 || i >25)
				{
					break;
				}
				arrayname = **str;

				(*str)++;
			}
			if (**str == '[')
			{
				(*str)++;
				n1=0;
				eval_expr(str, &n1);

				n1 = (long)listread(arrayname, (int)n1);
				(*str)++;
				break;
			}
			if (**str == '+' || **str == '-' || **str == '.')
			{
			}

			return ARRAY;
			break;
		case '$':
			//special var?
			{
				int noargs=0;
				n1=0;
				tmp=0;
				for (i=0; i<NOSPECS; i++)
				{
					if (!strncmp_P(*str, specials[i], strlen_P(specials[i])))
						break;
				}
	
				if (i==NOSPECS)
				{
					return -1; 		// no_match : match
				}

				*str = *str + strlen_P(specials[i]);

				if (i == sIR2ACT|| i == sDSIG  || i == sSQRT  || i == sSTAND ||
					i == sZEROS || i == sABS   || i == sCOS   || i == sCVB2I ||
					i == sSIN   || i == sNORM  || i == sSUM   || i == sSERVO ||
					i == sROM   || i == sMIN   || i == sMAX   || i == sIMAX  ||
					i == sGREY  || i == sTURTLE )
				{
					noargs=1;
				}

				if (i==sRND)
				{
					noargs=3;
					if (**str!='(') noargs=0;
				}

				if (i==sRANGE)
				{
					noargs=3;
				}

				if (i==sSIG || i==sFIND)
				{
					noargs=2;
				}

				if (noargs>0 && **str=='(') 
				{
					n1=noargs;
					while (noargs  > 0)
					{
						tmp=0;
						(*str)++;
						eval_expr(str, &tmp);
						epush(tmp);
						
						if (**str!=')' && **str!=',')
						{
							return -1;
						}
						noargs--;
					}
					tmp=n1;
					(*str)++;
					if (tmp==1) tmp=epop();
				}

				if (get_special(str, &tmp, i)==ARRAY)
				{
					arrayname = '!';
					return ARRAY;
				}
				n1=tmp;
			}
			break;
		default:
			done=1;
			(*str)--;
		}
	}

	epush(n1);

	while (op>top) {
		if (ops[op-1]==':')
		{
			if (op > top+1 && ops[op - 2] == '?' && sp>tsp+2)
			{
				if (stack[sp - 3] == 0)
					stack[sp - 3] = stack[sp - 1];
				else
					stack[sp - 3] = stack[sp - 2];
				op--;
				sp-=2;
			}
			else
			{
				rprintf("eval stack error %d, %d\r\n", op, sp);
				op=0;sp=0;
				return ERROR; 
			}
		}
		else
		{
			if (op > top && sp>tsp+1)
			{
				stack[sp-2] = math(stack[sp-2],stack[sp-1],ops[op-1]);
				sp--;
			}
			else
			{
				rprintf("eval stack pop error %d, %d [%d,%d]\r\n", op, sp, top, tsp);
				op=0;sp=0;
				return ERROR;
			}
		}
		op--;
	}

	*res = stack[tsp];
	sp=tsp; 
	return NUMBER;
}

long math(long n1, long n2, char op)
{
	switch (op) {
	case '+': 
		n1=n1+n2; break;
	case '-':
		n1=n1-n2; break;
	case '*':
		n1=n2*n1; break;
	case '/':
		if (n2==0)
		{
			rprintfProgStr (PSTR("Err - div 0\r\n"));
			return 0;
		}
		n1=n1/n2; break;
	case '&':
		n1=n2 && n1; break;
	case '|':
		n1=n1 || n2; break;
	case '%':
		n1=n1%n2; break;
	case '>':
		n1=(n1>n2)?1:0; break;		
	case '<':
		n1=(n1<n2)?1:0; break;	
	case 'l':
		n1=(n1<=n2)?1:0; break;	
	case 'g':
		n1=(n1>=n2)?1:0; break;	
	case 'n':
		n1=(n1!=n2)?1:0; break;	
	case '=':
		n1=(n2==n1)?1:0; break;		
	}
	return n1;
}
