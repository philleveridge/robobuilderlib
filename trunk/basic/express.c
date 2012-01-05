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

int variable[26]; // only A-Z at moment

int eval_list(char *p)
{
	// eval list "5,1,2,3,4,5" ->scene[5]
	int i,n;
	char *t=p;
	
	eval_expr(&p, &nis);
		
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

unsigned char eval_expr(char **str, long *res)
{
	char c;
	
	long n1=0;
	long stack[MAX_DEPTH]; 
	char ops[MAX_DEPTH];
	
	int sp=0;
	int op=0;
	long tmp=0;
	int done=0;
	
	while (**str != '\0' && !done)
	{
		if ((c = **str)==')')
			break;
		
		(*str)++;

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
			ops[op++]=c;
			stack[sp++]=n1;
			n1=0;
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
				n1 = **str-'A';
				if (n1<0 || n1 >25)
				{
					break;
				}
				n1 = variable[n1];
				readtext(n1, tmpA);				
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

	stack[sp++] = n1;

	while (op>0) {
		if (ops[op-1]==':')
		{
			if (op > 1 && ops[op - 2] == '?' && sp>2)
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
			if (op > 0 && sp>1)
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

	*res = stack[0];
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
