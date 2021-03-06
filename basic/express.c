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

extern int		BasicErr;
extern unsigned char	cpos[];
extern BYTE		sData[];
extern int 		sDcnt;
extern BYTE		nos;
extern volatile BYTE	MIC_SAMPLING;
extern volatile BYTE    MIC_NOS;

extern int dbg;

const char *o 	= "+-*/><gl=n&%|:?";
		// +  -  *  /  > < >= <= = <> AND MOD OR IF ELSE
const int mp[]  = {10,10,20,20,5,5,5, 5, 5,5, 2,  20, 2, 0, 0};

const  prog_char  specials[NOSPECS][7] = {
		"VOLT",  "IR",   "KBD",   "RND",  "SERVO", 
		"TICK",  "PORT", "ROM",   "TYPE", "ABS", 
		"MAPIR", "KIR",  "FIND",  "CVB2I","NE", 
		"NS",    "MAX",  "SUM",   "MIN",  "NORM", 
		"SQRT",  "SIN",  "COS",   "IMAX", "HAM", 
		"RANGE", "SIG",  "DSIG",  "STAND","ZEROS",
		"MIC",   "X",    "Y",     "Z",    "PSD", 
		"GREY",  "TURTLE","EVENT","MAP",  "SHUF", 
		"SCALE", "INPUT", "REV",  "SORT", "IMIN", "UPK"
#ifdef IMAGE
		,"IMR"  //sIMR
		,"PLY"  //sPLY
#endif
};

extern char *strchr(const char *, char);

char arrayname;

/*************************************************************************************************************

           Variable routines

*************************************************************************************************************/

static long variable[26]; // only A-Z at moment
void setvar(char n, long v)
{
	if (n>=0 && n<26)
		variable[n]=v;
}

long getvar(char n)
{
	if (n>=0 && n<26)
		return variable[n];
	return 0;
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

/*************************************************************************************************************

           Operator and value Stack routines

*************************************************************************************************************/

static long stack[MAX_DEPTH]; 
static char ops[MAX_DEPTH];	
static int sp=0;
static int op=0;

void dumpstack()
{
	int sc;
	if (dbg>2) {
		rprintf("Ops %d\n", op); 
		for (sc=0;sc<op;sc++) rprintf("%d - [%c]\n", sc,ops[sc]);
		rprintf("stack %d\n", sp); 
		for (sc=0;sc<sp;sc++) rprintf("%d - %ld\n", sc,stack[sc]);
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

long eswap(long n, int i)
{
	if (i>=0 && sp-i>=0)
	{
		long t = stack[sp-i-1];
		stack[sp-i-1] = n;
		return t;
	}
	else
	{
		rprintfProgStr(PSTR("eval stack error\n"));
		op=0;sp=0;
		return (long)0;
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

/*************************************************************************************************************

           Expression parser

*************************************************************************************************************/
extern int base;

extern void set_hi(char ln, int n, int v);
extern void set_lo(char ln, int n, int v);

int preci(char s)
{
	char *p = strchr(o,s);
	if (p==0) return -1;
	return mp[(p-o)];
}

#ifdef PARSE
extern tOBJ eval_oxpr(char *s);
extern tOBJ print (tOBJ);
#endif

unsigned char eval_expr(char **str, long *res)
{
	char c;
	
	long n1=0;
	int tsp=sp;
	int top=op;
	long tmp=0;
	int done=0;
	int i;
        int sgn=0;

	while (**str==' ') (*str)++; //ignore leading w/s

#ifdef PARSE
	if (**str=='!')
	{
		tOBJ v;
		char *cp;
		(*str)++;
		cp=strchr(*str, '!');
		if (cp != 0) *cp='\0';
		if (dbg > 1) printf ("E=[%s]\n", *str);
		v=eval_oxpr(*str);
		if (v.type==INTGR)
		{
			n1=(long)(v.number);
		}
		else if (v.type==FLOAT)
		{
			n1=(long)(v.floatpoint);
		}
		else
		{
			 n1=0;
		}
		*res=n1;
		if (cp !=0) 
			*str=cp+1;
		else 
			while (**str!='\0') 
				(*str)++;
		return NUMBER;
	}
#endif

	if (dbg>1) {
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

		if (c=='-' && sgn==0)
		{
			sgn=-1;	
			continue;
		}

		if (c>='A' && c<='Z' && **str>='A' && **str<='Z') // 
		{
			c='$';
			(*str)--;
		}

		if (c>='A' && c<='Z' && **str=='[')
		{
			c='@';
			(*str)--;
		}

		if (c>='0' && c<='9')
		{
			if (sgn==0) sgn=1;
			n1 = n1*10 + c - '0';
		}
		else
		if (c>='A' && c<='Z')
		{
			n1 = variable[c-'A'];
			sgn=1;
		}
		else
		switch (c)
		{
		case '(':
			eval_expr(str, &tmp);
			n1 = tmp;
			sgn=1;
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
			n1=n1*sgn;
			sgn=0;
			if (c=='>' && **str=='=') {c='g'; (*str)++;}
			if (c=='<' && **str=='=') {c='l'; (*str)++;}
			if (c=='<' && **str=='>') {c='n'; (*str)++;}

			if (op>top && (preci(c)<=preci(ops[op-1])))
			{
				n1 = math(stack[sp-1],n1,ops[op-1]);
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
				rprintf("stack err %d,%d\n", top,tsp);
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
			else if (**str=='{')
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
			else if (**str=='!')
			{
				//use current array
				arrayname = **str;
				(*str)++;
			}
			else if (**str=='<')
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
			else if (**str=='?')
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
				int bm=0;
				(*str)++;

				if (**str=='#')
				{ 
					(*str)++;
					bm=1;
				}
				n1=0;
				sgn=1;
				eval_expr(str, &n1);
				if (**str == ',')
				{
					//sub list
					int i; long ct;
					(*str)++;
					eval_expr(str, &ct);
					if (**str !=']' )
						break;

					i=listsize(arrayname);
					if (ct>=i) ct=i-1;
					if (n1>ct) n1=ct;					

					for (i=n1; i<=ct; i++)
					{
						scene[i-n1]= (int)listread(arrayname, i-base);
					}
					nis=ct-n1+1;
					arrayname='!';
					(*str)+=1;
					return ARRAY;
				}
				else
				{
					if (bm==1)
					{
						int n2=n1/2;
						n1=n1-base;
						if (n1%2==1)
						{
							n1=get_lo(arrayname, n2);
						}
						else
						{
							n1=get_hi(arrayname, n2);
						}
					}
					else
					{
						n1 = (long)listread(arrayname, (int)n1-base);

					}
					sgn=1;
					(*str)++;
				}
				break;
			}
			if ((**str == '+' || **str == '-' || **str == '/'|| **str == '*') && *(*str+1)=='@')
			{
				add_sub(arrayname, *(*str+2), **str) ;
				arrayname='!';
				(*str)+=3;
			}
			if (**str == '.' && *(*str+1)=='@')
			{
				join(arrayname, *(*str+2));
				arrayname='!';
				(*str)+=3;
			}
			return ARRAY;
			break;
		case '$':
			//special var?
			{
				int noargs=0;
				n1=0;
				sgn=1;
				tmp=0;

				if (**str=='F' && *(*str+1) =='N')
				{
					// user defined function
					// DEF FNA(x,y) = <expr>
					// TBD
					return -1;
 				}

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

				if (    i == sIR2ACT|| i == sDSIG  || i == sSQRT || i == sSTAND ||
					i == sZEROS || i == sABS   || i == sCOS  || i == sCVB2I ||
					i == sSIN   || i == sNORM  || i == sSUM  || i == sSERVO ||
					i == sROM   || i == sMIN   || i == sMAX  || i == sIMAX  ||
					i == sGREY  || i == sTURTLE|| i == sSHUF  || i == sREV   ||
					i == sSORT  || i == sIMIN  || i== sUPK )
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

				if (i==sSIG || i==sFIND || i==sHAM || i==sMAP)
				{
					noargs=2;
				}

				if (i==sSCALE)
				{
					noargs=4;
				}

				if (noargs>0 && **str=='(') 
				{
					n1=noargs;
					while (noargs  > 0)
					{
						int ty;
						tmp=0;
						if (**str==')') break;
						(*str)++;
						ty= eval_expr(str, &tmp);
						if (ty==NUMBER)
							epush(tmp);
						else
							epush(arrayname);

						if (**str!=')' && **str!=',')
						{
							return -1;
						}
						noargs--;
					}
					tmp=n1-noargs;
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
			break;
		}
	}

	n1=n1*sgn;
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
				rprintf("Stack error %d, %d\r\n", op, sp);
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
				rprintf("Stack error %d, %d [%d,%d]\r\n", op, sp, top, tsp);
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

/*************************************************************************************************************

           Math operations

*************************************************************************************************************/

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
		if (n2==0)
		{
			rprintfProgStr (PSTR("Err - mod 0\r\n"));
			return 0;
		}
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
