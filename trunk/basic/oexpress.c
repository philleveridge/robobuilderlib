#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "win.h"
#endif

#ifdef LINUX
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "linux.h"
#endif

#include "express.h"
#include "functions.h"
#include "lists.h"

//cmap.c functions
extern void showImage	(int n);
extern void clear_colors();
extern int  add_color   (char *s, int r, int g, int b, int min);
extern void clrmap      ();
extern void add_thresh  (int n, int minR, int maxR, int minG, int maxG, int minB, int maxB);
extern int  processFrame(int sz, int *data);

//jpeg.c
extern int  loadJpg     (char* Name);
extern void output_frame(int sz);



extern void sigcatch();
extern int  matrixload(int n, char *s);
extern int  matrixstore(int n, char *s);
extern int  loadimage(char *ifile, int x, int *f);
extern int  filterimage(char *ifile, int *data, int x, int a, int b, int c, int d, int e, int f);
extern void get_color_region(int);
extern int  dbg;
extern void output_grey1(int sz);
extern int *frame;

/* new matrix functions */

fMatrix readmatrix(char *s);
fMatrix newmatrix(int c, int r);
fMatrix fadd2(fMatrix *A, fMatrix *B, char op);  
int     fmatprint2(fMatrix *A);
fMatrix fmultiply2(fMatrix *A,fMatrix *B)  ; 
float   fget2(fMatrix *M, int c, int r);
float   fset2(fMatrix *M, int c, int r, float v);
fMatrix fmatcp(fMatrix *A); // clone
fMatrix ftranspose2(fMatrix *A)  ;
fMatrix fmatrshp(fMatrix *A, int c, int r) ;
fMatrix freplicate2(fMatrix *A, int m, int n);
fMatrix fconvolve2(fMatrix *A, fMatrix *B)  ;
fMatrix fimport2(char m2, int c, int r);
fMatrix fmatsum2(fMatrix *A, int mode) ;
fMatrix fmatzerodiag2(fMatrix *A)   ;
fMatrix fmatzeroregion(fMatrix *A, int c1, int r1, int c2, int r2)   ;

// The new expression parser
// work in progress
//


#define iswhite(c)   (c == ' ' || c == '\t')
#define isnumdot(c)  ((c >= '0' && c <= '9') || c == '.')
#define isnumber(c)  (c >= '0' && c <= '9')

#define isdelim(c)  (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '<'  || c == '>'  \
                    || c == '^' || c == '(' || c == ')' || c == ',' || c == '='  || c == '$' || c=='@')

#define isalpha(c)  ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')


#define MAXSTACK 50
#define MAXSTRING 1024


char varname[256];
tOBJ varobj[50];
int nov=0;


char exprbuff[MAXSTRING];
char *e;
char tokbuff[5];
char tmpstr[MAXSTRING];
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
void ocos ();
void otan ();
void osqrt();
void olog ();
void oexp ();
void oacos();
void oatan();
void oabs ();
void oacx ();
void oacy ();
void oacz ();
void osig ();
void ornd ();
void odsig ();
void opsd ();
void omax ();
void omat ();
void oprint ();
void otrn ();
void oapply ();
void orshp ();
void orep ();
void ozero ();
void oeye ();
void ovsum ();
void ohsum ();
void oconv ();
void oimp ();
void ocond();
void ozerob();
void ozerod();

tOBJ get(char *name);
int  set(char *name, tOBJ r);

tOP oplist[] = { 
/* 0 */	{"+",    10, PLUS,  2, NULL},
	{"-",    10, MINUS, 2, NULL},
	{"/",    20, DIVD,  2, NULL},
	{"*",    20, MULT,  2, NULL},
	{".*",   20, PROD,  2, NULL},
/* 5 */	{".^",   20, POWR,  2, NULL},
	{"AND",  8,  LAND,  2, NULL},
	{"OR",   8,  LOR,   2, NULL},
	{"<",    5,  LT,    2, NULL},
	{">",    5,  GT,    2, NULL},
/*10 */	{"=",    5,  EQL,   2, NULL},
	{"<>",   5,  NEQ,   2, NULL},
	{"(",    50, OBR,   1, NULL},
	{")",    50, CBR,   1, NULL},
	{",",    50, COMMA, 1, NULL},
/*15 */	{"SIN",  40, NA,    1, osin},  //function single arg
	{"COS",  40, NA,    1, ocos},  //function single arg
	{"TAN",  40, NA,    1, otan},  //function single arg
	{"ATAN", 40, NA,    1, oatan}, //function single arg
	{"ACOS", 40, NA,    1, oacos}, //function single arg
/*20 */	{"LOG",  40, NA,    1, olog},  //function single arg
	{"EXP",  40, NA,    1, oexp},  //function single arg
	{"SQRT", 40, NA,    1, osqrt}, //function single arg
	{"SIG",  40, NA,    1, osig},  //sigmoid functon
	{"DSIG", 40, NA,    1, odsig}, //sigmoid functon
/*25 */	{"PSD",  40, NA,    0, opsd},  //const
	{"ACCX", 40, NA,    0, oacx},  //const
	{"ACCY", 40, NA,    0, oacy},  //const
	{"ACCZ", 40, NA,    0, oacz},  //const
	{"ABS",  40, NA,    1, oabs},  //function single arg
/*30 */	{"RND",  40, NA,    0, ornd},  //in-const
	{"MAX",  40, NA,    2, omax},   //function two args
	{"TRN",  40, NA,    1, otrn},   //function single arg    <fMatrix>
	{"CELL", 40, NA,    3, omat},   //function three args   <fMatrix, int, int>
	{"RSHP", 40, NA,    3, orshp},  //function three args  <fMatrix, int, int>
/*35 */	{"REP",  40, NA,    3, orep},   //function three args  <fMatrix, int, int>
	{"ZERO", 40, NA,    2, ozero},  //function two args  <fint, int>
	{"EYE",  40, NA,    2, oeye},   //function two args  <fint, int>
	{"HSUM", 40, NA,    1, ohsum},  //function two args   <fMatrix>
	{"VSUM", 40, NA,    1, ovsum},  //function two args   <fMatrix>
/*40 */	{"APPL", 40, NA,    2, oapply}, //function two args   <fMatrix>
	{"CONV", 40, NA,    2, oconv},  //function three args   <fMatrix>
	{"IMP",  40, NA,    3, oimp},   //function two args   <string>, <int>, <int>
	{"COND", 40, NA,    5, ocond},  //function four args   <fmatrix>, <min>, <max> <value>
	{"ZERB", 40, NA,    4, ozerob}, //function four args   <fmatrix>, <int> <int> Mint> <int>
/*45 */	{"ZERD", 40, NA,    1, ozerod}  //function 1 args   <fmatrix>
};

tOBJ omath(tOBJ o1, tOBJ o2, int op);
tOBJ print(tOBJ r);

/**********************************************************/
/*  strings                                              */
/**********************************************************/

char strings[MAXSTRING], *estr=&strings[0];
char *newstring(char *s)
{
	char *r;
	int n=strlen(s);
	if (dbg) printf ("New String [%d]\n",n);
	r=(char *)malloc(n*sizeof(char));
	strcpy(r, s);
	return r;
}

void delstring(char *s)
{
	if (dbg) printf ("Del String \n");
	free(s);
}

tOBJ makestring(char *s)
{
	tOBJ r;
	r.type=STR;
	r.string = newstring(s);
	r.cnt=0;
	return r;
}

/**********************************************************/
/*  float   and integers                                  */
/**********************************************************/

tOBJ makefloat(float f)
{	
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=f;
	return r;
}

tOBJ makefloati(int i)
{	
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=(float)i;
	return r;
}

float tofloat(tOBJ v)
{
	float f=0.0;
	if (v.type==FLOAT)
		f=v.floatpoint;
	else
	if (v.type==INTGR)
		f=(float)v.number;

	return f;
}

tOBJ makeint(int i)
{	
	tOBJ r;
	r.type=INTGR;
	r.number=i;
	return r;
}

int toint(tOBJ v)
{
	int f=0;
	if (v.type==FLOAT)
		f=(int)v.floatpoint;
	else
	if (v.type==INTGR)
		f=v.number;
	return f;
}

/**********************************************************/
/*  stack                                                 */
/**********************************************************/

tOBJ stackobj[MAXSTACK];
int sop=0;

unsigned char stackop[MAXSTACK];
int oop=0;


int freeobj(tOBJ *b)
{
	if (b->type==FMAT2)
	{
		if (b->cnt==0) delmatrix(&b->fmat2);
	}

	if (b->type==STR)
	{
		if (b->cnt==0) delstring(b->string);
	}

	return 0;
}

int push(tOBJ a)
{
	if (dbg)  { printf ("PUSH %d = ", a.type); print(a); printf("\n"); }

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

	if (dbg) { printf ("POP %d = ", e.type);  print(e); printf("\n"); }

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
/*  Parser functions                                      */
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

int get_token(int flg) 
{
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

	if (*e == '"')
	{
		char *p=tmpstr;
		if (flg) return DELI;
		e++;
		while (*e != '"' && *e !='\0')
		{
			*p++=*e++;
		}
		e++;
		*p='\0';
		return STRNG;
	}

	if (*e == '[')
	{
		char *p=tmpstr;
		if (flg) return DELI;
		e++;
		while (*e != ']' && *e !='\0')
		{
			*p++=*e++;
		}
		e++;
		*p='\0';
		return MLIST;
	}

	if (*e== '.' && ((*(e+1)=='*') || (*(e+1)=='^')))
	{		
		tokbuff[tb++]='.';
		tokbuff[tb++]=*(e+1);
		tokbuff[tb]=0;
		e+=2;
		return OPR;
	}

	if (flg && *e=='-')
	{
		tokbuff[tb++]=*e++;
		tokbuff[tb]=0;
		return OPR;
	}

	if (isnumdot(*e) || (*e=='-' && isnumdot(*(e+1))) )
	{
		int r=NUMI;
		int sgn=1;
		double dn=0.1;
		tnum=0;
		if (flg) return DELI;
		if (*e=='-') { e++; sgn=-1; }
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
		if (r==NUMI) tnum=tnum*sgn;
		if (r==NUMF) tfloat=tfloat*(float)sgn;
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
	return DELI;
}


void reduce()
{
	tOBJ a,b, c;
	int i;

	if (oop<=0)
		return; //nothing to do

	i=stackop[--oop];

	if (oplist[i].nop==2 && oplist[i].func == NULL)
	{
		b = pop();
		a = pop();
		c = omath(a, b, oplist[i].type);
		push(c);
		freeobj(&a);
		freeobj(&b);
	}
	
	if (oplist[i].nop==1 && oplist[i].func == NULL)
	{
		return;
	}

	if (oplist[i].func != NULL)
	{
		if(oplist[i].nop>stacksize())
		{
			rprintf("Insuffcient parameters need %d but only have %d\n", oplist[i].nop, stacksize());
			stackprint();
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
	int gnf=0;

	r.type=EMPTY;
	strncpy(exprbuff,s,MAXSTRING-1);
	e=exprbuff;

	while (lf)
	{
		tk = get_token(gnf);

		if (gnf==0 || (tk==OPR) || (tk==DELI))
		{
			switch (tk)
			{
			case ALPHA:
				r = get(tokbuff);
				push(r);
				if (r.type==EMPTY)
				{
					printf ("Error - bad syntax `%s'\n", tokbuff);
					return r;
				}
				gnf=1;
				break;
			case STRNG:
				r=makestring(tmpstr);
				push(r);
				gnf=1;
				break;
			case NUMI:
				r.type   = INTGR;
				r.number = tnum;
				push(r);
				gnf=1;
				break;
			case NUMF:
				r.type   = FLOAT;
				r.floatpoint = tfloat;
				push(r);
				gnf=1;
				break;
			case MLIST: 
				{
				int toop=oop;
				oop=0;
				r.type   = FMAT2;
				r.fmat2 = readmatrix(tmpstr);
				r.cnt=0;
				oop=toop;
				push(r);	
				gnf=1;	
				break;	
				}
			case OPR:
				{
					op = getOP(tokbuff);

					if (oplist[op].nop==0 && oplist[op].func != NULL)
					{
						(*oplist[op].func)();
						continue;
					}
					else
					if (oop>0 && (oplist[op].type == CBR || oplist[op].type==COMMA))
					{
						int t=oplist[op].type;
						int i=stackop[oop-1];

						while (oplist[i].type != OBR && oplist[i].type != COMMA)
						{
							if (oop==0) break;
							reduce();
							i=stackop[oop-1];
				
						}
						if  ((t==CBR) && (oplist[i].type== OBR) )
							reduce();
						gnf=0;
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
					gnf=0;
				}
				break;
			case DELI:
				lf=0;
				gnf=0;
				continue;
			}
		}
		else
		{
			break;
		}
	}

	while (oop>0)
	{
		if (dbg) printf("Reduce %d \n",oop);
		reduce();
	}
	return pop();
}
 

/*
i
!LET MC=[2.0 3.6;2.7 1.2]

!let mx=[1.0 2.0;3.0 4.0]+[2.0 3.6;2.7 1.2]
!print mx

!let ms=mx+mc
!print ms

!print [1.0 2.0;3.0 4.0]*[2.0 3.6;2.7 1.2]

!print trn([1.0 2.0 3.0 4.0])
!print trn([1.0 2.0;3.0 4.0])

!PRINT [1.0 2.0;3.0 4.0]*[1.0 2.0;3.0; 4.0]
!PRINT [1.0 2.0;3.0 4.0]*[1.0 2.0;3.0 4.0]

!PRINT [1.0 2.0;3.0 4.0]*0.2
*/

fMatrix readmatrix(char *s)
{
	fMatrix m;
	float ts[10000];
	char *t;
	char savebuff[MAXSTRING];
	tOBJ v;
	int i=0;
	int j=0;
	int c=0;

	int toop;

	strncpy(savebuff, exprbuff, MAXSTRING); 
	toop=oop;
	t=e;

	e=s;

	if (dbg) printf("LIST=%s\n", e);

	while (e==0 || *e != 0)
	{
//printf ("[%s]\n", e);
		char *p=strchr(e, ' ');
		
		if (p !=0)
		{
			*p='\0';
			p++;
		}

		v=eval_oxpr(e);	
		ts[c] = tofloat(v);

		if (*e==';')
		{
			//
			j++; i=0; e++; c++;
			v=eval_oxpr(e);	
			ts[c] = tofloat(v);
		}

		if (p!=0) e=p;

		if (dbg) printf ("[%d,%d] = %f  (%s)\n", i,j,ts[c], e);

		while (*e==' ') e++; 

		if (*e==';') { j++; i=0; e++; } else {i++;}

		while (*e==' ') e++; 

		c++;
	}

	if (c != i*(j+1))
	{
		printf ("Bad matrix defintion ?\n");
		m.w=0;m.h=0;m.fstore=0;
		return m;
	}

	//create matrix

	m = newmatrix(i, j+1);
	
	//copy in ts[0-c];

	for (i=0; i<c; i++) m.fstore[i]=ts[i];

	strncpy(exprbuff, savebuff, MAXSTRING); oop=toop;
	e=t;

	return m;
}

/**********************************************************/
/*  print and formating                                   */
/**********************************************************/

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
    else if (r.type == FMAT2)
    {
	fmatprint2(&r.fmat2);	 
    }
    else    
    {               
        rprintfStr("? error - type\n");   
    }
    return;
}

/**********************************************************/
/*  Access variables (read/write)                         */
/**********************************************************/

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

	if (r.type==FMAT2) r.cnt++;

	while (n<nov)
	{
		if (strcmp(p,name)==0)
		{
			freeobj(&varobj[n]);
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

/**********************************************************/
/*  maths functions                                       */
/**********************************************************/

tOBJ omath(tOBJ o1, tOBJ o2, int op)
{
	tOBJ r;
	int i;
	r.type=EMPTY;


	if (dbg) {printf("math op : %d %d %d\n", o1.type, op, o2.type); } //debug info

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

	if (o1.type==STR || o2.type==STR)
	{
		r= makestring(strcat(o1.string,o2.string));
	}

	if (o1.type==FMAT2 && o2.type==FMAT2 && (op==PLUS || op==MINUS || op==DIVD || op==PROD))
	{
		r.type=FMAT2;	
		if (op==PLUS)  r.fmat2 = fadd2(&o1.fmat2,&o2.fmat2, '+') ;
		if (op==MINUS) r.fmat2 = fadd2(&o1.fmat2,&o2.fmat2, '-') ;	
		if (op==DIVD)  r.fmat2 = fadd2(&o1.fmat2,&o2.fmat2, '/') ;	
		if (op==PROD)  r.fmat2 = fadd2(&o1.fmat2,&o2.fmat2, '*') ;		
	}

	if (o1.type==FMAT2 && (o2.type==INTGR || o2.type==FLOAT) && op==PLUS)
	{
		r.type=FMAT2;	
		r.fmat2 = fmatcp(&o1.fmat2);
		for (i=0; i<o1.fmat2.w*o1.fmat2.h; i++)
			r.fmat2.fstore[i] += tofloat(o2);	
	}

	if (o1.type==FMAT2 && (o2.type==INTGR || o2.type==FLOAT) && op==POWR)
	{
		r.type=FMAT2;	
		r.fmat2 = fmatcp(&o1.fmat2);
		for (i=0; i<o1.fmat2.w*o1.fmat2.h; i++)
			r.fmat2.fstore[i] = pow(r.fmat2.fstore[i],tofloat(o2));	
	}

	if (o1.type==FMAT2 && (o2.type==INTGR || o2.type==FLOAT) && op==MULT)
	{
		r.type=FMAT2;	
		r.fmat2 = fmatcp(&o1.fmat2);
		for (i=0; i<o1.fmat2.w*o1.fmat2.h; i++)
			r.fmat2.fstore[i] = r.fmat2.fstore[i] * tofloat(o2);	
	}

	if (o1.type==FMAT2 && (o2.type==INTGR || o2.type==FLOAT) && op==DIVD)
	{
		r.type=FMAT2;	
		r.fmat2 = fmatcp(&o1.fmat2);
		for (i=0; i<o1.fmat2.w*o1.fmat2.h; i++)
			r.fmat2.fstore[i] /= tofloat(o2);	
	}

	if (o1.type==FMAT2 && (o2.type==INTGR || o2.type==FLOAT) && op==MINUS)
	{
		r.type=FMAT2;	
		r.fmat2 = fmatcp(&o1.fmat2);
		for (i=0; i<o1.fmat2.w*o1.fmat2.h; i++)
			r.fmat2.fstore[i] -= tofloat(o2);	
	}

	if (o1.type==FMAT2 && o2.type==FMAT2 && op==MULT)
	{
		r.type=FMAT2;	
		r.fmat2 = fmultiply2(&o1.fmat2,&o2.fmat2) ;	
		if (r.fmat2.w*r.fmat2.h == 0)
		{
			r.type=EMPTY;
		}
	}

	return r;
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

void ocos()
{
	tOBJ r,a;
	r.type=FLOAT;
	r.floatpoint=0.0;
	a=pop();
	if (a.type==INTGR)
	{
		r.type = INTGR;
		r.number = Cos(a.number%256);
	}

	if (a.type==FLOAT)
	{
		r.floatpoint=cos(a.floatpoint);
	}
	push(r);
	return ;
}

void otan()
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=tan(tofloat(pop()));
	push(r);
	return ;
}

void olog()
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=log(tofloat(pop()));
	push(r);
	return ;
}

void oacos()
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=acos(tofloat(pop()));
	push(r);
	return ;
}


void oatan()
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=atan(tofloat(pop()));
	push(r);
	return ;
}

void oabs()
{
	tOBJ a,r;
	int i;
	a = pop();
	r.type=EMPTY;

	if (a.type==FMAT2)
	{
		r.type = FMAT2;	
		r.fmat2 = fmatcp(&a.fmat2);
		for (i=0; i<a.fmat2.w*a.fmat2.h; i++)
		{
			r.fmat2.fstore[i] = fabs(r.fmat2.fstore[i]);
		}
	}
	else
	{
		r.type=FLOAT;
		r.floatpoint=fabs(tofloat(a));
	}
	push(r);
	return ;
}

void oexp()
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=exp(tofloat(pop()));
	push(r);
	return ;
}

void osqrt()
{
	//binary signmoid function
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=sqrt(tofloat(pop()));
	push(r);
	return ;
}

/**********************************************************/
/*  matrix function (loating point)                       */
/**********************************************************/
/*
!LET MA=[1.0 2.0;3.0 4.0]
!print cell(ma,1,1)
!PRINT DSIG(MA)
!print apply(ma, "expr")
*/
void omat() 
{
	//binary signmoid function
	tOBJ r,m;
	int row,col;
	r=makefloat(0.0);
	row=toint(pop());
	col=toint(pop());
	m=pop();

	if (m.type==FMAT2) 
	{
		r.floatpoint = fget2(&(m.fmat2),col,row);
	}

	push(r);
	return ;
}

void osig()
{
	//binary signmoid function
	tOBJ r, a;
	int i;
	a=pop();

	if (a.type==FMAT2)
	{
		r.type=FMAT2;
		// create new matrix
		r.fmat2=fmatcp(&a.fmat2);
		for (i=0; i<a.fmat2.h*a.fmat2.w; i++)
		{
			float f= a.fmat2.fstore[i];
			r.fmat2.fstore[i] =1/(1+exp(-f));
		}			
	}
	else
	{
		r.type=FLOAT;
		r.floatpoint=1/(1+exp(-tofloat(a)));
	}
	push(r);
	return ;
}

 
void ohsum()
{
	//binary signmoid function
	tOBJ r, a;
	r.type=EMPTY;
	a=pop();

	if (a.type==FMAT2)
	{
		r.type=FMAT2;
		// create new matrix
		r.fmat2=fmatsum2(&a.fmat2, 1);	
	}
	push(r);
	return ;
}

void ovsum()
{
	//binary signmoid function
	tOBJ r, a;
	r.type=EMPTY;
	a=pop();

	if (a.type==FMAT2)
	{
		r.type=FMAT2;
		// create new matrix
		r.fmat2=fmatsum2(&a.fmat2, 2);	
	}
	push(r);
	return ;
}

void odsig()
{
	//derivative binary sigmoid
	tOBJ r,a;

	int i;
	r=makefloat(0.0);
	a=pop();

	if (a.type==FLOAT)
	{
		r.floatpoint=a.floatpoint*(1-a.floatpoint);
	}

	if (a.type==FMAT2)
	{
		r.type=FMAT2;
		// create new matrix
		r.fmat2=fmatcp(&a.fmat2);
		for (i=0; i<a.fmat2.h*a.fmat2.w; i++)
		{
			float f= a.fmat2.fstore[i];
			r.fmat2.fstore[i] = f*(1-f);
		}			
	}

	push(r);
	return ;
}

void oapply()
{
	//apply (<matrix>, <string>)
	tOBJ r,a,b, v;
	int i=0, j=0, w, h;
	char *expr;

	r.type=EMPTY;
	a=pop();
	b=pop();

	if (a.type !=STR && b.type != FMAT2)
	{
		return;
	}

	r.type=FMAT2;
	// create new matrix
	r.fmat2=fmatcp(&b.fmat2);

	w=b.fmat2.w;
	h=b.fmat2.h;

	v.type=FLOAT;

	expr = a.string;

	if (dbg) printf ("Apply - %s\n", expr);
	
	for (i=0; i<w; i++)
	{
		for (j=0;j<h; j++)
		{
			v.floatpoint = fget2(&b.fmat2,i,j);
			set("ME",v);
			//v.floatpoint = (float)j;
			//set("ME_R",v);
			//v.floatpoint = (float)i;
			//set("ME_C",v);
			v=eval_oxpr(expr);
			fset2(&r.fmat2,i,j,tofloat(v));
		}
	}


	push(r);
	return ;
}

void otrn()
{
	tOBJ r,a;
	r=makefloat(0.0);
	a=pop();
	if (a.type==FMAT2)
	{
		r.type = FMAT2;
		r.fmat2 = ftranspose2(&a.fmat2); 
	}

	push(r);
	return ;
}

void orshp()
{	// RSHP(M,x,y)
	tOBJ r,m;
	int row,col;
	r.type=EMPTY;

	row=toint(pop());
	col=toint(pop());

	m=pop();
	if (m.type==FMAT2)
	{
		r.type=FMAT2;
		r.fmat2 = fmatrshp(&m.fmat2, col,row);
	}
	push(r);
	return ;
}


void orep ()
{	// REP(M,x,y)
	tOBJ r,m;
	int row,col;
	r.type=EMPTY;

	row=toint(pop());
	col=toint(pop());

	m=pop();
	if (m.type==FMAT2)
	{
		r.type=FMAT2;
		r.fmat2 = freplicate2(&m.fmat2, col,row);
	}
	push(r);
}


void ozero ()
{
	tOBJ r;
	int row,col;
	r.type=EMPTY;

	row=toint(pop());
	col=toint(pop());

	r.type=FMAT2;
	r.fmat2 = newmatrix(col, row);
	push(r);
}

void oeye ()
{
	tOBJ r;
	int i;
	int row,col;
	r.type=EMPTY;

	row=toint(pop());
	col=toint(pop());

	r.type=FMAT2;
	r.fmat2 = newmatrix(col, row);
		
	for (i=0; i<col; i++)
	{
		if (i<row) 
			r.fmat2.fstore[i+i*col] = 1.0;
	}
	push(r);
}

void oconv ()
{
	tOBJ r, a, b;

	r.type=EMPTY;

	b=pop(); a=pop();

	if (a.type != FMAT2 && b.type!=FMAT2)
		return;

	r.type=FMAT2;
	r.fmat2=fconvolve2(&a.fmat2,&b.fmat2) ;
		
	push(r);
}

void ocond()
{
	// COND (Matrix,lv, uv, nv1, nv2)
	tOBJ r, a, b, c;
	int i;
	float lv=0,uv=0,nv1=0,nv2=0;
	nv2 = tofloat(pop());
	nv1 = tofloat(pop());
	uv = tofloat(pop());
	lv = tofloat(pop());
	a = pop();
	r.type=EMPTY;
	push(r);
	if (a.type==FMAT2)
	{
		r.type = FMAT2;	
		r.fmat2 = fmatcp(&a.fmat2); // clone
		for (i=0; i<a.fmat2.w*a.fmat2.h; i++)
		{
			if (r.fmat2.fstore[i]>= lv && r.fmat2.fstore[i]<= uv)
				r.fmat2.fstore[i] = nv1;
			else
				r.fmat2.fstore[i] = nv2;
		}
	}
	push(r);
}

void ozerob()
{
	// ZER (Matrix,c1,r1,c2,r2)
	tOBJ r, a;
	int c1=0,c2=0,r1=0,r2=0;
	r2 = toint(pop());
	c2 = toint(pop());
	r1 = toint(pop());
	c1 = toint(pop());
	a = pop();
	r.type=EMPTY;
	if (a.type==FMAT2)
	{
		r.type = FMAT2;	
		r.fmat2 =fmatzeroregion(&a.fmat2, c1, r1, c2, r2)   ;
	}
	push(r);
}

void ozerod()
{	
	// ZERD (Matrix)
	tOBJ r, a;
	a=pop();
	r.type=EMPTY;
	if (a.type==FMAT2)
	{
		r.type = FMAT2;
		r.fmat2 = fmatzerodiag2(&a.fmat2) ;
	}
	push(r);
}

void oimp ()
{
	//IMP("A",2,2);

	tOBJ r, a, b, c;

	r.type=EMPTY;

	c=pop(); b=pop(); a=pop();

	if (a.type != STR)
		return;

	r.type=FMAT2;
	r.fmat2=fimport2(*(a.string), toint(b), toint(c)) ;
		
	push(r);
}

/**********************************************************/
/*  Access sensors                                        */
/**********************************************************/

void opsd()
{
	tOBJ r;
	Get_AD_PSD();
	r.type=INTGR;
	r.number=gDistance;
	push(r);
	return;
}

void oacx()
{
	tOBJ r;
	Acc_GetData();
	r.type=INTGR;
	r.number=x_value;
	push(r);
	return;
}

void oacy()
{
	tOBJ r;
	Acc_GetData();
	r.type=INTGR;
	r.number=y_value;
	push(r);
	return;
}

void oacz()
{
	tOBJ r;
	Acc_GetData();
	r.type=INTGR;
	r.number=z_value;
	push(r);
	return;
}

void ornd()
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=(float)rand()/RAND_MAX;
	push(r);
	return;
}

void omax()
{
	tOBJ r,a;
	int i;
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

	if (a.type==FMAT2)
	{
		r.type=FLOAT;
		r.floatpoint=a.fmat2.fstore[0];
		for (i=1; i<a.fmat2.h*a.fmat2.w; i++)
			if (a.fmat2.fstore[i]>r.floatpoint) r.floatpoint= a.fmat2.fstore[i];
	}

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
	int tk = get_token(0);
	
	if (s==(char *)0 && tk==ALPHA)
		return 1;

	if (tk==ALPHA && !strcmp(tokbuff,s))
		return 1;
	return 0;
}

int get_integer()
{
	int tk = get_token(0);
	
	if (tk==NUMI)
		return tnum;
	return 0;
}

int get_opr_token(unsigned char op)
{
	int tk = get_token(1);
	if (tk==OPR && oplist[getOP(tokbuff)].type==op)
		return 1;
	return 0;
}

/***********************************************************************

!BF "++++++++++[>+++++++>++++++++++>+++>+<<<<-]+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>."

***********************************************************************/


void brainf(char *s)
{
	char array[30000];
	char output[100];
	char *op=output;
	char *ptr=array;
	int i;
	for (i=0;i<30000;i++) array[i]=0;

	while (*s != 0)
	{
		if (dbg) printf("%c\n",*s);

		switch (*s) {
		case '>' : if (ptr<(array+29999)) ptr++; 	break;
		case '<' : if (ptr>=array)      ptr--; 		break;
		case '+' : (*ptr)++; 				break;
		case '-' : (*ptr)--; 				break;
		case '.' : printf("%c",*ptr); *op++=*ptr;			break;
		case ',' : *ptr = getchar(); 			break;
		case '[' : if (*ptr==0) while (*s!=']') s++; 	break;
		case ']' : while (*s!='[') s--; s--;		break;
		}
		s++;	
	}
	*op=0;
}

/***********************************************************************

Extended input

************************************************************************/

static int  intf=1;


void extend(char *x)
{
	tOBJ v,file;
	int val;

	if (intf) { // set up defaults
		intf=0; 
		set("PI",  makefloat (3.1415926));
		set("DFN", makestring("data.txt"));
		set("IFN", makestring("test.jpg"));
		
		//seed the RND Gen
		srand ( (unsigned)time ( NULL ) );
	}

	e=x;
	if (get_str_token("LET")==1)
	{
		if (get_str_token(NULL)==1)
		{	
			char var[5];
			int tk;
			strncpy(var,tokbuff, 5);
			tk = get_token(1);	
			if (tk==OPR && oplist[getOP(tokbuff)].type==EQL)
			{
				v=eval_oxpr(e);
				set(var,v);
				return;
			}
			if (tk==OPR && oplist[getOP(tokbuff)].type==OBR)
			{
				/*
				!LET MX=[1 2 3]
				!LET MX(2,0)=5
				!PRINT MX
				*/
				int c,r;
				float f;
				tOBJ R;

				char *t;
				t=strchr(e,',');
				if (t==NULL) return;
				*t='\0';
				t++;

				v=eval_oxpr(e);	
				c=toint(v);
				e=t;
				t=strchr(e,')');
				if (t==NULL) return;
				*t='\0';
				t++;

				//printf ("DBG e=%s\n",e);

				v=eval_oxpr(e);	
				r=toint(v);

				e=t;
				t=strchr(e,'=');
				if (t==NULL) return;
				*t='\0';
				e=t+1;

				//printf ("DBG e=%s\n",e);
				v=eval_oxpr(e); 

				f=tofloat(v);
				R=get(var);
				if (R.type == FMAT2)
				{
					if (dbg) printf ("DBG var=%s %d,%d, %f\n",var,c,r,f);
					fset2(&R.fmat2,c,r,f);
				}
				return;
			}
		}
	}

	e=x;
	if (get_str_token("BF")==1)
	{
		//brainf("++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.");

		v=eval_oxpr(e);
		if (v.type==STR)
			brainf(v.string);
		else
			printf("need a string argument");
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
			val=v.number;
			v=get("DFN");
			matrixload(val, v.string);
		}

		if (!strcmp(tokbuff,"STOR"))
		{
			if (*e != '\0')
				v=eval_oxpr(e);
			else
				v.number=nis;

			val=v.number;
			v=get("DFN");
			matrixstore(val, v.string);
		}
	}

#ifdef IMAGE

	e=x;
	if (get_str_token("IMAG")==1)
	{
		if (get_str_token("UNLO")==1)
		{
			rellock();
			return;
		}

		if (!strcmp(tokbuff,"LOAD"))
		{
			int sz = 8;
			if (*e != '\0')
				v=eval_oxpr(e);
			if (v.type==INTGR &&  v.number< sqrt(SCENESZ))
			{
				sz=v.number;
			}
			else
			{
        			rprintfStr("error = expect int size 1-%d\n", sqrt(SCENESZ)); 
				return;
			}
			file = get("IFN");	
			if (*e == ';')
			{
				e++;
				file=eval_oxpr(e);

				if (file.type!=STR)
				{
					rprintfStr("error = 2nd arg should be string file name\n"); 
					return;
				}
			}

			if (loadimage(file.string, sz, &scene[0])==0)
				nis=sz*sz;
			else
				nis=0;
			return;
		}


		if (!strcmp(tokbuff,"FILT"))
		{
			//!IMAGE FILTER 4;1;10;10;100;100;100
			int args[6];
			int sz,i;

			v=eval_oxpr(e);
			sz=v.number;
			if (*e++ != ';')
				return;

			for (i=0; i<6; i++)
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

			file = get("IFN");
			if (file.type!=STR)
			{
				rprintfStr("error in string file name\n"); 
				return;
			}
			
			if (filterimage(file.string,&scene[0],sz,args[0],args[1],args[2],args[3],args[4],args[5])==0)
				nis=sz*sz;
			else
				nis=0;
			return;
		}

		if (!strcmp(tokbuff,"RAW"))
		{
			file = get("IFN");
			if (*e != '\0')
				file=eval_oxpr(e);

			if (file.type!=STR)
			{
				rprintfStr("error = expecting string\n"); 
				return;
			}

			loadJpg(file.string);
			e++;
			return;
		}

		if (!strcmp(tokbuff,"THRE"))
		{
			//!IMAGE THRE 0
			//!IMAGE THRE 1;10;10;100;100;100
			int args[6];
			int n;


			if (*e++ == '\0')
			{
        			rprintfStr("clr thresholds\n"); 
				clrmap();
			}
			else
			{
				int i;
				v=eval_oxpr(e);
				n=v.number;
				e++;
				for (i=0; i<6; i++)
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
				//add_thresh(1, 120, 175, 40, 70, 30, 40);
				//!IMAGE THRE CID;120; 175; 40; 70; 30; 40
				add_thresh(n, args[0],args[1],args[2],args[3],args[4],args[5]);
			}
			return;
		}

		if (!strcmp(tokbuff,"COLO"))
		{
			int args[6];
			char color[10];

			if (*e++ == '\0')
			{
				clear_colors();
			}
			else
			{
				int i;
				int tk = get_token(0);

				if (tk !=STRNG || *e++ != ';')
				{
					rprintfStr("need colour name\n");
					return; 
				}
				strncpy(color,tmpstr,10);

				for (i=0; i<4; i++)
				{
					v=eval_oxpr(e);
					args[i]=v.number;
					if (*e == ';' || (i==3 && *e=='\0'))
					{
						e++;
					}
					else
					{
						rprintfStr("incorrect number args (5)\n"); 
						return;
					}
				}
				//add_color("orange", 20,30,60,2);
				//!IMAGE COLO "orange";20;30;60;2
				v.number =add_color(color, args[0],args[1],args[2],args[3]);
				v.type   =INTGR; 
				set("CID", v);
			}
			return;
		}

		if (!strcmp(tokbuff,"PROC"))
		{
			if (*e != '\0')
				v=eval_oxpr(e);
			else
				v.number=4;
			//
			processFrame(v.number, &scene[0]);
			nis=v.number*v.number;
			return;
		}

		if (!strcmp(tokbuff,"REG"))
		{
			if (*e != '\0')
				v=eval_oxpr(e);
			else
				v.number=1;

			get_color_region(v.number);
			return;
		}

		if (!strcmp(tokbuff,"SHOW"))
		{	
			int sz=sqrt(nis);
			frame=&scene[0];

			if (*e != '\0')
				v=eval_oxpr(e);
			else
				v.number=1;

			switch (v.number) {
			case 0: 
			case 1: 
			case 2:
			case 3:
			case 4:
				showImage(v.number); break;
			case 5:
				output_grey1(sz); break;
			case 6:
				output_frame(sz); break;
			}
			return;
		}

		if (!strcmp(tokbuff,"DEBU"))
		{
			dbg=1-dbg;
			return;
		}

		rprintfStr("No such option?\n");
		freeobj(&v);
		return;
	}
#endif

	e=x;
	if (get_str_token("PRIN")==1)
	{
		while (*e!='\0')
		{
			v=eval_oxpr(e);
			if (*e==',') 
			{
				print(v);
				rprintf(" ");

			}
			else if (*e==';') 
			{
				print(v);
				e++;
			}
			else if (*e!='\0') 
			{
				printf ("synatx error at '%c'\n", *e);
				return;
			}
			else
			{
				print(v);
				rprintfCRLF();
			}
			
		}
		freeobj(&v);
		return;
	}


#ifdef LINUX
	e=x;
	if (get_str_token("EXIT")==1)
	{
		sigcatch();
	}
#endif

	e=x;
	v=eval_oxpr(e);
	print(v);
	rprintfCRLF();

	freeobj(&v);
}



