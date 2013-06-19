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


// new float matrices

extern int   fmatrixcreate	(char m, int w, int h);
extern float fget		(char m,int w,int h);
extern int   fgetw		(char m);
extern int   fgeth		(char m);
extern int   fset		(char m,int w,int h, float v);
extern int   fmatzero		(char m, int a, int b, int c, int d);
extern int   fmatzerodiag	(char m) ;
extern int   fmatprint		(char m);
extern int   ftranspose		(char m, char lnx);
extern int   fmultiply		(char m, char ln2, char lnx);
extern int   fhistogram		(char m, int mode);
extern int   fconvolve		(char m, char ln2, char lnx);
extern int   fadd		(char m, char m2, char lnx, char op);
extern int   fsize		(char m, int p);
extern int   fimport		(char m, char m2);
extern int   fimportf		(char m, char m2, float rd);
extern int   fmatnorm		(char ln1);
extern int   fmatcopy		(char ma, char mb);

//matrix.c matrix fuctions 
extern void matzero	(char ln1, int a, int b, int c, int d) ;
extern void transpose	(char ln1) ;  				// @A^T"
extern void multiply	(char ln1, char ln2, char lnx);   	// "@X = @A * @B"//
extern void convolve	(char ln1, char ln2);   		// "@A (.) @B"
extern void histogram	(char ln1, int mode);   		// "@A 

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


// The new expression parser
// work in progress
//

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

#define iswhite(c)   (c == ' ' || c == '\t')
#define isnumdot(c)  ((c >= '0' && c <= '9') || c == '.')
#define isnumber(c)  (c >= '0' && c <= '9')

#define isdelim(c)  (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '<'  || c == '>'  \
                    || c == '^' || c == '(' || c == ')' || c == ',' || c == '='  || c == '$')

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

tOBJ get(char *name);
int  set(char *name, tOBJ r);

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
	{",",    50, COMMA, 1, NULL},
	{"AND",  8,  LAND,  2, NULL},
	{"OR",   8,  LOR,   2, NULL},
	{"SIN",  40, NA,    1, osin},  //function single arg
	{"COS",  40, NA,    1, ocos},  //function single arg
	{"TAN",  40, NA,    1, otan},  //function single arg
	{"ATAN", 40, NA,    1, oatan},  //function single arg
	{"ACOS", 40, NA,    1, oacos},  //function single arg
	{"LOG",  40, NA,    1, olog},  //function single arg
	{"EXP",  40, NA,    1, oexp},  //function single arg
	{"SQRT", 40, NA,    1, osqrt},  //function single arg
	{"SIG",  40, NA,    1, osig},  //sigmoid functon
	{"DSIG", 40, NA,    1, odsig},  //sigmoid functon
	{"PSD",  40, NA,    0, opsd},  //const
	{"ACCX", 40, NA,    0, oacx},  //const
	{"ACCY", 40, NA,    0, oacy},  //const
	{"ACCZ", 40, NA,    0, oacz},  //const
	{"ABS",  40, NA,    1, oabs},  //function single arg
	{"TRN",  40, NA,    1, otrn},  //function single arg
	{"RND",  40, NA,    0, ornd},  //in-const
	{"MAX",  40, NA,    2, omax},   //function two args
	{"CELL", 40, NA,    3, omat},   //function two args
};

tOBJ omath(tOBJ o1, tOBJ o2, int op);
tOBJ print(tOBJ r);

/**********************************************************/
/*  strings                                              */
/**********************************************************/



char strings[MAXSTRING], *estr=&strings[0];
char *newstring(char *s)
{
	char *r=estr;
	if (estr-&strings[0]>MAXSTRING-1)
		return 0;
	strcpy(estr, s);
	estr=estr+strlen(s);
	*estr++='\0';
	return r;
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
	return 0;
}

int push(tOBJ a)
{
if (dbg) printf ("PUSH %d\n", a.type);

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

if (dbg) printf ("POP %d\n", e.type);

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
		while (*e != '"')
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
		while (*e != ']')
		{
			*p++=*e++;
		}
		e++;
		*p='\0';
		return MLIST;
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
		//
	}

	if (oplist[i].func != NULL)
	{
		if(oplist[i].nop>stacksize())
		{
			rprintf("Insuffcient parameters\n");
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

		if (gnf==0 || (tk==OPR))
		{
			switch (tk)
			{
			case ALPHA:
				r = get(tokbuff);
				push(r);
				gnf=1;
				break;
			case STRNG:
				r.type=STR;
				r.string = newstring(tmpstr);
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
					
				continue;
				}
			case OPR:
				op = getOP(tokbuff);
				if (oplist[op].nop==0 && oplist[op].func != NULL)
				{
					(*oplist[op].func)();
					continue;
				}
				else
				if (oop>0 && (oplist[op].type == CBR || oplist[op].type==COMMA))
				{
					int i=stackop[oop-1];
					while (oplist[i].type != OBR && oplist[i].type != COMMA)
					{
						//stackprint();
						reduce();
						i=stackop[oop-1];
					}
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
				break;
			case DELI:
				lf=0;
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
		if (dbg) printf("Reduce\n");
		reduce();
	}

/*	while (oop==0 && stacksize()>1)
	{
		tOBJ a,b;
		int op=getOP("+");
		b = pop();
		a = pop();
		a = omath(a, b, oplist[op].type );
		push(a);
	}

	if (stacksize()>1)
	{
		printf("Suntax error - too many on stack\n");
		stackprint();
		clear();
	}
*/
	return pop();
}

float tofloat(tOBJ v);

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
*/

fMatrix readmatrix(char *s)
{
	fMatrix m;
	float ts[10000];
	char *t;
	char savebuff[MAXSTRING];
	tOBJ v;
	float f;
	int i=0;
	int j=0;
	int c=0;

	strncpy(savebuff, exprbuff, MAXSTRING);
	t=e;

	e=s;

	if (dbg) printf("LIST=%s\n", e);

	while (*e != 0)
	{
		v=eval_oxpr(e);	
		ts[c] = tofloat(v);

		if (dbg) printf ("[%d,%d] = %f  (%s)\n", i,j,ts[c], e);

		if (*e==';') { j++; i=0; e++; } else {i++;}

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

	strncpy(exprbuff, savebuff, MAXSTRING);
	e=t;

	return m;
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
    else if (r.type == FMAT)
    {
	fmatprint(r.fmat);	 
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

tOBJ omath(tOBJ o1, tOBJ o2, int op)
{
	tOBJ r;
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
		r.type=STR;
		r.string = newstring(strcat(o1.string,o2.string));
	}

	if (o1.type==FMAT2 && o2.type==FMAT2 && op==PLUS)
	{
		r.type=FMAT2;	
		r.fmat2 = fadd2(&o1.fmat2,&o2.fmat2, '+') ;	
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

fMatrix ftranspose2(fMatrix *A)  ;

void otrn()
{
	tOBJ r,a;
	r.type=FLOAT;
	r.floatpoint=0.0;
	a=pop();
	if (a.type==FMAT2)
	{
		r.type = FMAT2;
		r.fmat2 = ftranspose2(&a.fmat2); 
	}

	push(r);
	return ;
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
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=fabs(tofloat(pop()));
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

void osig()
{
	//binary signmoid function
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=1/(1+exp(-tofloat(pop())));
	push(r);
	return ;
}
/*
> !MAT DEF A=1;3
> !MAT LET A=1.0;2.0;3.0
> !PRINT CELL("A",0,1)
2.000000
*/
void omat() 
{
	//binary signmoid function
	tOBJ r,m;
	int row,col;
	r.type=FLOAT;
	r.floatpoint=0.0;
	row=toint(pop());
	col=toint(pop());
	m=pop();
	if (m.type==STR) 
	{
		r.floatpoint = fget(m.string[0],col,row);
	}
	push(r);
	return ;
}

void odsig()
{
	//derivative binary sigmoid
	tOBJ r,a;
	r.type=FLOAT;
	r.floatpoint=0.0;
	a=pop();

	if (a.type==FLOAT)
	{
		r.floatpoint=a.floatpoint*(1-a.floatpoint);
	}
	push(r);
	return ;
}

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


void omax()
{
	tOBJ r,a;
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

	push(r);
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

************************************************************************/

static int  intf=1;

void mexpress(char m)
{
	tOBJ v;
	
	int i=0, j=0;
	int w=fsize(m,0);
	int h=fsize(m,1);

	if (*e == '@')
	{
		//!MAT LET A=@B
		char im;
		e++;
		im=*e++;

		fimport(m,im);
		return;
	}

	if (*e >= 'A' && *e <='Z' && *(e+1)==';')
	{
		//!MAT LET A=B;1.0
		char im=*e;
		e+=2;
		v=eval_oxpr(e);		
		fimportf(m, im, tofloat(v));
		return;
	}

	if (*e >= 'A' && *e <='Z')
	{
		//!MAT LET A=B+C
		char op,mb;
		char ma=*e++;

		if (*e==0)
		{
			fmatcopy(ma, m);
			return;
		}

		while (*e != 0)
		{
			op=*e++;

			if (op == '^')
			{
				// transpose
				ftranspose(ma,m);
				ma=m;
				continue;
			}

			if (get_str_token(0))
			{
				mb=tokbuff[0];

				if (*e=='^')
				{
					ftranspose(mb,mb); //this overwrites
					e++;
				}
				
				if (op == '*')
				{
					// mult
					fmultiply(ma,mb,m);
				}
				else
				if (op == '+' || op == '-' || op  == '.' || op == '/')
				{
					if (op=='.') op='*';
					fadd(ma,mb,m,op);
				}
				else
				if (op == '#')
				{
					//convolve
					fconvolve(ma, mb, m) ;
				}
				else
				{
					printf("? invalid MAT op %c\n", op);
					return;
				}
				ma=m;
			}
		}
		return;
	}


	while (*e != 0)
	{
		v=eval_oxpr(e);	
		fset(m,i,j,tofloat(v));
		if (++i == w)
		{
			i=0; j++;
		}
		if (j==h)
			return;

		if (*e != ';')
		{
			printf ("Incorrect args [%c] (%d,%d) %d ?\n",*e,i,j,h*w);
			return;
		}
		e++;
	}
}

void extend(char *x)
{
	tOBJ v,file;
	int val;

	if (intf) { // set up defaults
		intf=0; 
		v.type=FLOAT; v.floatpoint=3.1415926;  set("PI", v);
		v.type=STR;   v.string    ="data.txt"; set("DFN", v);
		v.type=STR;   v.string    ="test.jpg"; set("IFN", v);
		
		//seed the RNG
		srand ( (unsigned)time ( NULL ) );
	}

	e=x;
	if (get_str_token("LET")==1)
	{
		if (get_str_token(NULL)==1)
		{	
			char var[5];
			strncpy(var,tokbuff, 5);
			if (get_opr_token(EQL)==1)
			{
				v=eval_oxpr(e);
				set(var,v);
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
	if (get_str_token("PRIN")==1)
	{
		while (*e!='\0')
		{
			v=eval_oxpr(e);
			print(v);
			if (*e!=';') 
			{
				rprintfCRLF();
			}
			else
				e++;
		}
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

		if (!strcmp(tokbuff,"DEF"))
		{
			// !MAT DEF A;1;2
			char m; int w; int h;
			if (*e != '\0')
			{
				if (get_str_token(0))
				{					
					m=tokbuff[0];
					if (*e++ == '=')
					{
						v=eval_oxpr(e);	
						w=v.number;
						if (*e++ == ';')
						{
							v=eval_oxpr(e);	
							h=v.number;
							if (dbg) printf ("Create matrix '%c' %dx%d\n", m,w,h);
							fmatrixcreate(m,w,h);
							return;
						}
					}
				}
			}
			printf ("MAT DEF - syntax error @ '%c'\n", *e=='\0'?'?':*e);
		}

		if (!strcmp(tokbuff,"PRIN"))
		{
			char m;
			while (*e != '\0')
			{
				if (get_str_token(0))
				{
					m=tokbuff[0];
					fmatprint(m);
					if (*e == ';') e++;	
				}
				else
				{
					printf ("MAT PRINT - syntax error @ '%c'\n", *e=='\0'?'?':*e);
					return;
				}
			}
			return;
		}

		if (!strcmp(tokbuff,"LET"))
		{
			//!MAT LET X=1.3,1.2 .....
			char m;
			if (*e != '\0')
			{
				if (get_str_token(0))
				{					
					m=tokbuff[0];

					if (*e++ == '=')
					{
						mexpress(m);
					}
				}
			}					
		}

		if (!strcmp(tokbuff,"APPL"))
		{
			//!MAT APPLY X=1.2*ME .....
			char *t, m;
			if (*e != '\0')
			{
				if (get_str_token(0))
				{					
					m=tokbuff[0];
					if (*e++ == '=')
					{
						int i=0, j=0;
						int w=fsize(m,0);
						int h=fsize(m,1);
						v.type=FLOAT;
						t=e; 
						for (i=0; i<w; i++)
						{
							for (j=0;j<h; j++)
							{
								v.floatpoint = fget(m,i,j);
								set("ME",v);
								v.floatpoint = (float)j;
								set("MR",v);
								v.floatpoint = (float)i;
								set("MC",v);
								e=t;
								v=eval_oxpr(e);
								fset(m,i,j,tofloat(v));
							}
						}
					}
				}
			}					
		}

		if (!strcmp(tokbuff,"HIST"))
		{
			// !MAT HIST 1;A
			
			int m=1; char ma='A';
			if (*e != '\0')
			{
				v=eval_oxpr(e);	
				m=v.number;
				if (*e++ == ';')
				{				
					if (get_str_token(0))
					{
						ma=tokbuff[0];
						fhistogram(ma, m);
						return;
					}
				}
			}
			printf ("MAT HIST - syntax error @ '%c'\n", *e=='\0'?'?':*e);
		}

		if (!strcmp(tokbuff,"SUM") || !strcmp(tokbuff,"SUM2")  )
		{
			// !MAT SUM X;A;1
			
			int m=1, f=0; char mx; char ma;

			if (!strcmp(tokbuff,"SUM2")) f=2;
 
			if (*e != '\0')
			{
				if (get_str_token(0))
				{
					mx=tokbuff[0];
				}
				if (*e++ == ';')
				{				
					if (get_str_token(0))
					{
						ma=tokbuff[0];

						if (*e++ == ';')
						{	
							v=eval_oxpr(e);	
							m=v.number;
							fmatsum(mx, ma, m+f)  ;
							return;
						}
					}
				}
			}
			printf ("MAT SUM - syntax error @ '%c'\n", *e=='\0'?'?':*e);
		}

		if (!strcmp(tokbuff,"NORM"))
		{
			// !MAT NORM A
			char ma='A';
			if (*e != '\0')
			{			
				if (get_str_token(0))
				{
					ma=tokbuff[0];
					fmatnorm(ma);
				}
			}
		}


		if (!strcmp(tokbuff,"ZERO"))
		{
			// !MAT ZERO A;1;1;2;2 or !MAT ZERO A
			
			int a[4],i;
			char ma='A';
			if (*e != '\0')
			{			
				if (get_str_token(0))
				{
					ma=tokbuff[0];
 					if (*e != ';')
				        {
				                //MAT ZERO A -> set diagnalo to zero
				                fmatzerodiag(ma);
				                return;
				        }

					for (i=0; i<4; i++)
					{
						if (i<3 && *e != ';')
							return;
						e++;
						v=eval_oxpr(e);
						a[i]=v.number;
					}
					fmatzero(ma,a[0],a[1],a[2],a[3]);
					return;
				}
	
			}
			printf ("MAT ZERO - syntax error @ '%c'\n", *e=='\0'?'?':*e);
		}

		if (!strcmp(tokbuff,"REP"))
		{
			// !MAT REP B;A;1;1
			
			int m1,n1,i;
			char mx='A',ma='A';
			if (*e != '\0')
			{			
				if (get_str_token(0))
				{
					mx=tokbuff[0];	

					if (*e++ != ';')
				        {
				                return;
				        }
					ma=tokbuff[0];	

					if (*e++ != ';')
				        {
				                return;
				        }
						
					v=eval_oxpr(e);
					m1=v.number;

					if (*e++ != ';')
				        {
				                return;
				        }

					v=eval_oxpr(e);
					n1=v.number;
					freplicate(mx, ma,  m1,  n1);

					return;
				}
	
			}
			printf ("MAT ZERO - syntax error @ '%c'\n", *e=='\0'?'?':*e);
		}
		return;
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
		return;
	}
#endif

#ifdef LINUX
	e=x;
	if (get_str_token("EXIT")==1)
	{
		sigcatch();
	}
#endif

	rprintfStr("No match ?\n");
}

void testeval()
{
	while (1)
	{
		readLine(exprbuff);
		if (exprbuff[0]=='.') break;

		extend(exprbuff);

	}
}


