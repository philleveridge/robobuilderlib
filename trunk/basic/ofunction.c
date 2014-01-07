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

//#include "express.h"


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
extern int  *frame;

extern void	setvar(char n, long v);
extern long	getvar(char n);

#include "functions.h"
#include "lists.h"

#include "oobj.h"
#include "ocells.h"
#include "ostring.h"
#include "odict.h"
#include "ostack.h"
#include "fmatrix.h"
#include "rbmread.h"

#include "ofunction.h"


#include "trex.h"

#ifdef _UNICODE
#define trex_sprintf swprintf
#else
#define trex_sprintf sprintf
#endif

#define isalpha(c)  ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')

tOP oplist[] = { 
/* 0 */	{"+",    10, PLUS,  2, NULL},
	{"-",    10, MINUS, 2, NULL},
	{"/",    20, DIVD,  2, NULL},
	{"*",    20, MULT,  2, NULL},
/* 5 */	{".*",   20, PROD,  2, NULL},
	{".^",   20, POWR,  2, NULL},
	{"AND",  8,  LAND,  2, NULL},
	{"OR",   8,  LOR,   2, NULL},
	{"MOD",  8,  LMOD,  2, NULL},
/*10 */	{"<",    5,  LT,    2, NULL},
	{">",    5,  GT,    2, NULL},
	{"=",    5,  EQL,   2, NULL},
	{"<>",   5,  NEQ,   2, NULL},
	{"<=",   5,  LTE,   2, NULL},
	{">=",   5,  GTE,   2, NULL},
	{"(",    50, OBR,   1, NULL},
	{")",    50, CBR,   1, NULL},
	{",",    50, COMMA, 1, NULL},
	{"SIN",  40, NA,    1, osin},  //function single arg
	{"COS",  40, NA,    1, ocos},  //function single arg
	{"TAN",  40, NA,    1, otan},  //function single arg
	{"ATAN", 40, NA,    1, oatan}, //function single arg
	{"ACOS", 40, NA,    1, oacos}, //function single arg
/*20 */	{"LOG",  40, NA,    1, olog},  //function single arg
	{"EXP",  40, NA,    1, oexp},  //function single arg
	{"SQRT", 40, NA,    1, osqrt}, //function single arg
	{"ABS",  40, NA,    1, oabs},  //function single arg
	{"RND",  40, NA,    0, ornd},  //in-const
	{"MAX",  40, NA,    2, omax},   //function two args
	{"SIG",  40, NA,    1, osig},  //sigmoid functon
	{"DSIG", 40, NA,    1, odsig}, //sigmoid functon
	{"PSD",  40, NA,    0, opsd},  //const
	{"ACCX", 40, NA,    0, oacx},  //const
/*30 */	{"ACCY", 40, NA,    0, oacy},  //const
	{"ACCZ", 40, NA,    0, oacz},  //const
//MATRIX BASED
	{"TRN",  40, NA,    1, otrn},   //function single arg    <fMatrix>
	{"CELL", 40, NA,    3, omat},   //function three args   <fMatrix, int, int>
	{"RSHP", 40, NA,    3, orshp},  //function three args  <fMatrix, int, int>
	{"REP",  40, NA,    3, orep},   //function three args  <fMatrix, int, int>
	{"ZERO", 40, NA,    2, ozero},  //function two args  <fint, int>
	{"EYE",  40, NA,    2, oeye},   //function two args  <fint, int>
	{"HSUM", 40, NA,    1, ohsum},  //function <fMatrix>
	{"VSUM", 40, NA,    1, ovsum},  //function <fMatrix>
	{"INV",  40, NA,    1, ominv},  //function <fMatrix>
	{"DET",  40, NA,    1, omdet},  //function <fMatrix>

/*40 */	{"H2SM", 40, NA,    1, ohsum2},  //function <fMatrix>
	{"V2SM", 40, NA,    1, ovsum2},  //function <fMatrix>
	{"APPL", 40, NA,    2, oapply}, //function two args   <fMatrix>
	{"CONV", 40, NA,    2, oconv},  //function three args   <fMatrix>
	{"COND", 40, NA,    5, ocond},  //function three args   <fMatrix>
	{"IMP",  40, NA,    3, oimp},   //function two args   <string>, <int>, <int>
	{"ZERB", 40, NA,    5, ozerob}, //function four args   <fmatrix>, <int> <int> Mint> <int>
	{"ZERD", 40, NA,    1, ozerod},  //function 1 args   <fmatrix>
//LIST BASED
	{"CAR",  40, NA,    1, ocar},  //function single arg
	{"CDR",  40, NA,    1, ocdr},  //function single arg

/*50 */	{"TYPE", 40, NA,    1, otype},  //function single arg
	{"CONS", 40, NA,    2, ocons},  //function single arg
	{"LEN",  40, NA,    1, olen},  //function single arg
	{"SUBS", 40, NA,    1, osubst},  //function single arg
	{"LAST", 40, NA,    1, olast},  //function single arg
	{"REV",  40, NA,    1, orev},  //function single arg
	{"NULL",  40, NA,   1, onull},  //function single arg
	{"MEMB",  40, NA,   1, omemb},  //function single arg
/*60 */	{"ASSN",  40, NA,   1, oasso},  //function single arg
	{"ATOM",  40, NA,   1, oatom},  //function single arg
	{"SET",   40, NA,   1, oset}, //function single arg
	{"GET",   40, NA,   1, oget}, //function single arg
	{"SERV",  40, NA,   0, oserv}, //function single arg
	{"POSE",  40, NA,   1, opose}, //function single arg
	{"NTH",   40, NA,   1, onth}, //function single arg
	{"PR",    40, NA,   1, opr},   //function single arg
	{"WHOS",  40, NA,   0, owhs},
/*70 */	{"BF",    40, NA,   1, obf},
	{"MAT",   40, NA,   3, omatr},// 
	{"DICT",  40, NA,   1, odict},// 
	{"LOAD",  40, NA,   1, oload},//
	{"SAVE",  40, NA,   1, osave},//
	{"REX",   40, NA,   2, orex},//
	{"LFT",   40, NA,   2, olft},//
/*80 */	{"RGT",   40, NA,   2, orgt},//
	{"MID",   40, NA,   3, omid},//
	{"EXIT",  40, NA,   0, oexit},
	{"RBM",   40, NA,   1, orbmrf},
	{"IMAGE", 40, NA,   9, oimg}
};

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

/**********************************************************/
/*  Access variables (read/write)                         */
/**********************************************************/

tOBJ get(Dict *ep, char *name)
{
	tOBJ r;

	if (dict_contains(ep,name))
	{
		return dict_getk(ep, name);
	}

	if (strlen(name)==1 && isalpha(*name)) //backwards compat
	{
		r.type=INTGR;
		r.number=getvar(*name-'A');
		return r;
	}

	if (strlen(name)==2 && *name=='@' && isalpha(*(name+1))) //backwards compat
	{
		char ln=*(name+1);
		int an=listsize(ln);
		int *array=listarray(ln);
		r = cnvtInttoList(an, array);
		return r;
	}
	return emptyObj();
}

int set(Dict * en, char *name, tOBJ r)
{
	if (en != NULL)
	{
		return dict_update(en, name, r);
	}
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

	if (strlen(name)==2 && *name=='@' && isalpha(*(name+1))) //backwards compat
	{
		if (r.type==CELL)
		{
			char ln=*(name+1);
			int an=listsize(ln);
			int *array=listarray(ln);
			int cnt=0;
			while (cnt<an && onull(r).number==0)
			{
				array[cnt++] = toint(ocar(r));
				r=ocdr(r); 
			}
	
		}
		return 1;
	}

	if (r.type==FMAT2 || r.type==STR || r.type==CELL || r.type==DICT || r.type==LAMBDA) r.cnt++;

	return 0;
}

extern tOBJ env;   //TBFixed

tOBJ owhs(tOBJ r)
{
	//display all variables and types
	//!WHOS
	dict_print(env.dict); 
	return emptyObj(); 
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

	if (op == EQL)
	{
		return makeint(compareObj(o1,o2));
	}

	if (op == NEQ)
	{
		return makeint(!compareObj(o1,o2));
	}

	if (op == PLUS && o1.type==CELL)
	{
		// {1 2} + 3 -> {1 2 3}
		// TBD
		tOBJ z;
		tCELLp n=NULL,p= o1.cell;
		r= makeCell();
		n=r.cell;

		while (p!= NULL)
		{
			tCELLp l;
			n->head = p->head;
			z =makeCell();
			l=n;
			n=z.cell;
			p=p->tail;	
			l->tail=n;		
		}
		n->head = cloneObj(o2);
		n->tail=NULL;	
		return r;
	}

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
			if (o2.number ==0) 
				r.type=EMPTY;
			else
				r.number = o1.number / o2.number; 
			break;
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
		case LTE:
			r.number = o1.number <= o2.number; break;
		case GTE:
			r.number = o1.number >= o2.number; break;
		case LMOD:
			r.number = o1.number % o2.number; break;
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
			if (b ==0) 
				r.type=EMPTY;
			else
				r.floatpoint = a / b; 
			break;
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
		int n=strlen(o1.string)+strlen(o2.string)+1;
		char *s=newstring1(n);
		strcat(s,o1.string);
		strcat(s,o2.string);
		r.type=STR;
		r.string=s;
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

tOBJ osin(tOBJ a)
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=0.0;
	if (a.type==INTGR)
	{
		r.type = INTGR;
		r.number = Sin(a.number%256);
	}

	if (a.type==FLOAT)
	{
		r.floatpoint=sin(a.floatpoint);
	}
	return r;
}

tOBJ ocos(tOBJ a)
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=0.0;
	if (a.type==INTGR)
	{
		r.type = INTGR;
		r.number = Cos(a.number%256);
	}

	if (a.type==FLOAT)
	{
		r.floatpoint=cos(a.floatpoint);
	}
	return r;
}

tOBJ otan(tOBJ a)
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=tan(tofloat(a));
	return r;
}

tOBJ olog(tOBJ a)
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=log(tofloat(a));
	return r;
}

tOBJ oacos(tOBJ a)
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=acos(tofloat(a));
	return r;
}


tOBJ oatan(tOBJ a)
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=atan(tofloat(a));
	return r;
}

tOBJ oabs(tOBJ a)
{
	tOBJ r;
	int i;
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
	return r;
}

tOBJ oexp(tOBJ a)
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=exp(tofloat(a));
	return r;
}

tOBJ osqrt(tOBJ a)
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=sqrt(tofloat(a));
	return r;
}

tOBJ ornd(tOBJ a)
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=(float)rand()/RAND_MAX;
	return r;
}

tOBJ omax(tOBJ a, tOBJ r)
{
	int i;

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

	return r;
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
tOBJ omat(tOBJ m, tOBJ a, tOBJ b) 
{
	//binary signmoid function
	tOBJ r;
	int row,col;
	r=makefloat(0.0);
	row=toint(a);
	col=toint(b);

	if (m.type==FMAT2) 
	{
		r.floatpoint = fget2(&(m.fmat2),col,row);
	}
	return r;
}

tOBJ ominv(tOBJ a)
{
	//invert a matrix
	tOBJ r;
	if (a.type==FMAT2) 
	{
		r.type=FMAT2;
		r.fmat2=inverse(&a.fmat2);
	}
	return r;
}
tOBJ omdet(tOBJ a)
{
	//det of a matrix
	tOBJ r;
	if (a.type==FMAT2) 
	{
		r.type=FLOAT;
		r.floatpoint=detrminant(&a.fmat2, a.fmat2.w);
	}
	return r;
}

tOBJ osig(tOBJ a)
{
	//binary signmoid function
	tOBJ r;
	int i;

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
	return r;
}

 
tOBJ ohsum(tOBJ a)
{
	tOBJ r=emptyObj();

	if (a.type==FMAT2)
	{
		r.type=FMAT2;
		r.fmat2=fmatsum2(&a.fmat2, 1);	
	}
	return r;
}

tOBJ ovsum(tOBJ a)
{
	tOBJ r=emptyObj();

	if (a.type==FMAT2)
	{
		r.type=FMAT2;
		r.fmat2=fmatsum2(&a.fmat2, 2);	
	}
	return r;
}

tOBJ ohsum2(tOBJ a)
{
	tOBJ r=emptyObj();

	if (a.type==FMAT2)
	{
		r.type=FMAT2;
		r.fmat2=fmatsum2(&a.fmat2, 3);	
	}
	return r;
}

tOBJ ovsum2(tOBJ a)
{
	tOBJ r=emptyObj();

	if (a.type==FMAT2)
	{
		r.type=FMAT2;
		r.fmat2=fmatsum2(&a.fmat2, 4);	
	}
	return r;
}

tOBJ odsig(tOBJ a)
{
	//derivative binary sigmoid
	tOBJ r;

	int i;
	r=makefloat(0.0);

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

	return r;
}

tOBJ oapply(tOBJ b, tOBJ a)
{
	//apply (<matrix>, <string>)
	tOBJ r, v;
	int i=0, j=0, w, h;
	char *expr;

	r.type=EMPTY;

	if (a.type !=STR && b.type != FMAT2)
	{
		return r;
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
			set(NULL, "ME",v);
			//v.floatpoint = (float)j;
			//set("ME_R",v);
			//v.floatpoint = (float)i;
			//set("ME_C",v);
			v=eval_oxpr(expr); // TBFixed!!
			fset2(&r.fmat2,i,j,tofloat(v));
		}
	}


	return r;
}

tOBJ otrn(tOBJ a)
{
	tOBJ r;
	r=makefloat(0.0);
	if (a.type==FMAT2)
	{
		r.type = FMAT2;
		r.fmat2 = ftranspose2(&a.fmat2); 
	}

	return r;
}

tOBJ orshp(tOBJ m, tOBJ a, tOBJ b)
{	// RSHP(M,x,y)
	tOBJ r=emptyObj();
	int row,col;
	row=toint(a);
	col=toint(b);

	if (m.type==FMAT2)
	{
		r.type=FMAT2;
		r.fmat2 = fmatrshp(&m.fmat2, col,row);
	}
	return r;
}


tOBJ orep (tOBJ m, tOBJ a, tOBJ b)
{	// REP(M,x,y)
	int row,col;
	tOBJ r=emptyObj();

	row=toint(a);
	col=toint(b);

	if (m.type==FMAT2)
	{
		r.type=FMAT2;
		r.fmat2 = freplicate2(&m.fmat2, col,row);
	}
	return r;
}


tOBJ ozero (tOBJ a, tOBJ b)
{
	tOBJ r;
	int row,col;
	r.type=EMPTY;

	row=toint(a);
	col=toint(b);

	r.type=FMAT2;
	r.fmat2 = newmatrix(col, row);
	return r;
}

tOBJ oeye (tOBJ a, tOBJ b)
{
	tOBJ r;
	int i;
	int row,col;
	r.type=EMPTY;

	row=toint(a);
	col=toint(b);

	r.type=FMAT2;
	r.fmat2 = newmatrix(col, row);
		
	for (i=0; i<col; i++)
	{
		if (i<row) 
			r.fmat2.fstore[i+i*col] = 1.0;
	}
	return r;
}

tOBJ oconv (tOBJ a, tOBJ b)
{
	tOBJ r;
	r.type=EMPTY;

	if (a.type != FMAT2 && b.type!=FMAT2)
		return r;

	r.type=FMAT2;
	r.fmat2=fconvolve2(&a.fmat2,&b.fmat2) ;		
	return r;
}

tOBJ ocond(tOBJ a, tOBJ b, tOBJ c, tOBJ d, tOBJ e)
{
	// COND (Matrix,lv, uv, nv1, nv2)
	tOBJ r;
	int i;
	float nv2 = tofloat(e);
	float nv1 = tofloat(d);
	float uv  = tofloat(c);
	float lv  = tofloat(b);

	r.type=EMPTY;
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
	return r;
}

tOBJ ozerob(tOBJ a, tOBJ b, tOBJ c, tOBJ d, tOBJ e)
{
	// ZER (Matrix,c1,r1,c2,r2)
	tOBJ r;
	int r2 = toint(e);
	int c2 = toint(d);
	int r1 = toint(c);
	int c1 = toint(b);

	r.type=EMPTY;
	if (a.type==FMAT2)
	{
		r.type = FMAT2;	
		r.fmat2 =fmatzeroregion(&a.fmat2, c1, r1, c2, r2)   ;
	}
	return r;
}

tOBJ ozerod(tOBJ a)
{	
	// ZERD (Matrix)
	tOBJ r;
	r.type=EMPTY;
	if (a.type==FMAT2)
	{
		r.type = FMAT2;
		r.fmat2 = fmatzerodiag2(&a.fmat2) ;
	}
	return r;
}

tOBJ oimp (tOBJ a, tOBJ b, tOBJ c)
{
	//IMP("A",2,2);

	tOBJ r;
	r.type=EMPTY;

	if (a.type != STR)
		return r;

	r.type=FMAT2;
	r.fmat2=fimport2(*(a.string), toint(b), toint(c)) ;
		
	return r;
}

/**********************************************************/
/*  List functions                                       */
/**********************************************************/


tOBJ ocar (tOBJ a)
{
	tOBJ r=emptyObj();
	if (a.type==CELL)
	{
		r=((tCELLp)(a.cell))->head;
	}
	return r;
}
tOBJ ocdr (tOBJ a)
{
	tOBJ r=emptyObj();
	if (a.type==CELL)
	{		
		tCELLp p= a.cell;
		if (p->tail != NULL)
		{
			r.type=CELL;
			r.cell=p->tail;
		}
	}
	return r;
}

tOBJ otype (tOBJ a)
{
	tOBJ r=makeint(a.type);
	return r;
}

tOBJ olen (tOBJ a)
{
	//!LEN {1 2 3} -> 3
	tOBJ r=makeint(0);

	if (a.type==STR)
		r=makeint(strlen(a.string));

	if (a.type==CELL)
	{
		int n=0;
		
		tCELLp p= a.cell;
		if (p==NULL) return makeint(0);
		do
		{
			n++;
			p=p->tail;
		}
		while (p!= NULL);
		r=makeint(n);
	}
	return r;
}

tOBJ ocons (tOBJ a, tOBJ b)
{
	//!CONS 1 '{2 3} -> {1 2 3}
	tOBJ r=emptyObj();

        if (b.type==CELL)
        {
                r=makeCell2(a, b.cell);
        }
	return r;
}

tOBJ olast(tOBJ a)
{
	//!LAST {1 {2 3}} -> {2 3}
	tOBJ r=emptyObj();
	if (a.type==CELL)
	{		
		tCELLp n=NULL,p= a.cell;

		while (p!= NULL)
		{
			n=p;
			p=p->tail;
		}
		if (n!=NULL) r=n->head;
	}	
	return r;
}

tOBJ osubst(tOBJ a)
{
	//!SUBS {2 1 {1 2 3}}
	//{2 2 3}

	tOBJ r=emptyObj();

	if (a.type==CELL)
	{
		tOBJ p1, p2, z;
		tCELLp prev=NULL;

		p2=ocar(a);
		a=ocdr(a);
		p1=ocar(a);

		a=ocar(ocdr(a));
		if (a.type!=CELL)
		{
			printf ("? not a list");
			return r;
		}

		do {
			tOBJ v= ocar(a);

			if (compareObj(v, p1))
			{
				z = makeCell2(p2, NULL);

			}
			else
			{
				z = makeCell2(v, NULL);
			}
		
			a=ocdr(a);
			if (prev == NULL) 
				r =z;
			else
				prev->tail=z.cell;
			prev = (tCELLp)(z.cell);

		} while (onull(a).number==0);	
	}	
	return r;
}
tOBJ orev(tOBJ a)
{
	//!REV {1 2 3} -> {3 2 1}
	tOBJ r=emptyObj();
	if (a.type==CELL)
	{
		tCELLp p=a.cell;
		tOBJ z;

		z=makeCell2(p->head, NULL);	
		p=p->tail;

		while (p != NULL)
		{
			z=makeCell2(p->head,z.cell);
			p=p->tail;
		} 
		r=z;		
	}	
	return r;
}

tOBJ onull(tOBJ a)
{
	//!NULL {} -> 1
	tOBJ r=makeint(0);	
	if ((a.type==EMPTY) 
	|| (a.type==CELL && ((a.cell==NULL || ((tCELLp)(a.cell))->head.type==EMPTY)) )
	|| (a.type==STR  && (a.string==NULL || *(a.string)=='\0')))	
		r = makeint(1);		
	return r;
}

tOBJ oatom(tOBJ a)
{
	//!ATOM 1 -> 1
	return makeint(!(a.type==CELL || a.type==FMAT2));
}

tOBJ omemb(tOBJ a)
{
	//!MEMB {2 {2 3}} -> 1
	//!MEMB {4 {2 3}} -> 0
	if (a.type==CELL)
	{
		tOBJ key = ocar(a);
		tOBJ lst = ocar(ocdr(a));
		tOBJ mem;

		if (lst.type != CELL)
		{
			printf ("Not a list\n");
			return makeint(0);
		}

		do
		{
			mem = ocar(lst);
			if (compareObj(mem, key))
			{
				return makeint(1);
			}
			lst=ocdr(lst); 	// lst=cdr lst				
		}
		while (onull(lst).number==0); 	
	}
	return makeint(0);
}


tOBJ oasso(tOBJ a)
{
	//!ASSN {1 {{1 2} {2 3} {3 4}}} -> {1 2}
	//!ASSN {2 {{1 2} {2 3} {3 4}}} -> {2 3}
	//!ASSN {7 {{1 2} {2 3} {3 4}}} -> NIL

	tOBJ r=emptyObj();
	if (a.type==CELL)
	{
		tOBJ key = ocar(a);
		tOBJ lst = ocar(ocdr(a));

		if (lst.type != CELL)
		{
			printf ("Not a list\n");
			return r;
		}

		do {
			tOBJ pair = ocar(lst);
			tOBJ n = ocar(pair);

			if (compareObj(key,n))  // if car pair = key
			{	
				return pair;				
			}
			lst=ocdr(lst); 	// lst=cdr lst		
		}
		while (onull(lst).number==0); 			
	}	
	return r;
}


tOBJ opr(tOBJ a)
{
	//> !PR 1
	//1
	tOBJ t=emptyObj();
	while (onull(a).number==0 && a.type==CELL)
	{
		t=ocar(a);
		print(t);
		a=ocdr(a);
	}
	if (onull(a).number==0)
	{
		t=print(a);
	}
	return t;
}

tOBJ oset(tOBJ a)
{
/*
!SET {"A" 2 "B" 3}
PRINT A,B
!LET EN=DICT {{"a" 1} {"b" 2.3}}
!GET {EN "b"}
!SET {EN "b" 2.9}
*/
	tOBJ r=emptyObj();
	tOBJ env=emptyObj();
	tOBJ name;
	tOBJ value=emptyObj();

	if (a.type==CELL)
	{
		while(1) {
			name = ocar(a);
			if (name.type ==DICT)
			{
				env= name;
				name = ocar(ocdr(a));
				a = ocdr(a);
			}

			if (name.type==EMPTY) break;
			if (name.type==SYM)
			{
				value = ocar(ocdr(a));
				if (env.type==EMPTY)
				{
					set(NULL, name.string, value);
				}
				else
				{
					dict_update(env.dict, name.string, value);
				}
			}
			a = ocdr(ocdr(a));
		}
		r=value;
	}	
	return r;
}

tOBJ oget(tOBJ a)
{
	//> !GET {"A"}
	//> !GET {A}
	tOBJ r=emptyObj();
	if (a.type==CELL)
	{
		tOBJ name = ocar(a);

		if (name.type==SYM )
			r = get(NULL, name.string);

		if (name.type==DICT)
		{
			tOBJ env = name;
			name = ocar(ocdr(a));
			if (name.type==STR)
				//r = dict_getk(env.dict, name.string);
				r = get(env.dict, name.string);
		}	
	}
	if (a.type==SYM)
	{
		r = get(NULL, a.string);
	}	
	return r;
}


tOBJ onth(tOBJ a)
{
	//!nth {0 {"one" "two" "three"}}
	//!nth {5 {"one" "two" "three"}}
	tOBJ indx = ocar(a);
	if (indx.type==INTGR)
	{
		tOBJ lst = ocar(ocdr(a));
		if (lst.type == CELL)
		{
			int cnt=indx.number;
			while (cnt>0)	
			{
				lst = ocdr(lst);
				cnt--;
			}
			return ocar(lst);
		}
	}
	return emptyObj();
}


/**********************************************************
 
String function  
   
**********************************************************/                                   

tOBJ olft(tOBJ  r, tOBJ a)
{
	//!LFT("test",2) -> "te"
	int p=toint(a);
	char *cp;
	if (r.type==STR)
	{
		int ln = strlen(r.string);
		if (ln>0 && p>0 && ln>p)
		{
			cp=newstring1(p+1);
			strncpy(cp, r.string,p);
			*(cp+p)='\0';
			r.string = cp; //<-is this missing a free?
			return r;
		}				
	}
	return emptyObj();
}

tOBJ orgt(tOBJ  r, tOBJ a)
{
	//!RGT("test",2) -> "st"
	int p=toint(a);
	char *cp;
	if (r.type==STR)
	{
		int ln = strlen(r.string);
		if (ln>0 && p>0 && ln>p)
		{
			cp=newstring1(p+1);
			strncpy(cp, (r.string)+ln-p,p);
			*(cp+p)='\0';
			r.string = cp;
			return r;
		}
	}
	return emptyObj();
}


tOBJ omid(tOBJ  r, tOBJ a, tOBJ  b)
{
	//!MID("testing",2,4) -> "est"
	int p1=toint(a);
	int p2=toint(b);
	char *cp;
	if (r.type==STR)
	{
		int ln = strlen(r.string);
		if (ln>0 && p1>0 && p2>p1 && ln>p2)
		{
			cp=newstring1(p2-p1+1);
			strncpy(cp, (r.string)+p1-1,p2-p1+1);
			*(cp+p2-p1+1)='\0';
			r.string = cp;
			return r;
		}
	}
	return emptyObj();
}

/**********************************************************
 Uses T-Rex a tiny regular expression library
 Copyright (C) 2003-2004 Alberto Demichelis
**********************************************************/

/*
i
!LET S1="testing"
!LET S2="est"
!print s1;" ";s2;" ";REX(S1,s2)
!LET S2="xst"
!print s1;" ";s2;" ";REX(S1,s2)
!LET S2="e.*n"
!print s1;" ";s2;" ";REX(S1,s2)
*/

tOBJ orex(tOBJ  a, tOBJ r)
{
	//!REX("testing","est")
	TRex *x;
	tOBJ v=emptyObj();
	const TRexChar *begin,*end;
	TRexChar sTemp[200];
	const TRexChar *error = NULL;

	if (!(a.type==STR && r.type==STR)) // two strings needed
		return emptyObj();
	
	x = trex_compile(_TREXC(r.string),&error);
	if(x) {
		trex_sprintf(sTemp,_TREXC(a.string));
		if(trex_search(x,sTemp,&begin,&end))
		{
			int i,n = trex_getsubexpcount(x);
			TRexMatch match;
			for(i = 0; i < n; i++)
			{
				trex_getsubexp(x,i,&match);

			}
			v = makeint(trex_getsubexpcount(x));
		}
		else {
			v = makeint(0);
		}
		trex_free(x);
	}
	else {
		trex_printf(_TREXC("compilation error [%s]!\n"),error?error:_TREXC("undefined"));
	}
	return v;
}

/**********************************************************/
/* SERVO routines                                        */
/**********************************************************/

extern BYTE cpos[];
extern BYTE nos;

tOBJ oserv(tOBJ a)
{
	//> !SERV
	tOBJ r;
	readservos(0);
	r=cnvtBytetoList(nos, cpos);	
	return r;
}

tOBJ opose(tOBJ a)
{
	//> !POSE {1 2 3 4}
	BYTE sp[32];
	int nb,i;
	int speed=4, tm=1000, fm=10, fmflg=0;

	if ((nb=cnvtListtoByte(a, 32, sp)))
	{
		for (i=0; i<nb; i++)
		{
			if (dbg) printf ("%d\n", sp[i]);
		}
		if (!dbg) PlayPose(tm, fm, speed, sp, (fmflg==0)?nb:0);
		fmflg=1;
	}
	return emptyObj();
}

/**********************************************************/
/*  Access sensors                                        */
/**********************************************************/

tOBJ opsd(tOBJ a)
{
	tOBJ r;
	Get_AD_PSD();
	r.type=INTGR;
	r.number=gDistance;
	return r;
}

tOBJ oacx(tOBJ a)
{
	tOBJ r;
	Acc_GetData();
	r.type=INTGR;
	r.number=x_value;
	return r;
}

tOBJ oacy(tOBJ a)
{
	tOBJ r;
	Acc_GetData();
	r.type=INTGR;
	r.number=y_value;
	return r;
}

tOBJ oacz(tOBJ a)
{
	tOBJ r;
	Acc_GetData();
	r.type=INTGR;
	r.number=z_value;
	return r;
}

tOBJ oexit(tOBJ a)
{
#ifdef LINUX
	sigcatch();
#endif
	return emptyObj();
}

extern fMatrix readmatrix(char **str );
extern tOBJ parse(char *s);

tOBJ oload(tOBJ  n)
{
	//!LOAD "fn"
	// file start with [ is a matrix
	// file start with { x } is a list
	// else string
	tOBJ r=emptyObj();

       	FILE *fp;
	char *s = n.string;
	int cn=0;
	int sz=1024;
	char *m=malloc(sz);
	int ch;
	if (n.type != STR)
	{
		printf ("? requires filename\n");
		return r;
	}

	if ((fp = fopen(s, "r")) == 0)
	{
		printf ("? can't find file - %s\n",s);
		return r;
	}

	while ( (ch=fgetc(fp))>=0)
	{
		m[cn++]=ch;
	}
	m[cn]=0;
	if (dbg) printf ("Loaded [%s] %d\n", m,cn);	
	fclose(fp);

	switch (m[0])
	{
	case '[' : 
		m++;
		r.type=FMAT2;
		r.fmat2 = readmatrix(&m);
		break;
	case '{' : 
		r = parse(m); 
		break;
	case '!' : 
		m++;
		r = eval_oxpr(m); 
		break;
	default:
		r = makestring(m);
		break;
	}
	return r;
}


tOBJ osave(tOBJ  n)
{
	//!SAVE "fn" object
	tOBJ f=ocar(n);
	tOBJ r=ocar(ocdr(n));

       	FILE *fp;
	char *s;

	if (f.type==STR) s = f.string;
	

	if ((fp = fopen(s, "w")) == 0)
	{
		printf ("? can't write to file - %s\n",s);
		return r;
	}

	if (r.type==FMAT2)
	{
		fprintf(fp,"[");
		fmatprint(fp, &r.fmat2);
		fprintf(fp,"]\n");
	}
	else
	{
		fprint(fp, r);
	}
	fclose(fp);
	return r;
}

tOBJ odict(tOBJ  lst)
{
	//!DICT {{"a" 2} {"b" 3} {"c" 4}}
	tOBJ r;
	if (lst.type==CELL)
	{
		r = makedict();
		do {
			tOBJ pair = ocar(lst);
			tOBJ n = ocar(pair);
			tOBJ v = ocar(ocdr(pair));

			if (n.type==STR)
				dict_add(r.dict, n.string, v);

			lst=ocdr(lst); 	// lst=cdr lst		
		}
		while (onull(lst).number==0); 	
		return r;		
	}	
	return emptyObj();
}




/***********************************************************************

Extended input

************************************************************************/

tOBJ omatr(tOBJ a)
{
	tOBJ n=ocar(a); a=ocdr(a);
	tOBJ v=ocar(a); a=ocdr(a);
	tOBJ file=ocar(a); a=ocdr(a);
	int val;
	// !MAT LOAD 8 "text.txt"
	// !MAT STORE 8 "test.txt"

	if (n.type==STR && n.type==INTGR)
	{
		if (!strcmp(n.string,"LOAD"))
		{
			val=toint(v);
			//v=get(env, "DFN");
			matrixload(val, file.string);
		}
		if (!strcmp(n.string,"STORE"))
		{
			val=toint(v);
			//v=get(env, "DFN");
			matrixstore(val, file.string);
		}
	}
	return (emptyObj());
}

tOBJ oimg(tOBJ v)
{
	//!IMAGE {"command" {parameters}}
	//comand : {"UNLOCK" "LOAD" "FILT" "RAW" "THRE" "COLOR" "PROC" "REG" "SHOW" 
	
	tOBJ cmd = ocar(v);
	v=ocdr(v);

	if (cmd.type == SYM)
	{
		if (!strcmp(cmd.string,"UNLOCK"))
		{
			rellock();
		}
		else
		if (!strcmp(cmd.string,"LOAD")) //  IMAG LOAD 8 "Text"
		{
			int sz;
			tOBJ a=ocar(v); v=ocdr(v);
			tOBJ file=ocar(v);
			sz=toint(a);
			if (sz<1 || sz>sqrt(SCENESZ))
			{
        			printf("error = expect int size 1-%d\n", (int)sqrt(SCENESZ)); 
				return emptyObj();
			}
			if (file.type != STR)
			{
				file = makestring("test.jpg"); //should read from env
			}

			if (file.type==STR && loadimage(file.string, sz, &scene[0])==0)
			{
				nis=sz*sz;
				return makeint(nis);
			}
			else
			{
				printf ("error\n");
				nis=0;
			}
		}
		else
		if (!strcmp(cmd.string,"FILTER"))
		{
			//!IMAGE FILTER 4 1 10 10 100 100 100
			char *file = "test.jpg";

			int sz= toint(ocar(v)); v=ocdr(v);
			int a = toint(ocar(v)); v=ocdr(v);
			int b = toint(ocar(v)); v=ocdr(v);
			int c = toint(ocar(v)); v=ocdr(v);
			int d = toint(ocar(v)); v=ocdr(v);
			int e = toint(ocar(v)); v=ocdr(v);
			int f = toint(ocar(v));

			if (filterimage(file,&scene[0],sz,a,b,c,d,e,f)==0)
				nis=sz*sz;
			else
				nis=0;
			return makeint(nis);
		}
		else
		if (!strcmp(cmd.string,"THRESH"))
		{
			//!IMAGE THRE 0
			//!IMAGE THRE CID 120 175 40 70 30 40

			int cid = toint(ocar(v));  v=ocdr(v);

			if (cid==0)
			{
        			printf("clr thresholds\n"); 
				clrmap();
			}

			int a = toint(ocar(v)); v=ocdr(v);
			int b = toint(ocar(v)); v=ocdr(v);
			int c = toint(ocar(v)); v=ocdr(v);
			int d = toint(ocar(v)); v=ocdr(v);
			int e = toint(ocar(v)); v=ocdr(v);
			int f = toint(ocar(v));

			add_thresh(cid, a,b,c,d,e,f);
		}
		else
		if (!strcmp(cmd.string,"COLOR"))  
		{
			//!IMAGE COLOR "orange" 20 30 60 2
			tOBJ r=emptyObj();
			tOBJ colour = ocar(v);  v=ocdr(v);
			int a = toint(ocar(v)); v=ocdr(v);
			int b = toint(ocar(v)); v=ocdr(v);
			int c = toint(ocar(v)); v=ocdr(v);
			int d = toint(ocar(v));

			if (colour.type==STR)
			{
				r  = makeint(add_color(colour.string, a,b,c,d));
			}
			return r;
		}
		else
		if (!strcmp(cmd.string,"RAW"))
		{
			tOBJ file= ocar(v);
			if (file.type==STR) loadJpg(file.string);
		}
		else
		if (!strcmp(cmd.string,"PROC"))
		{
			int n=toint(ocar(v));
			processFrame(n, &scene[0]);
			nis=n*n;
		}
		else
		if (!strcmp(cmd.string,"REG"))
		{
			cmd=ocar(v);
			get_color_region(toint(cmd)); 
		}
		else
		if (!strcmp(cmd.string,"SHOW"))
		{
			int sz=sqrt(nis);
			int n=toint(ocar(v));

			frame=&scene[0];

			switch (n) {
			case 0: 
			case 1: 
			case 2:
			case 3:
			case 4:
				showImage(n); break;
			case 5:
				output_grey1(sz); break;
			case 6:
				output_frame(sz); break;
			}
		}
		else
		{
			printf ("Invalid options %s\n", cmd.string);
		}
	}
	return (emptyObj());
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
		case '.' : printf("%c",*ptr); *op++=*ptr;	break;
		case ',' : *ptr = getchar(); 			break;
		case '[' : if (*ptr==0) while (*s!=']') s++; 	break;
		case ']' : while (*s!='[') s--; s--;		break;
		}
		s++;	
	}
	*op=0;
}

tOBJ obf(tOBJ v)
{
	if (v.type==STR)
		brainf(v.string);
	else
		printf("need a string argument");

	return (emptyObj());
}


tOBJ orbmrf(tOBJ v)
{
	tOBJ r=emptyObj();
	Motion m;
	if (v.type==STR) 
	{ 
		m = rbmload(v.string) ;
		rbmprint (&m);
		r.type=RBM;
		r.mot = m;
		r.q=0;
	}
	return r;
}






