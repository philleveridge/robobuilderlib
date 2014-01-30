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
#include "oexpress.h"

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
	{"&",    8,  LAND,  2, NULL},
	{"|",    8,  LOR,   2, NULL},
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
	{"ABS",   40, NA,   1, oabs},  //function single arg
	{"ACCX",  40, NA,   0, oacx},  //const
	{"ACCY",  40, NA,   0, oacy},  //const
	{"ACCZ",  40, NA,   0, oacz},  //const
	{"ACOS",  40, NA,   1, oacos}, //function single arg
	{"AND",   40, NA,   9, oand}, 
	{"APPEND",40, NA,   9, oappend}, 
	{"APPLY", 40, NA,   2, oapply}, //function two args   <fMatrix>
	{"ASSN",  40, NA,   1, oasso},  //function single arg
	{"ATAN",  40, NA,   1, oatan}, //function single arg
	{"ATOM",  40, NA,   1, oatom},  //function single arg
	{"ATOS",  40, NA,   1, oatos}, 
	{"BEGIN", 40, NA,   9, obegin},  
	{"BF",    40, NA,   1, obf},
	{"BREAK", 40, NA,   1, obreak}, 
	{"CAR",   40, NA,   1, ocar},  //LIST BASED
	{"CAAR",  40, NA,   1, ocaar},  //LIST BASED
	{"CADR",  40, NA,   1, ocadr},  //LIST BASED
	{"CADAR", 40, NA,   1, ocadar},  //LIST BASED
	{"CDR",   40, NA,   1, ocdr},  //function single arg
	{"CELL",  40, NA,   3, omat},   //function three args   <fMatrix, int, int>
	{"COL",   40, NA,   1, ocol},   //function single arg    <fMatrix>
	{"COND",  40, NA,   9, ocond}, 
	{"CONS",  40, NA,   2, ocons},  //function single arg
	{"CONV",  40, NA,   2, oconv},  //function three args   <fMatrix>
	{"COS",   40, NA,   1, ocos},  //function single arg
	{"DET",   40, NA,   1, omdet},  //function <fMatrix>
	{"DICT",  40, NA,   1, odict},// 
	{"DO",    40, NA,   9, obegin},  
	{"DSIG",  40, NA,   1, odsig}, //sigmoid functon
	{"EXIT",  40, NA,   0, oexit},
	{"EXP",   40, NA,   1, oexp},  //function single arg
	{"EYE",   40, NA,   2, oeye},   //function two args  <fint, int>
	{"FOR",   40, NA,   9, ofor}, 
	{"FOREACH",40, NA,  9, ofore}, 
	{"GETB",  40, NA,   1, getb}, //function single arg
	{"GETK",  40, NA,   1, oget}, //function single arg
	{"GETSERVO",40, NA, 1, ogetservo}, //function single arg
	{"H2SM",  40, NA,   1, ohsum2},  //function <fMatrix>
	{"HSUM",  40, NA,   1, ohsum},  //function <fMatrix>
	{"IMAGE", 40, NA,   9, oimg},
	{"IMP",   40, NA,   3, oimp},   //function two args   <string>, <int>, <int>
	{"INT",   40, NA,   1, oint},  //function <fMatrix>
	{"INV",   40, NA,   1, ominv},  //function <fMatrix>
	{"KEY",   40, NA,   1, okey}, 
	{"LAST",  40, NA,   1, olast},  //function single arg
	{"LEN",   40, NA,   1, olen},  //function single arg
	{"LFT",   40, NA,   2, olft},//
	{"LIST",  40, NA,   9, olist},   
	{"LOAD",  40, NA,   1, oload},//
	{"LOG",   40, NA,   1, olog},  //function single arg
	{"MAPCAR",40,NA,    2, omapcar}, //function two args   <fMatrix>
	{"MAT",   40, NA,   3, omatr},// 
	{"MAX",   40, NA,   2, omax},   //function two args
	{"MCOND", 40, NA,   5, omcond},  //function three args   <fMatrix>
	{"MEMBER",40, NA,   2, omemb},  //function single arg
	{"MID",   40, NA,   3, omid},//
	{"NOT",   40, NA,   1, onot},  //function single arg
	{"NTH",   40, NA,   2, onth}, //function two arg
	{"NULL",  40, NA,   1, onull},  //function single arg
	{"OR",    40, NA,   9, oor}, 
	{"PEEK",  40, NA,   2, opeek}, 
	{"PLUS",  40, NA,   9, oplus}, 
	{"POP",   40, NA,   1, opop}, //function single arg
	{"POSE",  40, NA,   1, opose}, //function single arg
	{"PR",    40, NA,   9, opr}, 
	{"PRINT", 40, NA,   9, opr},  
	{"PSD",   40, NA,   0, opsd},  //const
	{"PUSH",  40, NA,   2, opush},  //function single arg
	{"PUTC",  40, NA,   1, oputch},  //function single arg
	{"RBM",   40, NA,   1, orbmrf},
	{"REP",   40, NA,   3, orep},   //function three args  <fMatrix, int, int>
	{"RETURN",40, NA,   1, oreturn}, 
	{"REV",   40, NA,   1, orev},  //function single arg
	{"REX",   40, NA,   2, orex},//
	{"RGT",   40, NA,   2, orgt},//
	{"RND",   40, NA,   0, ornd},  //in-const
	{"ROW",   40, NA,   1, orow},   //function single arg    <fMatrix>
	{"RSHP",  40, NA,   3, orshp},  //function three args  <fMatrix, int, int>
	{"SAVE",  40, NA,   2, osave},//
	{"SCELL", 40, NA,   5, omats},   //function three args   <fMatrix, int, int>
	{"SERVO", 40, NA,   0, oserv}, //function single arg
	{"SETB",  40, NA,   2, setb}, //function single arg
	{"SETK",  40, NA,   1, oset}, //function single arg
	{"SETSERVO", 40, NA,3, osetservo}, //function single arg
	{"SIG",  40, NA,    1, osig},  //sigmoid functon
	{"SIGN", 40, NA,    1, osign},  //sigmoid functon
	{"SIN",  40, NA,    1, osin},  //function single arg
	{"SQRT", 40, NA,    1, osqrt}, //function single arg
	{"STOA", 40, NA,    1, ostoa}, 
	{"STACK",40, NA,    1, ostack}, 
	{"SUBS", 40, NA,    1, osubst},  //function single arg
	{"SUM",  40, NA,    1, osum},  //function <fMatrix>
	{"TAN",  40, NA,    1, otan},  //function single arg
	{"TRN",  40, NA,    1, otrn},   ////MATRIX tRANSPOSE 
	{"TYPE", 40, NA,    1, otype},  //function single arg
	{"V2SM", 40, NA,    1, ovsum2},  //function <fMatrix>
	{"VSUM", 40, NA,    1, ovsum},  //function <fMatrix>
	{"WAIT", 40, NA,    1, owait},  
	{"WHILE",40, NA,    9, owhile}, 
	{"WHOS", 40, NA,    1, owhs},
	{"ZERB", 40, NA,    5, ozerob},  //Zero Matrix bounded
	{"ZERD", 40, NA,    1, ozerod},  //Zero matrix diaganol
	{"ZERO", 40, NA,    2, ozero}    //Zero Matrx x,y
};

/**********************************************************/
/*  AFunction PTRs                                        */
/**********************************************************/

tOBJ makeop(int i)
{
	tOBJ r;
	r.type=FUNC;
	r.fptr=&oplist[i];
	return r;	
}

void loadop (Dict *e)
{
	int i=0;
	while (i<sizeof(oplist)/sizeof(tOP))
	{
		dict_add(e, oplist[i].name, makeop(i));
		i++;
	}	
}

tOBJ getOP2(Dict *e, char *str)
{
	return dict_getk(e, str);	
}

int getOPlevel(Dict *e, char *str)
{
	tOBJ r =  dict_getk(e, str);
	if (r.type != FUNC) return -1;
	return ((tOPp)(r.fptr))->level;	
}

int getOPtype(Dict *e, char *str)
{
	tOBJ r =  dict_getk(e, str);
	if (r.type != FUNC) return -1;
	return ((tOPp)(r.fptr))->type;		
}


/**********************************************************/
/*  Access variables (read/write)                         */
/**********************************************************/

tOBJ get(Dict *ep, char *name)
{
	if (dict_contains(ep,name))
	{
		return dict_getk(ep, name);
	}
	return emptyObj();
}

//!> GETB "@A"
//!> GETB "A"
tOBJ getb(tOBJ n)
{
	tOBJ r;
	char *name;
	if (n.type != STR) 
		return emptyObj();

	name=n.string;

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
}

//!> SETB "@A" '(2 4 5 1 6)
//!> SETB "A" 1
tOBJ setb(tOBJ n, tOBJ r)
{
	char *name;
	if (n.type != STR) 
		return emptyObj();

	name=n.string;

	if (strlen(name)==1 && isalpha(*name)) //backwards compat
	{
		if (r.type==INTGR)
			setvar(*name-'A',r.number);
		if (r.type==FLOAT)
			setvar(*name-'A',(long)r.floatpoint);
		if (r.type==EMPTY)
			setvar(*name-'A',0);
		return r;
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
		return r;
	}
	return emptyObj();
}

tOBJ owhs(tOBJ r)
{
	//display all variables and types
	//!WHOS
	if (r.type==EMPTY)
		dict_print(env.dict,1);  //exclude any type FUNC
	else
		dict_print(env.dict,toint(r));  //exclude any type FUNC
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


	if (dbg>1) {printf("math op : %d %d %d\n", o1.type, op, o2.type); } //debug info

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
		return r;
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
			if (b ==0.0) 
				r.type=EMPTY;
			else
				r.floatpoint = a / b; 
			break;
		case LMOD:
			r.floatpoint = fmod(a,b) ; break;
		case LT:
			r.type=INTGR; r.number = a < b; break;
		case EQL:
			r.type=INTGR; r.number = a == b; break;
		case NEQ:
			r.type=INTGR; r.number = a != b; break;
		case GT:
			r.type=INTGR; r.number = a > b; break;
		case LTE:
			r.type=INTGR; r.number = a<= b; break;
		case GTE:
			r.type=INTGR; r.number = a >= b; break;
		case LAND:
			r.type=INTGR; r.number = a && b; break;
		case LOR:
			r.type=INTGR; r.number = a || b; break;
		default:
			r.type=EMPTY; //error
			break;
		}
		return r;
	}

	if (op == EQL)
	{
		return makeint(compareObj(o1,o2));
	}

	if (op == NEQ)
	{
		return makeint(!compareObj(o1,o2));
	}

	if (o1.type==STR && o2.type==STR && op==PLUS)
	{
		int n=strlen(o1.string)+strlen(o2.string)+1;
		char *s=newstring1(n);
		strcat(s,o1.string);
		strcat(s,o2.string);
		r.type=STR;
		r.string=s;
		return r;
	}

	if (o1.type==FMAT2 && o2.type==FMAT2 && (op==PLUS || op==MINUS || op==DIVD || op==PROD))
	{
		r.type=FMAT2;	
		if (op==PLUS)  r.fmat2 = fadd2(o1.fmat2,o2.fmat2, '+') ;
		if (op==MINUS) r.fmat2 = fadd2(o1.fmat2,o2.fmat2, '-') ;	
		if (op==DIVD)  r.fmat2 = fadd2(o1.fmat2,o2.fmat2, '/') ;	
		if (op==PROD)  r.fmat2 = fadd2(o1.fmat2,o2.fmat2, '*') ;
		return r;		
	}

	if (o1.type==FMAT2 && (o2.type==INTGR || o2.type==FLOAT) && op==PLUS)
	{
		r.type=FMAT2;	
		r.fmat2 = fmatcp(o1.fmat2);
		for (i=0; i<o1.fmat2->w*o1.fmat2->h; i++)
			r.fmat2->fstore[i] += tofloat(o2);	
	}

	if (o1.type==FMAT2 && (o2.type==INTGR || o2.type==FLOAT) && op==POWR)
	{
		r.type=FMAT2;	
		r.fmat2 = fmatcp(o1.fmat2);
		for (i=0; i<o1.fmat2->w*o1.fmat2->h; i++)
			r.fmat2->fstore[i] = pow(r.fmat2->fstore[i],tofloat(o2));	
	}

	if (o1.type==FMAT2 && (o2.type==INTGR || o2.type==FLOAT) && op==MULT)
	{
		r.type=FMAT2;	
		r.fmat2 = fmatcp(o1.fmat2);
		for (i=0; i<o1.fmat2->w*o1.fmat2->h; i++)
			r.fmat2->fstore[i] = r.fmat2->fstore[i] * tofloat(o2);	
	}

	if (o1.type==FMAT2 && (o2.type==INTGR || o2.type==FLOAT) && op==DIVD)
	{
		r.type=FMAT2;	
		r.fmat2 = fmatcp(o1.fmat2);
		for (i=0; i<o1.fmat2->w*o1.fmat2->h; i++)
			r.fmat2->fstore[i] /= tofloat(o2);	
	}

	if (o1.type==FMAT2 && (o2.type==INTGR || o2.type==FLOAT) && op==MINUS)
	{
		r.type=FMAT2;	
		r.fmat2 = fmatcp(o1.fmat2);
		for (i=0; i<o1.fmat2->w*o1.fmat2->h; i++)
			r.fmat2->fstore[i] -= tofloat(o2);	
	}

	if (o1.type==FMAT2 && o2.type==FMAT2 && op==MULT)
	{
		r.type=FMAT2;	
		r.fmat2 = fmultiply2(o1.fmat2,o2.fmat2) ;	
		if (r.fmat2->w*r.fmat2->h == 0)
		{
			r.type=EMPTY;
		}
	}

	return r;
}

tOBJ oint(tOBJ a)
{
	tOBJ r;
	r.type=INTGR;
	r.number=toint(a);
	return r;
}

tOBJ osign(tOBJ a)
{
	tOBJ r;
	int i=toint(a);
	r.type=INTGR;
	if (i==0) r.number=0;
	else r.number=(i>0)?1:-1;
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

tOBJ onot(tOBJ a)
{
	tOBJ r;
	r.type=INTGR;
	r.number=(toint(a)!=0)?0:1;
	return r;
}

tOBJ oputch(tOBJ a)
{
	putchar(toint(a));
	return emptyObj();
}

tOBJ oabs(tOBJ a)
{
	tOBJ r;
	int i;
	r.type=EMPTY;

	if (a.type==FMAT2)
	{
		r.type = FMAT2;	
		r.fmat2 = fmatcp(a.fmat2);
		for (i=0; i<a.fmat2->w*a.fmat2->h; i++)
		{
			r.fmat2->fstore[i] = fabs(r.fmat2->fstore[i]);
		}
	}
	else
	if (a.type==INTGR)
	{
		r.type=INTGR;
		r.number=abs(toint(a));
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
		r.floatpoint=a.fmat2->fstore[0];
		for (i=1; i<a.fmat2->h*a.fmat2->w; i++)
			if (a.fmat2->fstore[i]>r.floatpoint) r.floatpoint= a.fmat2->fstore[i];
	}

	return r;
}

/**********************************************************/
/*  matrix function (loating point)                       */
/**********************************************************/
/*
!SET MA [1.0 2.0;3.0 4.0]
!print cell ma 1 1
!PRINT DSIG MA
!MAPCAR 'expr ma
*/
tOBJ omat(tOBJ m, tOBJ a, tOBJ b) 
{
	//read matrix cell
	tOBJ r;
	int row,col;
	r=makefloat(0.0);
	row=toint(a);
	col=toint(b);

	if (m.type==FMAT2) 
	{
		r.floatpoint = fget2((m.fmat2),col,row);
	}
	return r;
}

tOBJ omats(tOBJ m, tOBJ a, tOBJ b, tOBJ v, tOBJ dummy) 
{
	//store in mat cell
	float cv=tofloat(v);
	int row,col;
	row=toint(a);
	col=toint(b);

	if (m.type==FMAT2) 
	{
		fset2((m.fmat2),col,row, cv);
	}
	return makefloat(cv);
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


tOBJ ominv(tOBJ a)
{
	//invert a matrix
	tOBJ r;
	if (a.type==FMAT2) 
	{
		r.type=FMAT2;
		r.fmat2=inverse(a.fmat2);
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
		r.floatpoint=detrminant(a.fmat2, a.fmat2->w);
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
		r.fmat2=fmatcp(a.fmat2);
		for (i=0; i<a.fmat2->h*a.fmat2->w; i++)
		{
			float f= a.fmat2->fstore[i];
			r.fmat2->fstore[i] =1/(1+exp(-f));
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
		r.fmat2=fmatsum2(a.fmat2, 1);	
	}
	return r;
}

tOBJ osum(tOBJ a)
{
	tOBJ r=emptyObj();

	if (a.type==FMAT2)
	{ 
		r.type=FLOAT;
		r.floatpoint = fsum(a.fmat2);
	}
	return r;
}

tOBJ orow(tOBJ a)
{
	tOBJ r=emptyObj();

	if (a.type==FMAT2)
	{ 
		r.type=INTGR;
		r.number = a.fmat2->h;
	}
	return r;
}

tOBJ ocol(tOBJ a)
{
	tOBJ r=emptyObj();

	if (a.type==FMAT2)
	{ 
		r.type=INTGR;
		r.number = a.fmat2->w;
	}
	return r;
}

tOBJ ovsum(tOBJ a)
{
	tOBJ r=emptyObj();

	if (a.type==FMAT2)
	{
		r.type=FMAT2;
		r.fmat2=fmatsum2(a.fmat2, 2);	
	}
	return r;
}

tOBJ ohsum2(tOBJ a)
{
	tOBJ r=emptyObj();

	if (a.type==FMAT2)
	{
		r.type=FMAT2;
		r.fmat2=fmatsum2(a.fmat2, 3);	
	}
	return r;
}

tOBJ ovsum2(tOBJ a)
{
	tOBJ r=emptyObj();

	if (a.type==FMAT2)
	{
		r.type=FMAT2;
		r.fmat2=fmatsum2(a.fmat2, 4);	
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
		r.fmat2=fmatcp(a.fmat2);
		for (i=0; i<a.fmat2->h*a.fmat2->w; i++)
		{
			float f= a.fmat2->fstore[i];
			r.fmat2->fstore[i] = f*(1-f);
		}			
	}
	return r;
}

tOBJ oapply(tOBJ a, tOBJ b)
{
	// APPLY 'PLUS '{2 3 4} -> 9

	tOBJ r=emptyObj();
	if (dbg) printf ("APPLY -\n");

	//a=eval(a,env.dict);
	if (b.type==CELL)
	{
		r  = procall (a, b, env.dict);		
	}
	return r;
}
/*
!> MAPCAR 'FACT '{1 2 3 4}
 = {1 2 6 24}
!> MAPCAR {LAMBDA {X} {+ X 1}} '{1 2 3 4}
 = {2 3 4 5}
*/
// MAPCAR SIN '(0.100000 0.200000 0.300000)
// FUNC   FOO  (X) (+ 1.0 X)
// MAPCAR FOO '(0.100000 0.200000 0.300000)

tOBJ omapcar(tOBJ a, tOBJ b)
{
	tOBJ r=emptyObj(), v;
	if (dbg) printf ("Mapcar -\n");

	a=eval(a,env.dict);

	if (a.type==FUNC && ((tOPp)(a.fptr))->nop>1)
	{
		printf ("?too many params\n"); return emptyObj();
	}
	else
	if (a.type!=LAMBDA && a.type!=FUNC)
	{
		printf ("?err\n"); return emptyObj();
	}

	if (b.type==FMAT2)
	{

		// MAPCAR 'FOO [ 1 2 ; 3 4 ]
		int i=0, j=0;
		// create new matrix
		r.type=FMAT2;
		r.fmat2=fmatcp(b.fmat2);
	
		for (i=0; i<b.fmat2->w; i++)
		{
			for (j=0;j<b.fmat2->h; j++)
			{
				v.type=FLOAT;
				v.floatpoint =fget2(b.fmat2,i,j);
				if (a.type==FUNC) v=append(emptyObj(),v);
				tOBJ n  = procall (a, v, env.dict);	
				fset2(r.fmat2,i,j,tofloat(n));			
			}
		}
		freeobj(&env);
	}
	else if (b.type==CELL)
	{
		// MAPCAR FOO '(2 3 4)
		while (b.type != EMPTY)
		{
			tOBJ arg= ocar(b);
			arg.q=1;
			if (a.type==FUNC) arg=append(emptyObj(),arg);
			tOBJ n  = procall (a, arg, env.dict);		
			b=ocdr(b);
			r=append(r,n);
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
		r.fmat2 = ftranspose2(a.fmat2); 
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
		r.fmat2 = fmatrshp(m.fmat2, col,row);
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
		r.fmat2 = freplicate2(m.fmat2, col,row);
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
			r.fmat2->fstore[i+i*col] = 1.0;
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
	r.fmat2=fconvolve2(a.fmat2,b.fmat2) ;		
	return r;
}

tOBJ omcond(tOBJ a, tOBJ b, tOBJ c, tOBJ d, tOBJ e)
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
		r.fmat2 = fmatcp(a.fmat2); // clone
		for (i=0; i<a.fmat2->w*a.fmat2->h; i++)
		{
			if (r.fmat2->fstore[i]>= lv && r.fmat2->fstore[i]<= uv)
				r.fmat2->fstore[i] = nv1;
			else
				r.fmat2->fstore[i] = nv2;
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
		r.fmat2 =fmatzeroregion(a.fmat2, c1, r1, c2, r2)   ;
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
		r.fmat2 = fmatzerodiag2(a.fmat2) ;
	}
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

tOBJ ocadr (tOBJ a)
{
	return ocar(ocdr(a));
}

tOBJ ocadar (tOBJ a)
{
	return ocar(ocdr(ocar(a)));
}

tOBJ ocaar (tOBJ a)
{
	return ocar(ocar(a));
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

	if (a.type==STACK)
	{
		r=makeint(((tStackp)(a.stk))->noe);
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
	if (b.type==EMPTY)
	{
		r=makeCell();
		((tCELLp)(r.cell))->head=a;
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
	//SUBS '(2 1 (1 2 3))
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
	return makeint(!(a.type==CELL || a.type==FMAT2 || a.type==EMPTY ||  a.type==RBM || a.type==STACK));
} 

tOBJ omemb(tOBJ key, tOBJ lst)
{
	//!MEMB '2 '{2 3}-> 1
	//!MEMB '4 '{2 3} -> 0
	if (lst.type==CELL)
	{

		if (lst.type != CELL)
		{
			printf ("Not a list\n");
			return makeint(0);
		}

		do
		{
			tOBJ mem = ocar(lst);
			if (compareObj(mem, key))
			{
				return lst;
			}
			lst=ocdr(lst); 	// lst=cdr lst				
		}
		while (onull(lst).number==0); 	
	}
	return emptyObj();
}


tOBJ oasso(tOBJ a)
{
	//!ASSN '{1 {{1 2} {2 3} {3 4}}} -> {1 2}
	//!ASSN '{2 {{1 2} {2 3} {3 4}}} -> {2 3}
	//!ASSN '{7 {{1 2} {2 3} {3 4}}} -> NIL

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

/***********************************************************************


***********************************************************************/

tOBJ olist(tOBJ o, Dict *e)
{
	tOBJ r=emptyObj();
	while (o.type != EMPTY)
	{
		tOBJ exp = ocar(o); o=ocdr(o);	
		r = append(r, eval(exp, e));
	}
	return r;
}

tOBJ oplus(tOBJ o, Dict *e)
{
	tOBJ r = eval(ocar(o),e); 
	o=ocdr(o);	

	while (o.type != EMPTY)
	{
		tOBJ exp = ocar(o); o=ocdr(o);	
		r = omath(r, eval(exp, e), PLUS);
	}
	return r;
}

tOBJ oor(tOBJ o, Dict *e)
{
	tOBJ r = eval(ocar(o),e); 
	o=ocdr(o);
	if (r.type==INTGR && r.number>0) return makeint(1);	
	r=makeint(0);
	while (o.type != EMPTY)
	{
		tOBJ exp = ocar(o); o=ocdr(o);	
		if (toint(omath(r, eval(exp, e), LOR))==1) return makeint(1);
		r=makeint(0);
	}
	return makeint(0);
}

tOBJ oand(tOBJ o, Dict *e)
{
	tOBJ r = eval(ocar(o),e); 
	o=ocdr(o);	
	if (r.type == EMPTY || (r.type==INTGR && r.number==0)) return makeint(0);
	while (o.type != EMPTY)
	{
		tOBJ exp = ocar(o); o=ocdr(o);	
		r = omath(r, eval(exp, e), LAND);
		if (r.type == EMPTY || (r.type==INTGR && r.number==0)) return makeint(0);
	}
	return makeint(1);
}

tOBJ oappend(tOBJ o, Dict *e)
{
	tOBJ r=emptyObj();
	while (o.type != EMPTY)
	{
		tOBJ exp = eval(ocar(o),e); o=ocdr(o);	
		while (exp.type != EMPTY)
		{
			r = append(r, ocar(exp));
			exp=ocdr(exp);
		}
	}
	return r;
}

tOBJ opr(tOBJ a, Dict *e)
{
	//> !PR 1
	//1
	tOBJ t=emptyObj();
	int f=1;
	while (onull(a).number==0)
	{
		tOBJ v=ocar(a);
		if (v.type==SYM && !strcmp(v.string,";"))
		{
			printf(" ");
			f=0;
		}
		else
		{
			t=eval(v,e);
			print(t);
			f=1;
		}
		a=ocdr(a);
	}
	if (f) printf ("\n");
	return t;
}

/*
!> COND ('A 'B 'C)
 = B
!> COND (1 'B 'C)
 = B
!> COND (0 'B 'C)
 = C
!> COND (0 'A 0 'B 1 'E)
 = E
!> COND (0 'A 0 'B 1 )  
 = 1
!> COND ( '() 1 2)
 = 2
!> COND ( '(1) 1 2)
*/
tOBJ ocond(tOBJ o, Dict *e)
{
	tOBJ cond=emptyObj();
	tOBJ act=emptyObj();
	o=ocar(o);
	while (o.type != EMPTY)
	{
		cond = eval(ocar(o),e); o=ocdr(o);
		act  = ocar(o); o=ocdr(o);
		if (dbg) {println ("cond =", cond);}
		if ((cond.type == INTGR && cond.number != 0 ) 
		|| (cond.type != INTGR && cond.type!=EMPTY && cond.type!=FLOAT)
		|| (cond.type == FLOAT && cond.floatpoint != 0.0 ) ) 
			return (onull(act).number==1)?cond:eval(act,e);
	}	
	return cond;
}



tOBJ oset(tOBJ a)
{
/*
SETK '{A 2 B 3}
GETK 'A
> PRINT A,B
SET ENZ {DICT '{{"a" 1} {"b" 2.3}}}
set env {DICT '{{AZ 1.0} {BZ 2.0}}}
GETK '{ENV 'AZ}
SETK '{ENV 'BZ 2.9}
*/
	tOBJ r=emptyObj();
	tOBJ en=emptyObj();
	tOBJ value=emptyObj();

	if (a.type==CELL)
	{
		while(1) {
			tOBJ name = eval(ocar(a),env.dict);
			if (name.type ==DICT)
			{
				en   = name;
				name = ocar(ocdr(a));
				a    = ocdr(a);
			}

			if (name.type==EMPTY) break;
			if (name.type==SYM)
			{
				//value = eval(ocar(ocdr(a)), en.dict);
				value = ocar(ocdr(a));
				if (en.type==EMPTY)
				{
					set(en.dict, name.string, value);
				}
				else
				{
					dict_update(en.dict, name.string, value);
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
	//> !GETK '{A B}
	//> !GETK 'A
	//SET ENV {DICT '{{AZ 1.0} {BZ 2.0}}}
	//GETK '{ENV 'AZ}
	tOBJ r=emptyObj();
	if (a.type==CELL)
	{
		tOBJ name = eval(ocar(a),env.dict);

		if (name.type==SYM )
		{
			r = get(env.dict, name.string);
		}

		if (name.type==DICT)
		{
			tOBJ en = name;
			name = eval(ocar(ocdr(a)),en.dict);

			if (name.type==STR || name.type==SYM )
			{
				r = get(en.dict, name.string);
			}
		}	
	}
	if (a.type==SYM)
	{
		r = get(env.dict, a.string);
	}	
	return r;
}


tOBJ onth(tOBJ a, tOBJ lst)
{
	//!nth 0 '{"one" "two" "three"}
	//!nth 5 '{"one" "two" "three"}
	int cnt = toint(a);

	if (lst.type == CELL)
	{
		while (cnt>0)	
		{
			lst = ocdr(lst);
			cnt--;
		}
		return ocar(lst);
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
	if (r.type==STR || r.type==SYM)
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
	if (r.type==STR|| r.type==SYM)
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
	//!MID "testing" 2 4  -> "est"
	//!MID "tes" 2        -> "es"
	int p1=toint(a);
	int p2=toint(b);
	char *cp;
	if (r.type==STR|| r.type==SYM)
	{
		int ln = strlen(r.string);
		if (p2==0) p2=ln;
		if (ln>0 && p1>0 && p2>=p1 && p2<=ln)
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

tOBJ ostoa(tOBJ  r)
{
	if (r.type==STR) r.type=SYM;
	return r;
}

tOBJ oatos(tOBJ  r)
{
	if (r.type==SYM) r.type=STR;
	return r;
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
extern int wckReadPos(int id, int d1);
extern int wckPassive(int id);
extern int wckMovePos(int id, int pos, int torq);
extern int response[32];

tOBJ oserv(tOBJ a)
{
	//> !SERV
	tOBJ r;
	int n=readservos(0);
	r=cnvtBytetoList(n, cpos);	
	return r;
}

tOBJ ogetservo(tOBJ a, tOBJ b)
{
	//> !GETSERVO 10
	int id=toint(a);
	int d1=toint(b);
	wckReadPos(id, d1);
	return makeint(response[1]);
}

tOBJ osetservo(tOBJ s, tOBJ p, tOBJ v)
{
	//> !SETSERVO 12 120 4
	int id =toint(s);
	int pos=toint(p);
	int tor=toint(v);
	if (id<0 || id>31 || pos <0 || pos>254 || tor<0 || tor>4) return emptyObj();
	wckMovePos(id,pos,tor);
	return makeint(response[1]);
}

tOBJ opose(tOBJ a)
{
	//> !POSE '{25 1 {1 2 3 4}}
	//   POSE {RBM "rbm"}

	BYTE sp[32];
	int nb=0,i;
	int speed=4, tm=1000, fm=10, fmflg=0;

	if (a.type==RBM)
	{
		//put play code here
		int i,j;
		Motion *m;
		m=a.mot;
		printf ("RB Motion %s (%d,%d)\n",m->name, m->no_servos,m->no_scenes);
		for (i=0; i<m->no_servos; i++)
		{
			sp[i]=(BYTE)(m->Postn[i]);
			if (dbg); printf ("%d,", sp[i]);			
		}
		if (dbg); printf ("\n");

		if (!dbg) PlayPose(tm, fm, speed, sp, (fmflg==0)?nb:0);

		for (j=0; j<m->no_scenes; j++)
		{
			Scene p=m->sc[j];

			tm = p.TransitionTime;
			fm = p.Frames;

			printf ("Scene %d %d %d\n",j, tm, fm);

			for (i=0; i<m->no_servos; i++)
			{
				sp[i]=(BYTE)(m->sc[j].F.Position[i]);

				if (dbg) printf ("%d,", sp[i]);			
			}
			if (!dbg) PlayPose(tm, fm, speed, sp, (fmflg==0)?nb:0);
			if (dbg) printf ("\n");
		}
	}
	else
	if (a.type==CELL)
	{
		tm = toint(ocar(a)); a=ocdr(a);
		fm = toint(ocar(a)); a=ocdr(a);

		nb=cnvtListtoByte(ocar(a), 32, sp);

		printf ("%d %d %d\n", tm,fm,nb);

		for (i=0; i<nb; i++)
		{
			if (dbg); printf ("%d,", sp[i]);
		}
		if (!dbg) PlayPose(tm, fm, speed, sp, (fmflg==0)?nb:0);
		fmflg=1;
		if (dbg) printf ("\n");
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

extern long fn_kir(long v);
extern long fn_kbd(long v);
extern long fn_input(long v);


tOBJ owait(tOBJ a)
{
	delay_ms(toint(a));
	return emptyObj();
}

tOBJ okey(tOBJ a)
{
	tOBJ r;
	r.type=INTGR;
	switch(toint(a)) 
	{
	case 1:
		{
		r.number=(int)fn_kbd(0);
		break;
		}
	case 2:
		{
		r.number=(int)fn_input(0) ;
		break;
		}
	default:
		{
		r.number=(int)fn_kir(0);
		break;
		}	
	}
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


tOBJ oload(tOBJ  n)
{
	//!LOAD "fn"
	// file start with [ is a matrix
	// file start with { x } is a list
	// else string
	tOBJ r=emptyObj();

       	FILE *fp;
	char *s = n.string;
	int cn=0,lc=0,cf;
	//int sz=1024;
	//char *m=malloc(sz);
	char m[32000];
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
		if (ch==10 || ch==13) {cf=0; lc=0; continue;   }
		if (lc==0 && ch==';') {cf=1; continue;}
		if (cf==1) continue;
		m[cn++]=ch;
		lc++;
	}
	m[cn]=0;
	if (dbg>2) printf ("Loaded [%s] %d\n", m,cn);	
	fclose(fp);

	switch (m[0])
	{
	case '[' : 
		//m++;
		r.type=FMAT2;
		r.fmat2 = readmatrix(&m[1]);
		break;
	case '{' : 
		r = parse(m); 
		break;
	case '!' : 
		//m++;
		r = eval_oxpr(&m[1]); 
		break;
	default:
		r = makestring(m);
		break;
	}
	return r;
}


tOBJ osave(tOBJ  f, tOBJ r)
{
	//!SAVE "fn" object
       	FILE *fp;
	char *s;

	if (f.type==STR) 
		s = f.string;
	else
		return emptyObj();
	

	if ((fp = fopen(s, "w")) == 0)
	{
		printf ("? can't write to file - %s\n",s);
		return r;
	}

	if (r.type==FMAT2)
	{
		fprintf(fp,"[");
		fmatprint(fp, r.fmat2);
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
	//!DICT '{{"a" 2} {"b" 3} {"c" 4}}
	tOBJ r;
	if (lst.type==CELL)
	{
		int n=toint(olen(lst));
		r = makedict(n);
		do {
			tOBJ pair = ocar(lst);
			tOBJ n = ocar(pair);
			tOBJ v = ocar(ocdr(pair));

			if (n.type==STR ||n.type==SYM  )
				dict_add(r.dict, n.string, v);

			lst=ocdr(lst); 
		}
		while (onull(lst).number==0); 	
		return r;		
	}	
	return emptyObj();
}

/***********************************************************************

LOOP functions

************************************************************************/

int retflg=0;
int brkflg=0;

tOBJ obegin(tOBJ o, Dict *e)
{
	tOBJ r=emptyObj();
	retflg=0;
	while (!(retflg || brkflg) && o.type != EMPTY)
	{
		tOBJ exp = ocar(o); o=ocdr(o);	
		r=eval(exp,e);
	}
	return r;
}

tOBJ owhile  (tOBJ  o, Dict *e)
{
	//SETQ N 5
	//WHILE {> N 0} {DO {PRINT N} {SET N {- N 1}}}
	//WHILE {> N 0} {PRINT N} {DEC }

	tOBJ r=emptyObj();

	tOBJ cond =ocar(o);
	o=ocdr(o);
	brkflg=0;
	while (	!(retflg || brkflg) && toint(eval(cond,e))!=0)
	{
		//loop
		r = obegin(o,e);
	}
	brkflg=0;
	return r;
}

tOBJ obreak  (tOBJ  o)
{
	brkflg=1;
	return o;
}

tOBJ oreturn  (tOBJ  o)
{
	retflg=1;
	return o;
}

tOBJ ofore   (tOBJ  o, Dict *e)
{
	//!FOREACH X '{1 2 3} {PR X}
	//!FOREACH X [1 2 3]  {PR X}
	tOBJ r=emptyObj();

	tOBJ ind =ocar(o);
	o=ocdr(o);

	if (ind.type != SYM) return emptyObj();

	tOBJ list = eval(ocar(o),e);
	o=ocdr(o);

	if (list.type == FMAT2)
	{		
		for (int i=0; i<list.fmat2->w; i++)
		{
			for (int j=0;j<list.fmat2->h; j++)
			{
				tOBJ v;
				v.type=FLOAT;
				v.floatpoint =fget2(list.fmat2,i,j);
				dict_update(e, ind.string, v);
				r = obegin(o,e);		
			}
		}
	}
	else
	if (list.type == CELL)
	{
		brkflg=0;			
		do {
			tOBJ value=ocar(list);
			list=ocdr(list);

			dict_update(e, ind.string, value);
		
			//loop
			r = obegin(o,e);

		} while (!(retflg || brkflg) && list.type != EMPTY);
		brkflg=0;
	}
	else
	if (list.type == STACK)
	{
		brkflg=0;		
		for (int i=0; (!(retflg || brkflg) &&  i<((tStackp)(list.stk))->noe); i++)
		{
			dict_update(e, ind.string, peek(list.stk,i));
			r = obegin(o,e);		
		}
		brkflg=0;
	}
	return r;
}


tOBJ ofor (tOBJ  o, Dict *e)
{
	//!FOR X '{3 5} {PR X}
	int s,f;
	tOBJ r=emptyObj();

	tOBJ ind =ocar(o);
	o=ocdr(o);

	if (ind.type != SYM) return emptyObj();

	tOBJ range =ocar(o);

	s=toint(eval(ocar(range),e));
	f=toint(eval(ocar(ocdr(range)),e));

	o=ocdr(o);
	brkflg=0;

	if (f>=s) {
		for (int i=s; (!(retflg || brkflg) && i<=f); i++)
		{
			dict_update(e, ind.string, makeint(i));
			r = obegin(o,e);
			i = toint(dict_getk(e, ind.string));
		}
	}
	else {
		for (int i=s; (!(retflg || brkflg) && i>=f); i--)
		{
			dict_update(e, ind.string, makeint(i));
			r = obegin(o,e);
			i = toint(dict_getk(e, ind.string));
		}
	}
	brkflg=0;
	return r;
}



/***********************************************************************

IMAGE functions

************************************************************************/

extern int gkp;
extern int imready;
extern int pbrunning;

tOBJ oimg(tOBJ v, Dict *e)
{
	//!IMAGE "command" {parameters}
	//comand : "UNLOCK" "LOAD" "FILT" "RAW" "THRE" "COLOR" "PROC" "REG" "SHOW" 
	
	tOBJ cmd = ocar(v);
	v=ocdr(v);

	if (cmd.type == SYM)
	{
		if (!strcmp(cmd.string,"UNLOCK"))
		{
			rellock();
		}
		else
		if (!strcmp(cmd.string,"LOAD") || !strcmp(cmd.string,"NORM")) //  IMAG LOAD 8 "Text"
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
				if (!strcmp(cmd.string,"NORM"))
				{
					tOBJ r;
					fMatrix *m = newmatrix(sz,sz);
					r.type=FMAT2;
					r.fmat2=m;
					float s=0.0;
					for (int i=0; i<sz*sz; i++) {m->fstore[i]=(float)scene[i]; if (s< m->fstore[i]) s=m->fstore[i] ;}
					for (int i=0; i<sz*sz; i++) m->fstore[i]/=s;
					return r;	
				}
				else
				{
					nis=sz*sz;
					return makeint(nis);
				}
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
			return makeint(nis);
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
			return makeint(nis);
		}
		else
		if (!strcmp(cmd.string,"WAIT"))
		{
			int key;
			while (imready==0 && (key=uartGetByte())<0 ) 
				; // wait for signal
			imready=0;
			if (key>=0) gkp=key;
		}
		else
		{
			printf ("Invalid options %s\n", cmd.string);
		}
	}
	return (emptyObj());
}

/***********************************************************************

Stack

***********************************************************************/

tOBJ ostack(tOBJ a)
{
	if (a.type!=INTGR) return emptyObj();
	return makestack(toint(a));
}

tOBJ opush(tOBJ a, tOBJ b)
{
	if (a.type!=STACK) return emptyObj();
	if (push((tStackp)a.stk, b))
		return b;
	else
		return emptyObj();
}

tOBJ opop(tOBJ a)
{
	if (a.type!=STACK) return emptyObj();
	return pop((tStackp)a.stk);
}

tOBJ opeek(tOBJ a, tOBJ b)
{
	if (a.type!=STACK) return emptyObj();
	return peek((tStackp)a.stk, toint(b));
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



/***********************************************************************

Import export functions

************************************************************************/

tOBJ orbmrf(tOBJ v)
{
	tOBJ r=emptyObj();
	Motion *m;
	if (v.type==STR) 
	{ 
		m = rbmload(v.string) ;
		rbmprint (m);
		r.type=RBM;
		r.mot = m;
		r.q=0;
	}
	return r;
}

tOBJ omatr(tOBJ n, tOBJ v, tOBJ file)
{
	// !MAT LOAD 8 "text.txt"
	// !MAT STORE 8 "test.txt"
	int val;

	if (n.type==STR && v.type==INTGR && file.type==INTGR)
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







