#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include "win.h"
#endif

#ifdef LINUX
#include "linux.h"
#endif

#ifdef AVR
#include "main.h"
#include "global.h"
#include "macro.h"
#include "wckmotion.h"
#include "rprintf.h"
#endif

#include "express.h"
#include "functions.h"

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
long variable[26]; // only A-Z at moment

extern void readtext(int ln, char *b);
extern char *strchr(char *, int);

int eval_list(char *p)
{
	// eval list "5,1,2,3,4,5" ->scene[5]
	int i;
	long n;
	char *t=p;
	
	eval_expr(&p, &n);
	nis=(int)n;
		
	if (*p++ != ',') { BasicErr=6; return 0; }
	for (i=0;i<nis;i++)
	{
		n=0;
		eval_expr(&p, &n);
		if (i!=(nis-1) && *p++ != ',') { BasicErr=6; return 0; }
		scene[i]=n;
	}
	return p-t;
}




int preci(char s)
{
	char *p = strchr(o,s);
	if (p==0) return -1;
	return mp[(p-o)];
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


unsigned char eval_expr(char **str, long *res)
{
	char c;
	
	long n1=0;
	int tsp=sp;
	int top=op;
	long tmp=0;
	int done=0;

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

	dumpstack(); // debug
	if (dbg) {
		rprintf("Eval =%s\n", *str); 
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
			(*str)++;
		}

		if (c=='M' && **str=='O'  && *(*str+1)=='D')
		{
			(*str)++;
			c='%';
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
				rprintf("eval stack %d,%d\n", top,tsp);
				 *res=0; 
				return NUMBER;
			}
			n1=0;
dumpstack(); // debug
			break;
		case '"':
			return STRING;
		case ' ':
			break; //ignore sp
		case '@':
			if (**str=='#')
            {
				int i;
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
                (*str)++;
                cnt=eval_list(*str);
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
				(*str)++;
			}
			else
            if (**str=='<')
            {
				//use sound array
				for (n1=0; (n1<MIC_NOS && n1<SDATASZ && n1<SCENESZ); n1++) 
				{
					scene[n1] = sData[n1];     // and clear
				}
				nis=MIC_NOS;
				(*str)++;
			}
			else
            if (**str=='?')
            {
				//use servo pos array
				readservos(nos);

				for (n1=0; n1<nos; n1++) 
				{
					scene[n1] = cpos[n1];     // and clear
				}
				nis=nos;
				(*str)++;
			}
            else
            {
				char tmpA[128];
				n1 = (long)(**str-'A');
				if (n1<0 || n1 >25)
				{
					break;
				}
				n1 = variable[(int)n1];
				readtext((int)n1, tmpA);				
				if (tmpA[0]==0xFF) // DATA
				{
					int i=0;
					nis=tmpA[1];
					for (i=0; i<nis; i++)
						scene[i]=(unsigned char)tmpA[i+2];
				}
				else
					eval_list(tmpA);
				//
				(*str)++;
			}
			if (**str == '[')
			{
				(*str)++;
				n1=0;
				eval_expr(str, &n1);
				if (n1>=nis) n1=nis-1;
				if (n1<0)    n1=0;

				n1 = scene[n1];
				(*str)++;
				break;
			}
			if (**str == '+' || **str == '-' || **str == '.')
			{
				/*
				//add array
				int i,tnis;
				int tempB[SCENESZ];
				char o = **str;
				(*str)++;
				for (i=0;i<SCENESZ; i++)
				{
					tempB[i]=scene[i];
				}
				tnis=nis;
				if (eval_expr(str,res)==ARRAY)
				{
					if (o=='.')
					{
						if (tnis+nis<SCENESZ) 
						{
							int m=(nis>tnis)?nis:tnis;
							for (i=m-1;i>=0; i--)
							{
								if (i<nis)
									scene[i+tnis] = scene[i];
								if (i<tnis)
									scene[i]     = tempB[i];
							}
							nis=nis+tnis;
						}
					}
					else
					{
						int m=(nis>tnis)?nis:tnis;
						for (i=0;i<m; i++)
						{
							if (o == '+') 
								scene[i] = ((i<tnis)?tempB[i]:0) + ((i<nis)?scene[i]:0); 
							else 
								scene[i] = ((i<tnis)?tempB[i]:0) - ((i<nis)?scene[i]:0); 
						}
						nis=m;
					}
				}
				*/
			}

			return ARRAY;
			break;
		case '$':
			//special var?
			if (get_special(str, &n1)==ARRAY)
				return ARRAY;
			break;
		default:
			done=1;
			(*str)--;
		}
	}

	if (sp<MAX_DEPTH-1)
		stack[sp++] = n1;
	else
	{
		rprintf("eval stack \n"); *res=0; return NUMBER;
	}

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
				rprintf("eval stack error %d, %d\r\n", op, sp);
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
