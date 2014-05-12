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
#include "oimg.h"
#include "fmatrix.h"
#include "rbmread.h"
#include "oexpress.h"

#include "ofunction.h"
#include "opflow.h"
#include "cmap.h"
#include "mem.h"

#include "trex.h"

#ifdef _UNICODE
#define trex_sprintf swprintf
#else
#define trex_sprintf sprintf
#endif

#define isalpha(c)  ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')

tOP oplist[] = { 
/* 0 */	{"+",     10, PLUS, 2, {NULL}},
	{"-",     10, MINUS,2, {NULL}},
	{"/",     20, DIVD, 2, {NULL}},
	{"*",     20, MULT, 2, {NULL}},
/* 5 */	{".*",    20, PROD, 2, {NULL}},
	{".^",    20, POWR, 2, {NULL}},
	{"&",     8,  LAND, 2, {NULL}},
	{"|",     8,  LOR,  2, {NULL}},
	{"MOD",   8,  LMOD, 2, {NULL}},
/*10 */	{"<",     5,  LT,   2, {NULL}},
	{">",     5,  GT,   2, {NULL}},
	{"=",     5,  EQL,  2, {NULL}},
	{"<>",    5,  NEQ,  2, {NULL}},
	{"<=",    5,  LTE,  2, {NULL}},
	{">=",    5,  GTE,  2, {NULL}},
	{"(",     50, OBR,  1, {NULL}},
	{")",     50, CBR,  1, {NULL}},
	{",",     50, COMMA,1, {NULL}},
	{"ABS",   40, NA,   1, {oabs}},  		//function single arg
	{"ACCX",  40, NA,   0, {oacx}},  		//const
	{"ACCY",  40, NA,   0, {oacy}},  		//const
	{"ACCZ",  40, NA,   0, {oacz}},  		//const
	{"ACOS",  40, NA,   1, {oacos}}, 		//function single arg
	{"AND",   40, NA,   9, {.funce=oand}}, 
	{"APPEND",40, NA,   9, {.funce=oappend}}, 
	{"APPLY", 40, NA,   2, {.func2=oapply}}, 	//function two args   <fMatrix>
	{"ASSN",  40, NA,   1, {oasso}},  		//function single arg
	{"ATAN",  40, NA,   1, {oatan}},  		//function single arg
	{"ATOM",  40, NA,   1, {oatom}},  		//function single arg
	{"BEGIN", 40, NA,   9, {.funce=obegin}},  
	{"BF",    40, NA,   1, {obf}},
	{"BREAK", 40, NA,   1, {obreak}}, 
	{"CAR",   40, NA,   1, {ocar}},  		//LIST BASED
	{"CAAR",  40, NA,   1, {ocaar}},  		//LIST BASED
	{"CADR",  40, NA,   1, {ocadr}},  		//LIST BASED
	{"CADAR", 40, NA,   1, {ocadar}},  		//LIST BASED
	{"CDR",   40, NA,   1, {ocdr}},  		//function single arg
	{"CELL",  40, NA,   3, {.func3=omat}},   	//function three args   <fMatrix, int, int>
	{"CLEAR", 40, NA,   9, {.funce=oclear}},   	//function three args   <fMatrix, int, int>
	{"COL",   40, NA,   1, {ocol}},   		//function single arg    <fMatrix>
	{"COND",  40, NA,   9, {.funce=ocond}}, 
	{"CONS",  40, NA,   2, {.func2=ocons}},  	//function single arg
	{"CONV",  40, NA,   2, {.func2=oconv}},  	//function three args   <fMatrix>
	{"COS",   40, NA,   1, {ocos}},   		//function single arg
	{"DEC",   40, NA,   9, {.funce=odec}}, 
	{"DEF"   ,40, NA,   9, {.funce=ofunc}}, 
	{"DET",   40, NA,   1, {omdet}},  		//function <fMatrix>
	{"DICT",  40, NA,   1, {odict}},  		// 
	{"DO",    40, NA,   9, {.funce=obegin}},  
	{"DSIG",  40, NA,   1, {odsig}},  		//sigmoid functon
	{"EVAL",  40, NA,   9, {.funce=oeval}}, 
	{"EXIT",  40, NA,   0, {oexit}},
	{"EXP",   40, NA,   1, {oexp}},   		//exponent
	{"EYE",   40, NA,   2, {.func2=oeye}},   	//create I matrix
	{"FOR",   40, NA,   9, {.funce=ofor}}, 
	{"FOREACH",40, NA,  9, {.funce=ofore}}, 
	{"FORM"  ,40, NA,   9, {.funce=oform}}, 
	{"FUNC"  ,40, NA,   9, {.funce=ofunc}},   	//def function
	{"GAUS",  40, NA,   3, {.func3=ogaus}},   	//mean, variance, x value
	{"GAUSK", 40, NA,   2, {.func2=ogausk}},  	//mean, variance
	{"GETB",  40, NA,   1, {getb}},    		//get basic variable
	{"GETK",  40, NA,   9, {.funce=ogetk}},   	//get diktionary variable
	{"GETSERVO",40, NA, 2, {.func2=ogetservo}}, 	//function 
	{"H2SM",  40, NA,   1, {ohsum2}},  		//Horizontal squared matrix sum
	{"HSUM",  40, NA,   1, {ohsum}},   		//Horizontal matrix sum
	{"IF"    ,40, NA,   9, {.funce=oif}}, 
	{"IMAGE", 40, NA,   9, {.funce=oimg}},    	//image functions
	{"INC",   40, NA,   9, {.funce=oinc}},    	//increment variable
	{"IMP",   40, NA,   3, {.func3=oimp}},    	//function two args   <string>, <int>, <int>
	{"INT",   40, NA,   1, {oint}},    		//convert to int
	{"INV",   40, NA,   1, {ominv}},   		//function <fMatrix>
	{"KEY",   40, NA,   1, {okey}},  
	{"LAMBDA",40, NA,   9, {.funce=olambda}}, 
	{"LAST",  40, NA,   1, {olast}},  		//function single arg
	{"LEN",   40, NA,   1, {olen}},   		//function single arg
	{"LET"   ,40, NA,   9, {.funce=olet}},   	//set variable to formula
	{"LFT",   40, NA,   2, {.func2=olft}},   	//string left
	{"LIST",  40, NA,   9, {.funce=olist}},   
	{"LOAD",  40, NA,   1, {oload}},  		//
	{"LOG",   40, NA,   1, {olog}},   		//function single arg
	{"MAPCAR",40,NA,    2, {.func2=omapcar}},	//function two args   <fMatrix>
	{"MAT",   40, NA,   3, {.func3=omatr}},  	// 
	{"MAX",   40, NA,   2, {.func2=omax}},   	//function two args
	{"MCOND", 40, NA,   5, {.func5=omcond}}, 		//function three args   <fMatrix>
	{"MEMBER",40, NA,   2, {.func2=omemb}},  	//function single arg
	{"MID",   40, NA,   3, {.func3=omid}},   		//
	{"MIN",   40, NA,   2, {.func2=omin}},   	//function two args
	{"NOT",   40, NA,   1, {onot}},   		//function single arg
	{"NTH",   40, NA,   2, {.func2=onth}},   	//function two arg
	{"NULL",  40, NA,   1, {onull}},  		//function single arg
	{"OPFLOW",40, NA,   2, {.func2=OpticFlow}}, 
	{"OR",    40, NA,   9, {.funce=oor}}, 
	{"PEEK",  40, NA,   9, {.funce=opeek}}, 	//peek element on stack
	{"PIPE",  40, NA,   1, {opipe}}, 		//create a pipe (fifo stack)
	{"PLUS",  40, NA,   9, {.funce=oplus}}, 
	{"POP",   40, NA,   9, {.funce=opop}}, 	 		//function single arg
	{"POSE",  40, NA,   1, {opose}}, 		//function single arg
	{"PR",    40, NA,   9, {.funce=opr}}, 
	{"PRINT", 40, NA,   9, {.funce=opr}},  
	{"PSD",   40, NA,   0, {opsd}},   		//const (distance sensor)
	{"PUSH",  40, NA,   9, {.funce=opush}},  	//function single arg
	{"PUTC",  40, NA,   1, {oputch}}, 		//putchar
	{"QT",    40, NA,   9, {.funce=oquote}}, 	//
	{"RBM",   40, NA,   1, {orbmrf}}, 		//read motion file
	{"READ"  ,40, NA,   9, {.funce=oread}},  	//read terminal input
	{"REP",   40, NA,   3, {.func3=orep}},   	//function three args  <fMatrix, int, int>
	{"RETURN",40, NA,   1, {oreturn}},		//return from function
	{"REV",   40, NA,   1, {orev}},   		//reverse a list
	{"REX",   40, NA,   2, {.func2=orex}},   	//regular expression match
	{"RGT",   40, NA,   2, {.func2=orgt}},   	//right string
	{"RND",   40, NA,   0, {ornd}},   		//random number
	{"ROW",   40, NA,   1, {orow}},   		//number of rows
	{"RSHP",  40, NA,   3, {.func3=orshp}},  	//reshape matrix or image
	{"SAVE",  40, NA,   2, {.func2=osave}},  	//
	{"SERVO", 40, NA,   0, {oserv}},  		//function single arg
	{"SET"   ,40, NA,   9, {.funce=oset}}, 
	{"SETB",  40, NA,   2, {.func2=setb}},  	//set basic variable
	{"SETC",  40, NA,   9, {.funce=osetc}}, 	//set cell elemnt in matrix, or list
	{"SETK",  40, NA,   9, {.funce=osetk}}, 	//set dictionary element
	{"SETQ"  ,40, NA,   9, {.funce=osetq}}, 	//set variable
	{"SETSERVO",40,NA,  3, {.func3=osetservo}}, 	//set servo position
	{"SIG",  40, NA,    1, {osig}},  		//sigmoid functon
	{"SIGN", 40, NA,    1, {osign}}, 		//sigmoid functon
	{"SIN",  40, NA,    1, {osin}},  		//function single arg
	{"SQRT", 40, NA,    1, {osqrt}}, 		//function single arg
	{"STACK",40, NA,    1, {ostack}}, 
	{"SUBS", 40, NA,    1, {osubst}},		//function single arg
	{"SUM",  40, NA,    1, {osum}},  		//function <fMatrix>
	{"TAN",  40, NA,    1, {otan}},  		//function single arg
	{"TRN",  40, NA,    1, {otrn}},  		//ATRIX tRANSPOSE 
	{"TYPE", 40, NA,    1, {otype}}, 		//object type
	{"V2SM", 40, NA,    1, {ovsum2}},		//function <fMatrix>
	{"VSUM", 40, NA,    1, {ovsum}}, 		//function <fMatrix>
	{"WAIT", 40, NA,    1, {owait}},  
	{"WHILE",40, NA,    9, {.funce=owhile}}, 
	{"WITH" ,40, NA,    9, {.funce=owith}}, 
	{"WHOS", 40, NA,    1, {owhs}},
	{"ZERB", 40, NA,    5, {.func5=ozerob}},  	//Zero Matrix bounded
	{"ZERD", 40, NA,    1, {ozerod}},  		//Zero matrix diaganol
	{"ZERO", 40, NA,    2, {.func2=ozero}}    	//Zero Matrx x,y
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
	tOBJ r =  dict_getc(e, str);
	int n=-1;
	if (r.type == FUNC)
		n=  ((tOPp)(r.fptr))->level;	
	return n;
}

int getOPtype(Dict *e, char *str)
{
	tOBJ r =  dict_getc(e, str);
	int n=-1;
	if (r.type == FUNC) 
		n = ((tOPp)(r.fptr))->type;	
	return n;	
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
	if (n.type != SYM) 
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
	return -1;
}

//!> SETB "@A" '(2 4 5 1 6)
//!> SETB "A" 1
tOBJ setb(tOBJ n, tOBJ r)
{
	char *name;
	if (n.type != SYM) 
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

	printf ("Total size= %d (%d) - Mem %d\n",((Dict *) (env.dict))->ip, ((Dict *) (env.dict))->sz, bas_mem());

	if (dbg) bas_show();
	
	return emptyObj(); 
}

/**********************************************************/
/*  maths functions                                       */
/**********************************************************/

tOBJ omath(tOBJ o1, tOBJ o2, int op)
{
	tOBJ r = emptyObj();
	int i;

	if (dbg>1) {printf("math op : %d %d %d\n", o1.type, op, o2.type); } //debug info

	if (o1.type==EMPTY) return r;

	if (o1.type==CELL && o2.type==CELL) 
	{
		// + '(1 2 3) '(3 4 5)
		tOBJ a = omath(ocar(o1), ocar(o2), op);
		tOBJ b = omath(ocdr(o1), ocdr(o2), op);
		r = ocons(a, b);
		freeobj(&a);
		freeobj(&b);
		return r;
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
		case POWR:
			r.number = 1;
			for (int x=1; x<= o2.number; x++) r.number *= o1.number; 
			break;
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
		case POWR:
			r.floatpoint = pow (a, b); break;
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

	if (o1.type==SYM && o2.type==SYM && op==PLUS)
	{
		int n=strlen(o1.string)+strlen(o2.string)+1;
		char *s=newstring1(n);
		strcpy(s,o1.string);
		strcat(s,o2.string);
		r.type=SYM;
		r.string=s;
		r.q=0;
		r.cnt=0;
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
		if (r.fmat2==0 || r.fmat2->w*r.fmat2->h == 0)
		{
			r.type=EMPTY;
		}
	}

	if (o1.type==IMAG && o2.type==IMAG && op==PLUS)
	{
		r.type=IMAG;
		r.imgptr = cloneimage(o1.imgptr);
		for (int i=0; i<r.imgptr->h; i++)
			for (int j=0; j<r.imgptr->w; j++)
				r.imgptr->data[j+i*r.imgptr->w] = abs(r.imgptr->data[j+i*r.imgptr->w] + o2.imgptr->data[j + i*o2.imgptr->w])%256;	
	}

	if (o1.type==IMAG && o2.type==IMAG && op==MINUS)
	{
		r.type=IMAG;
		r.imgptr = cloneimage(o1.imgptr);
		for (int i=0; i<r.imgptr->h; i++)
			for (int j=0; j<r.imgptr->w; j++)
				r.imgptr->data[j+i*r.imgptr->w] = abs(r.imgptr->data[j+i*r.imgptr->w] - o2.imgptr->data[j + i*o2.imgptr->w])%256;	
	}

	if (o1.type==IMAG && o2.type==IMAG && op==MULT)
	{
		r.type=IMAG;
		r.imgptr = cloneimage(o1.imgptr);
		for (int i=0; i<r.imgptr->h; i++)
			for (int j=0; j<r.imgptr->w; j++)
				r.imgptr->data[j+i*r.imgptr->w] = abs(r.imgptr->data[j+i*r.imgptr->w] * o2.imgptr->data[j + i*o2.imgptr->w])%256;	
	}

	if (o1.type==IMAG && (o2.type==INTGR || o2.type==FLOAT) && op==PLUS)
	{
		r.type=IMAG;
		r.imgptr = cloneimage(o1.imgptr);
		for (i=0; i<o1.imgptr->w*o1.imgptr->h; i++)
			r.imgptr->data[i] = abs(r.imgptr->data[i] + toint(o2)) % 256;	
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
	return a;
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
	tOBJ r=emptyObj();
	if (a.type==FLOAT || a.type==INTGR)
	{
		r.type=FLOAT;
		r.floatpoint=sqrt(tofloat(a));
	}
	return r;
}

tOBJ ogaus(tOBJ m, tOBJ v, tOBJ x)
{

	float mean =tofloat(m);  // mu
	float var  =tofloat(v);  // sigma^2
	float xv   =tofloat(x);
	float Pi = 3.1414;
	tOBJ r = makefloat((1/sqrt(2*Pi*var)) * exp( -0.5 * ((xv-mean)* (xv-mean))/var));
	return r;
}

tOBJ ogausk(tOBJ ks, tOBJ gs)
{
	tOBJ r=emptyTPObj(FMAT2,gausian(toint(ks),toint(gs))); 
	return r;
}

tOBJ ornd(tOBJ a)
{
	tOBJ r=makefloat((float)rand()/RAND_MAX);
	return r;
}

tOBJ omax(tOBJ a, tOBJ b)
{
	int i;
	tOBJ r= emptyObj();

	if (r.type==INTGR && a.type==INTGR)
	{
		if (a.number>b.number) r=a; else r=b;
	}
		
	if (r.type==FLOAT && a.type==FLOAT)
	{
		if (a.floatpoint>b.floatpoint) r=a; else r=b;
	}

	if (a.type==FMAT2)
	{
		r.type=FLOAT;
		r.floatpoint=a.fmat2->fstore[0];
		for (i=1; i<a.fmat2->h*a.fmat2->w; i++)
			if (a.fmat2->fstore[i]>r.floatpoint) r.floatpoint= a.fmat2->fstore[i];
	}

	if (a.type==IMAG)
	{
		r.type=INTGR;
		r.number=maxval(a.imgptr);
	}

	if (a.type==CELL)
	{ 
		tOBJ v=ocar(a);	

		if (v.type==INTGR)
		{
			int n=toint(v);

			while (a.type != EMPTY)
			{
				if (toint(v) >n) n=toint(v);
				a = ocdr(a);
				v=ocar(a);
			}
			r.type=INTGR;
			r.number =n ;
		}
		if (v.type==FLOAT)
		{
			float n=0;

			while (a.type != EMPTY)
			{
				if (tofloat(v) >n) n=tofloat(v);
				a = ocdr(a);
				v=ocar(a);
			}
			r.type=FLOAT;
			r.floatpoint =n ;
		}
	}

	return r;
}

tOBJ omin(tOBJ a, tOBJ b)
{
	int i;
	tOBJ r= emptyObj();

	if (r.type==INTGR && a.type==INTGR)
	{
		if (a.number<b.number) r=a; else r=b;
	}
		
	if (r.type==FLOAT && a.type==FLOAT)
	{
		if (a.floatpoint<b.floatpoint) r=a; else r=b;
	}

	if (a.type==FMAT2)
	{
		r.type=FLOAT;
		r.floatpoint=a.fmat2->fstore[0];
		for (i=1; i<a.fmat2->h*a.fmat2->w; i++)
			if (a.fmat2->fstore[i]<r.floatpoint) r.floatpoint= a.fmat2->fstore[i];
	}

	if (a.type==IMAG)
	{
		r.type=INTGR;
		r.number=minval(a.imgptr);
	}

	if (a.type==CELL)
	{ 
		tOBJ v=ocar(a);	

		if (v.type==INTGR)
		{
			int n=toint(v);

			while (a.type != EMPTY)
			{
				if (toint(v) <n) n=toint(v);
				a = ocdr(a);
				v=ocar(a);
			}
			r.type=INTGR;
			r.number =n ;
		}
		if (v.type==FLOAT)
		{
			float n=0;

			while (a.type != EMPTY)
			{
				if (tofloat(v) <n) n=tofloat(v);
				a = ocdr(a);
				v=ocar(a);
			}
			r.type=FLOAT;
			r.floatpoint =n ;
		}
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
*/
tOBJ omat(tOBJ m, tOBJ a, tOBJ b) 
{
	//read matrix cell
	tOBJ r=emptyObj();

	int row=toint(a);
	int col=toint(b);

	if (m.type==FMAT2) 
	{
		r=makefloat(0.0);
		r.floatpoint = fget2((m.fmat2),col,row);
	}

	if (m.type==IMAG)
	{ 
		r.type=INTGR;
		r.number = getpoint(m.imgptr, col, row); 
	}
	return r;
}

tOBJ osetc(tOBJ o, Dict *e) 
{
	//SETC IM 1 1 255
	//SETC MA 1 1 0.5
	//SETC CE 1 V

	tOBJ exp = emptyObj();
	tOBJ var = ocar(o); 	 	o=ocdr(o);
	tOBJ a = eval(ocar(o),e); 	o=ocdr(o);
	tOBJ b = eval(ocar(o),e); 	o=ocdr(o);
	tOBJ c = eval(ocar(o),e); 

	if (var.type==SYM)
	{
		tOBJ arr=dict_getc(e, var.string);
		if (arr.type==IMAG)
		{
			int v = toint(c);
			setpoint(arr.imgptr,toint(a),toint(b),v);
			exp=makeint(v);
		}

		if (arr.type==FMAT2)
		{
			float v = tofloat(c);
			fset2(arr.fmat2,toint(a),toint(b), v);
			exp=makefloat(v);
		}

		if (arr.type==CELL)
		{
			//find nth element and update
			int cnt = toint(a);
			if (cnt<0) cnt=0;
			while (cnt>0 && arr.type == CELL)	
			{
				arr = ocdr(arr);
				cnt--;
			}
			if (cnt==0)
			{
				tCELLp p= arr.cell;
				freeobj(&(p->head));
				p->head = cloneObj(b);
				exp=b;
			}
		}
	}

	freeobj(&c);
	freeobj(&b);
	freeobj(&a);

	return exp;
}

tOBJ oimp (tOBJ a, tOBJ b, tOBJ c)
{
	//IMP "A" 2 2

	tOBJ r=emptyObj();

	if (a.type != SYM)
		return r;

	r.type=FMAT2;
	r.fmat2=fimport2(*(a.string), toint(b), toint(c)) ;
		
	return r;
}


tOBJ ominv(tOBJ a)
{
	//invert a matrix
	tOBJ r=emptyObj();
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
	tOBJ r=emptyObj();
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
	tOBJ r=emptyObj();
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
	if (a.type==IMAG)
	{ 
		r.type=INTGR;
		r.number = sumoImage(a.imgptr);
	}
	if (a.type==CELL)
	{ 
		tOBJ v=ocar(a);	

		if (v.type==INTGR)
		{
			int n=0;

			while (a.type != EMPTY)
			{
				n += toint(v);
				a = ocdr(a);
				v=ocar(a);
			}
			r.type=INTGR;
			r.number =n ;
		}
		if (v.type==FLOAT)
		{
			float n=0;

			while (a.type != EMPTY)
			{
				n += tofloat(v);
				a = ocdr(a);
				v=ocar(a);
			}
			r.type=FLOAT;
			r.floatpoint =n ;
		}
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
	if (a.type==IMAG)
	{ 
		r.type=INTGR;
		r.number = a.imgptr->h;
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
	if (a.type==IMAG)
	{ 
		r.type=INTGR;
		r.number = a.imgptr->w;
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
	tOBJ r=emptyObj();

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
	// APPLY 'PLUS '(2 3 4) -> 9

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
MAPCAR SIN '(0.100000 0.200000 0.300000)
MAPCAR 'FACT '(1 2 3 4)
MAPCAR (LAMBDA (X) (+ X 1)) '(1 2 3 4)
MAPCAR SIN [0.1 0.2 ; 0.3 0.4 ]
*/

tOBJ omapcar(tOBJ a, tOBJ b)
{
	tOBJ r=emptyObj();
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
		tOBJ v=emptyObj();
		// MAPCAR 'FOO [ 1 2 ; 3 4 ]
		int i=0, j=0;
		// create new matrix
		r.type=FMAT2;
		r.fmat2=fmatcp(b.fmat2);
	
		for (i=0; i<b.fmat2->w; i++)
		{
			for (j=0;j<b.fmat2->h; j++)
			{
				v =makefloat(fget2(b.fmat2,i,j));
				
				if (a.type==FUNC) 
				{
					tOBJ t =append(emptyObj(),v);
					freeobj(&v);
					v=t;
				}
				tOBJ n  = procall (a, v, env.dict);	
				fset2(r.fmat2,i,j,tofloat(n));		
				freeobj(&n);	
				freeobj(&v);	
			}
		}
	}
	else 
	if (b.type==IMAG)
	{
		tOBJ v=emptyObj();
		// MAPCAR 'FOO (IMAGE LOAD "test.jpg")
		int i=0, j=0;
		// create new IMAGE
		r.type=IMAG;
		r.imgptr=cloneimage(b.imgptr);
	
		for (i=0; i<b.imgptr->w; i++)
		{
			for (j=0;j<b.imgptr->h; j++)
			{
				v =makeint(b.imgptr->data[i+j*b.imgptr->w]);
				
				if (a.type==FUNC) 
				{
					tOBJ t =append(emptyObj(),v);
					freeobj(&v);
					v=t;
				}
				tOBJ n  = procall (a, v, env.dict);	

				r.imgptr->data[i+j*b.imgptr->w] = abs(toint(n))%256;
				freeobj(&n);	
				freeobj(&v);	
			}
		}
	}
	else if (b.type==CELL)
	{
		// MAPCAR FOO '(2 3 4)
		while (b.type != EMPTY)
		{
			tOBJ arg= ocar(b); b=ocdr(b);
			arg.q=1;
			if (a.type==FUNC) 
			{
				tOBJ ta = append(emptyObj(),arg);
				freeobj(&arg);
				arg=ta;

			}
			else
			{
				arg=cloneObj(arg);
			}

			tOBJ n  = procall (a, arg, env.dict);		
			tOBJ t =append(r,n);

			freeobj(&r);
			freeobj(&n);
			freeobj(&arg);
			r=t;
		}
	}
	return r;
}

tOBJ otrn(tOBJ a)
{
	tOBJ r=makefloat(0.0);
	if (a.type==FMAT2)
	{
		r.type = FMAT2;
		r.fmat2 = ftranspose2(a.fmat2); 
	}

	return r;
}

/*
!> RSHP [1 2 3] 3 3
 = Matrix 3x3
1.000000 2.000000 3.000000 
0.000000 0.000000 0.000000 
0.000000 0.000000 0.000000 
*/
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

	if (m.type==IMAG)
	{
		r.type=IMAG;
		r.imgptr = reshapeoImage(m.imgptr, col, row);
	}

	return r;
}

/*
!> REP [1 2 3] 3 1
 = Matrix 3x3
1.000000 2.000000 3.000000 
1.000000 2.000000 3.000000 
1.000000 2.000000 3.000000 
*/

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
	int row,col;
	tOBJ r=emptyObj();

	row=toint(a);
	col=toint(b);

	r.type=FMAT2;
	r.fmat2 = newmatrix(col, row);
	return r;
}

tOBJ oeye (tOBJ a, tOBJ b)
{
	int i;
	int row,col;
	tOBJ r=emptyObj();

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
	tOBJ r=emptyObj();

	if (a.type == FMAT2 &&  b.type==FMAT2)
	{
		r.type=FMAT2;
		r.fmat2=fconvolve(a.fmat2,b.fmat2) ;	
	}	
	return r;
}

tOBJ omcond(tOBJ a, tOBJ b, tOBJ c, tOBJ d, tOBJ e)
{
	// COND (Matrix,lv, uv, nv1, nv2)
	tOBJ r=emptyObj();
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
	tOBJ r=emptyObj();
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
	tOBJ r=emptyObj();
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
		r.cnt=1;
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
			r.cnt=1;
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

	if (a.type==SYM)
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

tOBJ ocons(tOBJ a, tOBJ b)
{
	//CONS 1 '(1 2 3)
	tOBJ r=emptyObj();

        if (b.type==CELL)
        {
		b=cloneObj(b);
                r=makeCell2(a, b.cell);
        }
	if (b.type==EMPTY)
	{
		r=makeCell();
		((tCELLp)(r.cell))->head=cloneObj(a);
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
	|| (a.type==SYM  && (a.string==NULL || *(a.string)=='\0')))	
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
		tOBJ exp = eval(ocar(o),e); o=ocdr(o);	
		tOBJ rn  = append(r, exp );
		freeobj(&exp);
		freeobj(&r);
		r=rn;
	}
	return r;
}

//PLUS 'A 'B 'C
//PLUS 1 2 3 4
tOBJ oplus(tOBJ o, Dict *e)
{
	tOBJ r = eval(ocar(o),e); 
	o=ocdr(o);	

	while (o.type != EMPTY)
	{
		tOBJ a = eval(ocar(o), e);
		o=ocdr(o);	
		tOBJ b = omath(r, a, PLUS);
		freeobj(&a);
		freeobj(&r);
		r=b;
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
			tOBJ t = append(r, ocar(exp));
			exp=ocdr(exp);
			freeobj(&r);
			r=t;
		}
		freeobj(&exp);
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
			freeobj(&t);
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
!> COND 'A 'B 'C
 = B
!> COND 1 'B 'C
 = B
!> COND 0 'B 'C
 = C
!> COND 0 'A 0 'B 1 'E
 = E
!> COND 0 'A 0 'B 1   
 = 1
!> COND  '() 1 2
 = 2
!> COND  '(1) 1 2
*/
tOBJ ocond(tOBJ o, Dict *e)
{
	tOBJ cond=emptyObj();
	tOBJ act=emptyObj();

	while (o.type != EMPTY)
	{
		freeobj(&cond);
		cond = eval(ocar(o),e); o=ocdr(o);
		act  = ocar(o); o=ocdr(o);
		if (dbg) {println ("cond =", cond);}
		if ((cond.type == INTGR && cond.number != 0 ) 
		|| (cond.type != INTGR && cond.type!=EMPTY && cond.type!=FLOAT)
		|| (cond.type == FLOAT && cond.floatpoint != 0.0 ) ) 
		{
			if (onull(act).number==1)
			{
				return cond;
			}
			else
			{
				freeobj(&cond);
				return eval(act,e);
			}
		}
	}	
	return cond;
}

/*
SETQ ENV (DICT '((AZ 1.0) (BZ 2.0)))
GETK 'ENV 'AZ
SETK 'ENV 'AZ 5.0
*/
tOBJ osetk(tOBJ o, Dict *e)
{
	tOBJ a=eval(ocar(o),e); o=ocdr(o);
	tOBJ b=eval(ocar(o),e); o=ocdr(o);
	tOBJ c=eval(ocar(o),e);
	tOBJ r=emptyObj();

	if (a.type==SYM && b.type==SYM)
	{
		//SETK 'ENV 'BZ 2.9

		tOBJ en = dict_getc(e, a.string);

		if (en.type==DICT)
		{
			dict_update(en.dict, b.string, c);
			r=cloneObj(c);
		}
	}

	freeobj(&a);
	freeobj(&b);
	freeobj(&c);

	return r;
}

tOBJ ogetk(tOBJ o, Dict *e)
{
	tOBJ a=eval(ocar(o),e); o=ocdr(o);
	tOBJ b=eval(ocar(o),e);
	tOBJ r=emptyObj();

	if (a.type==SYM && b.type==EMPTY)
	{
		//GETK 'A
		r = get(env.dict, a.string);
	}

	if (a.type==SYM && b.type!=EMPTY)
	{
		//GETK 'ENV 'AZ
		tOBJ t = dict_getc(e, a.string);

		if (t.type==DICT && b.type==SYM)
		{
			r = dict_getk(t.dict, b.string);
		}
		freeobj(&b);
	}
	freeobj(&a);	
	return r;
}

tOBJ oget(tOBJ a)
{
	//GETK '(ENV AZ)
	tOBJ r=emptyObj();
	if (a.type==CELL) 
	{
		tOBJ en=ocar(a);
		tOBJ var=ocar(ocdr(a));

		if (en.type==SYM && var.type==SYM)
		{
			en = dict_getc(env.dict, a.string);
			if (en.type==DICT)
			{
				r = get(en.dict, var.string);
			}
		}
	}	
	return r;
}


tOBJ onth(tOBJ a, tOBJ lst)
{
	//nth 0 '("one" "two" "three")
	//nth 5 '("one" "two" "three")
	//nth 1 "abcd"
	int cnt = toint(a);

	if (lst.type == CELL)
	{
		while (cnt>0)	
		{
			lst = ocdr(lst);
			cnt--;
		}
		return cloneObj(ocar(lst));
	}
	if (lst.type == SYM)
	{
		int ln = strlen(lst.string);
		if (cnt>=0 && cnt<ln)
			return makeint(lst.string[cnt]);
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
	if (r.type==SYM)
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
	if (r.type==SYM)
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

tOBJ omid(tOBJ  s, tOBJ a, tOBJ  b)
{
	//!MID "testing" 2 4  -> "est"
	//!MID "tes" 2        -> "es"
	tOBJ r=emptyObj();
	int p1=toint(a);
	int p2=toint(b);
	char *cp;
	if (s.type==SYM)
	{
		int ln = strlen(s.string);
		if (p2==0) p2=ln;
		if (ln>0 && p1>0 && p2>=p1 && p2<=ln)
		{
			cp=newstring1(p2-p1+1);
			strncpy(cp, (s.string)+p1-1,p2-p1+1);
			*(cp+p2-p1+1)='\0';
			r.type=SYM;
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

	if (r.type!=SYM) // two strings needed
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
	// SETSERVO 12 120 4
	// SETSERVO 12 'P
	// SETSERVO 12 'IO 4
	int id =toint(s);
	if (id<0 || id>31) return emptyObj();


	if (p.type==SYM && !strcmp(p.string,"P"))
	{
		wckPassive(id);
	}
	else if (p.type==SYM && !strcmp(p.string,"IO"))
	{
		unsigned char io=(toint(v))&0xFF;
		wckWriteIO(id,io);
	}
	else
	{
		int pos=toint(p);
		int tor=toint(v);
		if (pos>254 || tor<0 || tor>4) return emptyObj();
		wckMovePos(id,pos,tor);
	}
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
	char *mp = &m[0];
	int ch;
	if (n.type != SYM)
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

		if (ch==10 || ch==13) 	{cf=0; lc=0; continue;   }
		if (lc==0 && ch==';') 	{cf=1; continue;}
		if (cf==1) 		continue;
		if (ch=='`') {
			ch=fgetc(fp);
			if (ch>=100) 	{ m[cn++] = (ch/100) +'0'; }
			ch=ch%100;
			m[cn++] = (ch/10) +'0';
			m[cn++] = (ch%10) +'0';
			continue;
		}
		m[cn++]=ch;
		lc++;
	}
	m[cn]=0;
	if (dbg>2) printf ("Loaded [%s] %d\n", m,cn);	
	fclose(fp);

	switch (*mp)
	{
	case '[' : 
		mp++;
		r.type=FMAT2;
		r.fmat2 = readmatrix(&mp, ']');
		break;
	case '{' : 
		mp++;
		r.type=IMAG;
		r.imgptr = readmatrix(&mp, '}');
		break;
	case '!' : 
		r = eval_oxpr(mp+1); 
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

	if (f.type==SYM) 
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

			if (n.type==SYM )
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
		freeobj(&r);
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
		freeobj(&r);
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
	return cloneObj(o);
}

tOBJ ofore   (tOBJ  o, Dict *e)
{
	//!FOREACH X '(1 2 3)  (PR X)
	//!FOREACH X  [1 2 3]  (PR X)
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
				freeobj(&r);
				r = obegin(o,e);		
			}
		}
	}
	else
	if (list.type == IMAG)
	{		
		for (int i=0; i<(list.imgptr->w*list.imgptr->h); i++)
		{
			tOBJ v;
			v.type=INTGR;
			v.number =list.imgptr->data[i];
			dict_update(e, ind.string, v);
			freeobj(&r);
			r = obegin(o,e);		
		}
	}
	else
	if (list.type == CELL)
	{
		//FOREACH X '( (A 1) (B 2) (C 3)) (PR (CAR X))
		brkflg=0;
		tOBJ t= list;			
		do {
			tOBJ value=ocar(list);
			list=ocdr(list);
			dict_update(e, ind.string, value);		
			//loop
			freeobj(&r);
			r = obegin(o,e);

		} while (!(retflg || brkflg) && list.type != EMPTY);
		freeobj(&t);
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
	else
	if (list.type == DICT)
	{
		//SETQ E (DICT '(("A" 1) ("B" 2)))
		//FOREACH X E (PR X (GETK 'E X))
		Dict *p=list.dict;
		brkflg=0;
		for (int i=0; (!(retflg || brkflg) && (i<p->ip)); i++)
		{
			tOBJ t = makestring(p->db[i].key);
			dict_update(e, ind.string, t );
			freeobj(&r);
			freeobj(&t);
			r = obegin(o,e);
		}		
		brkflg=0;
	}
	else
	if (list.type == SYM)
	{
		//FOREACH X "string" (Putc X )
		int ln=strlen(list.string);
		brkflg=0;
		for (int i=0; (!(retflg || brkflg) && (i<ln)); i++)
		{
			dict_update(e, ind.string, makeint(list.string[i]));
			freeobj(&r);
			r = obegin(o,e);
		}		
		brkflg=0;
	}
	freeobj(&list);	
	return r;
}


tOBJ ofor (tOBJ  o, Dict *e)
{
	//!FOR X '(3 5) (PR X)
	int s,f;
	tOBJ r=emptyObj();

	tOBJ ind =ocar(o);
	o=ocdr(o);

	if (ind.type != SYM) return emptyObj();

	tOBJ range =eval(ocar(o),e);

	s=toint(eval(ocar(range),e));
	f=toint(eval(ocar(ocdr(range)),e));

	o=ocdr(o);
	brkflg=0;

	if (f>=s) {
		for (int i=s; (!(retflg || brkflg) && i<=f); i++)
		{
			dict_update(e, ind.string, makeint(i));
			freeobj(&r);
			r = obegin(o,e);
			i = toint(dict_getk(e, ind.string));
		}
	}
	else {
		for (int i=s; (!(retflg || brkflg) && i>=f); i--)
		{
			dict_update(e, ind.string, makeint(i));
			freeobj(&r);
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

#ifdef VJ
extern int    vj     (char *f) ;
extern int    vj_img (int w, int h, int m, unsigned char *A);
extern MyRect vj_get (int i) ;
#else
int vj     (char *f)                        {return 0;}
int vj_img (int w, int h, int m, unsigned char *A) {return 0;}
#endif


/*
RESULT	CMD		ARGS					EXAMPLE
======	========	====================			====================
(LIst)	CAMSHIFT	<I>  <n> <n>
<n>	COLOUR		<s>  <n> <n> <n> <n> 			IMAGE COLOUR "orange" 20 30 60 2
<I>	DRAWLINE	<I>  <n> <n> <n> <n>
<I>	DRAWRECT	<I>  <n> <n> <n> <n>
<I>	FILTER		<s>  <list>				IMAGE FILTER   "test.jpg" '(1 10 10 100 100 100)
<I> 	F-DETECT	<s>  [ <n> ] 				IMAGE F-DETECT "img.jpg" 1
<I>	LOAD		<n>  <s>				IMAGE LOAD     "Text" 8
<I>	NEW  		<n>  <n>
<I>	MAT		<M>  					IMAGE MAT [1 2; 3 4]
(LIst)	MEANSHIFT	<I>  <n> <n>
(LIst)	MOMENT		<I>
<M>	NORM		<n>  <s>				IMAGE NORM     "Text" 8
NIL	PGM		<I> <s>					IMAGE PGM {I}  test.pgm"				
<I>	PROCESS		<s>  <n> <n>				IMAGE PROC "test.jpg" 10 10
<I>	RAW		<s>
(list)	RECOG		<I> | INIT
(List)	REGION
<I>	SCALE		<I> <n> <n>		
<I>	SUBSET		<I> <list>				IMAGE SUBSET IM '(X Y W H)
<M>	SERIAL  	<n>  <s>				IMAGE SERIAL   "Text" 8
NIL	SHOW		<I> | <n>
NIL	THRESH		0 |  <n> <list> 			IMAGE THRESH 1 '(120 175 40 70 30 40)
NIL	TRAIN		<I>
NIL	UNLOCK
NIL	WAIT		<n>
*/

oFilter setFilterfromList(tOBJ v)
{		
	oFilter x;	
	x.minR = toint(ocar(v)); v=ocdr(v);
	x.maxR = toint(ocar(v)); v=ocdr(v);
	x.minG = toint(ocar(v)); v=ocdr(v);
	x.maxG = toint(ocar(v)); v=ocdr(v);
	x.minB = toint(ocar(v)); v=ocdr(v);
	x.maxB = toint(ocar(v));
	return x;
}

MyRect setRectfromList(tOBJ v)
{		
	MyRect x;	
	x.x      = toint(ocar(v)); v=ocdr(v);
	x.y      = toint(ocar(v)); v=ocdr(v);
	x.width  = toint(ocar(v)); v=ocdr(v);
	x.height = toint(ocar(v)); v=ocdr(v);
	return x;
}
			
extern void rellock();

tOBJ oimg(tOBJ v, Dict *e)
{
	tOBJ cmd = ocar(v);
	v=ocdr(v);

	if (cmd.type == SYM)
	{
		upperstring(cmd);

		if (!strcmp(cmd.string,"UNLOCK"))
		{
			rellock();
		}
		else
		if (!strcmp(cmd.string,"F-DETECT"))
		{
			//IMAGE F-DETECT "img2.jpg"
			//IMAGE F-DETECT "img.jpg" 1
			tOBJ a=eval(ocar(v), e); 
			tOBJ ret = emptyObj();
			if (a.type==SYM) 
			{
				oImage *im = loadoImage(a.string);

				if (im==0)
				{
					printf("Image load failed\n");
				}
				else
				{
					int  m   = maxval(im);
					int  s   = vj_img (im->w, im->h, m, im->data); 

					for (int i=0; i<s; i++)
					{
						MyRect r = vj_get(i);

						if (dbg); printf ("%d, %d, %d, %d\n", r.x,r.y,r.width,r.height);	
		
						tOBJ t= append(ret, makeint(r.x));
						freeobj(&ret); ret=t;
						t = append(ret, makeint(r.y));
						freeobj(&ret); ret=t;
						t = append(ret, makeint(r.width));
						freeobj(&ret); ret=t;
						t = append(ret, makeint(r.height));
						freeobj(&ret); 
						ret=t;
					}

					if (s>0 && toint(ocar(ocdr(v)))>0)
					{
						freeobj(&ret); 
						ret=emptyTPObj(IMAG, im);
					}
					else
						delimage(im); 
				}

			}
			freeobj(&a);
			return ret;
		}
		else
		if (!strcmp(cmd.string,"LOAD") || !strcmp(cmd.string,"NORM")  || !strcmp(cmd.string,"SERIAL")) 
		{
			tOBJ file= eval(ocar(v),e); v=ocdr(v);
			tOBJ a   = eval(ocar(v),e);
			int  sz  = toint(a);
			if (sz<1 || sz>sqrt(SCENESZ))
			{
        			printf("error = expect int size 1-%d\n", (int)sqrt(SCENESZ)); 
				return emptyObj();
			}
			if (file.type != SYM)
			{
				freeobj(&file);
				file = makestring("test.jpg"); //should read from env
			}

			if (file.type==SYM && loadimage(file.string, sz, &scene[0])==0)
			{
				if (!strcmp(cmd.string,"NORM") || !strcmp(cmd.string,"SERIAL"))
				{
					tOBJ r=emptyTObj(FMAT2);
					fMatrix *m;
					if (!strcmp(cmd.string,"SERIAL"))
						m = newmatrix(1,sz*sz);
					else				
						m = newmatrix(sz,sz);
					r.fmat2=m;
					float s=0.0;
					for (int i=0; i<sz*sz; i++) 
					{	
						m->fstore[i]=(float)scene[i]; 
						if (s< m->fstore[i]) 
							s=m->fstore[i] ;
					}
					for (int i=0; i<sz*sz; i++) 
						m->fstore[i]/=s;

					freeobj(&file);
					return r;	
				}
				else
				{
					nis=sz*sz;
					freeobj(&file);
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
			char *file = "test.jpg";
			tOBJ fn = eval(ocar(v),e);
			if (fn.type==SYM)
			{
				//SETQ F '(1 10 10 100 100 100)
				//IMAGE FILTER "test.jpg" F
				tOBJ f = eval(ocar(ocdr(v)),e);
				tOBJ r = emptyObj();
				if (f.type==CELL)
				{
					r= emptyTPObj(IMAG, FloadImage(fn.string, setFilterfromList(f)));
				}
				freeobj(&fn);
				freeobj(&f);
				return r;
			}
			else if (fn.type==INTGR)
			{
				//!IMAGE FILTER 4 1 10 10 100 100 100 (depracted)
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
				return emptyObj();
		}
		else
		if (!strcmp(cmd.string,"THRESH"))
		{
			int cid = toint(eval(ocar(v),e));  v=ocdr(v);
			if (cid==0)
			{
				//!IMAGE THRESH 0
        			printf("clr thresholds\n"); 
				clrmap();
			}
			else
			{				
				tOBJ ft = eval(ocar(v),e);
				if (ft.type==CELL)
				{					
					//!IMAGE THRESH 1 '(120 175 40 70 30 40)
					oFilter n =setFilterfromList(ft);
					add_thresh(cid, n.minR, n.maxR, n.minG, n.maxG, n.minB, n.maxB);
				}
				else
				{
					//!IMAGE THRESH 1 120 175 40 70 30 40
					int a,b,c,d,e,f;
					a = toint(ocar(v)); v=ocdr(v);
					b = toint(ocar(v)); v=ocdr(v);
					c = toint(ocar(v)); v=ocdr(v);
					d = toint(ocar(v)); v=ocdr(v);
					e = toint(ocar(v)); v=ocdr(v);
					f = toint(ocar(v));
					add_thresh(cid, a,b,c,d,e,f);
				}
				freeobj(&ft);
			}
			return emptyObj();
		}
		else
		if (!strcmp(cmd.string,"COLOR") || !strcmp(cmd.string,"COLOUR"))  
		{
			//!IMAGE COLOR "orange" 20 30 60 2
			tOBJ r=emptyObj();
			tOBJ colour = ocar(v);  v=ocdr(v);
			int a = toint(ocar(v)); v=ocdr(v);
			int b = toint(ocar(v)); v=ocdr(v);
			int c = toint(ocar(v)); v=ocdr(v);
			int d = toint(ocar(v));

			if (colour.type==SYM)
			{
				r  = makeint(add_color(colour.string, a,b,c,d));
			}
			return r;
		}
		else
		if (!strcmp(cmd.string,"NEW"))
		{
			int w=toint(eval(ocar(v),e));
			int h=toint(eval(ocar(ocdr(v)),e));
			tOBJ r = emptyTPObj(IMAG, makeimage(h,w));
			return r;
		}
		else
		if (!strcmp(cmd.string,"SCALE"))
		{
			tOBJ img=eval(ocar(v),e); v=ocdr(v);
			int  nw =toint(eval(ocar(v),e));
			int  nh =toint(eval(ocar(ocdr(v)),e));

			tOBJ r = emptyTPObj(IMAG, makeimage(nh,nw));

			if (r.type==IMAG)
			{
				int h=img.imgptr->h;
				int w=img.imgptr->w;

				if (nh>h) nh=h;
				if (nw>w) nw=w;

				tOBJ mat  = emptyTPObj(FMAT2, newmatrix(nw,nh));

				//cpy into mat values from img.impgptr->data
				int x,y;
				for (int i=0;i<h; i++)
				{
					y=(i*nh)/h;
					for (int j=0; j<w; j++)
					{
						x=(j*nw)/w;
						mat.fmat2->fstore[x+y*nw] += img.imgptr->data[j+i*w];
					}
				}
				float mx=fmatmax(mat.fmat2);

				if (mx != 0.0) for (x=0; x<nh*nw;x++) r.imgptr->data[x] = abs(256*(mat.fmat2->fstore[x] / mx))%256;

				freeobj(&mat);
				freeobj(&img);
			}			
			return r;
		}
		else
		if (!strcmp(cmd.string,"SUBSET"))
		{
			//IMAGE SUBSET IM '(X Y H W)
			tOBJ img=eval(ocar(v),e); v=ocdr(v);
			tOBJ val=eval(ocar(v),e); 
			tOBJ r=emptyObj();
			if (img.type==IMAG && val.type==CELL)
			{
				MyRect t = setRectfromList(val);
				r = emptyTPObj(IMAG, subimage(img.imgptr,t.x,t.y,t.width,t.height));
			}
			freeobj(&img);
			freeobj(&val);		
			return r;
		}
		else
		if (!strcmp(cmd.string,"MOMENT"))
		{
			tOBJ im = eval(ocar(v),e);
			tOBJ ret=emptyObj();
			if (im.type==IMAG)
			{
				int cx, cy;
				moment(im.imgptr, &cx, &cy);

				tOBJ t  = append(ret, makeint(cx));
				freeobj(&ret); ret=t;
				t = append(ret, makeint(cy));
				freeobj(&ret); ret=t;
			}
			freeobj(&im);
			return ret;
		}
		else
		if (!strcmp(cmd.string,"MEANSHIFT"))
		{
			tOBJ im = eval(ocar(v),e); v=ocdr(v);
			int  w  = toint(eval(ocar(v),e));  v=ocdr(v);
			int  h  = toint(eval(ocar(v),e));
			tOBJ ret=emptyObj();
			if (im.type==IMAG)
			{
				int cx, cy;
				meanshift(im.imgptr, 10, w, h, &cx, &cy); 
				tOBJ t  = append(ret, makeint(cx));
				freeobj(&ret); ret=t;
				t = append(ret, makeint(cy));
				freeobj(&ret); ret=t;
			}
			freeobj(&im);
			return ret;
		}
		else
		if (!strcmp(cmd.string,"CAMSHIFT"))
		{
			tOBJ im = eval(ocar(v),e); v=ocdr(v);
			int  w  = toint(eval(ocar(v),e));  v=ocdr(v);
			int  h  = toint(eval(ocar(v),e));
			tOBJ ret=emptyObj();
			if (im.type==IMAG)
			{
				int cx=0, cy=0, nw=0, nh=0;
				camshift(im.imgptr, 10, w, h, &cx, &cy, &nw, &nh); 
				tOBJ t  = append(ret, makeint(cx));
				freeobj(&ret); ret=t;
				t = append(ret, makeint(cy));
				freeobj(&ret); ret=t;
				t = append(ret, makeint(nw));
				freeobj(&ret); ret=t;
				t = append(ret, makeint(nh));
				freeobj(&ret); ret=t;
			}
			freeobj(&im);
			return ret;
		}
		else
		if (!strcmp(cmd.string,"MAT"))
		{
			tOBJ m = eval(ocar(v),e);
			tOBJ r = emptyObj();
			if (m.type==FMAT2)
			{
				int h=m.fmat2->h;
				int w=m.fmat2->w;
				float mx = fmatmax(m.fmat2);
				oImage *p = makeimage(h,w);
				if (p != NULL)
				{
					r.type=IMAG;
					r.imgptr=p;
					for (int i=0; i<h*w; i++) r.imgptr->data[i] = (int)(255*m.fmat2->fstore[i]/mx);
				}
			}
			return r;
		}
		else
		if (!strcmp(cmd.string,"RAW"))
		{
			tOBJ file= eval(ocar(v),e);
			tOBJ r = emptyObj();
			if (file.type==SYM) 
			{
				oImage *p = loadoImage(file.string);
				if (p != NULL)
				{
					r.type=IMAG;
					r.imgptr=p;
				}
			}
			else
			{
				//backward compatibility
				r = makeint(loadJpg("test.jpg"));		
			}
			freeobj(&file);
			return r;
		}
		else
		if (!strcmp(cmd.string,"PROC") || !strcmp(cmd.string,"PROCESS"))
		{
			tOBJ p=eval(ocar(v),e);
			if (p.type==SYM)
			{
				//IMAGE PROC "test.jpg" 10 10
				v=ocdr(v);
				int  w  = toint(ocar(v));  v=ocdr(v);
				int  h  = toint(ocar(v));
				tOBJ r  = emptyTPObj(IMAG, processImage(p.string, w, h)); 
				freeobj(&p);
				return r;
			}
			else
			{
				//backward compatibility (assumes IMAGE RAW called)
				//IMAGE PROC 5
				int n=toint(p);
				processFrame(n, &scene[0]);
				nis=n*n;
				return makeint(nis);
			}
		}
		else
		if (!strcmp(cmd.string,"REG") || !strcmp(cmd.string,"REGION"))
		{
			return color_regions(); 
		}
		else
		if (!strcmp(cmd.string,"SHOW"))
		{
			int  sz=sqrt(nis);
			tOBJ p =eval(ocar(v),e);

			if (p.type==IMAG)
			{
				tOBJ p2=eval(ocar(ocdr(v)),e);
				if (toint(p2)==1)
				{
					printimage(p.imgptr);
					printf ("\n");
					imageshow(p.imgptr);
				}
				else
				{
					printimage(p.imgptr);
					printf ("\n");
					imageogray(p.imgptr);
				}
				freeobj(&p);
				freeobj(&p2);
				return emptyObj();
			}

			int n=toint(ocar(v));

			frame=&scene[0];

			switch (n) {
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
		if (!strcmp(cmd.string,"RECOG"))
		{
			tOBJ im  = eval(ocar(v),e);
			tOBJ ret = emptyObj();
			int r=0;
			if (im.type==IMAG)
			{
				r=recognition(im.imgptr);
				if (dbg) recog_dump(2);

					tOBJ t= append(ret, makeint(r));
					ret   = append(t, makeint(recog_result(r)));
					freeobj(&t);
			}
			if (im.type==SYM && !strcmp(im.string,"INIT"))
			{
				recog_del(); //clean up memory
			}
			freeobj(&im);
			return ret;
		}
		else
		if (!strcmp(cmd.string,"TRAIN"))
		{
			tOBJ im = eval(ocar(v),e);
			tOBJ c  = eval(ocar(ocdr(v)),e);
			if (im.type==IMAG && c.type==INTGR)
			{
				training (im.imgptr, toint(c));
				if (dbg) recog_dump(1);
			}

			freeobj(&im);
			freeobj(&c);
			return emptyObj();
		}
		else
		if (!strcmp(cmd.string,"PGM"))
		{
			tOBJ im = eval(ocar(v),e);
			tOBJ fn = eval(ocar(ocdr(v)),e);
			tOBJ r=emptyObj();

			if (im.type==IMAG && fn.type==SYM)
			{
				int n = image2Pgm(im.imgptr, fn.string);
				r=makeint(n);
			}
			
			freeobj(&im);
			freeobj(&fn);
			return r;
		}
		else
		if (!strcmp(cmd.string,"DRAWLINE") || !strcmp(cmd.string,"DRAWRECT"))
		{

			tOBJ r  = emptyObj();
			tOBJ im = eval(ocar(v),e); v=ocdr(v);
			tOBJ rc = eval(ocar(v),e); v=ocdr(v);

			if (im.type==IMAG && rc.type==CELL)
			{
				//IMAGE DRAWLINE (IMAGE NEW 10 10)  '(1 2 3 4)   ; (a,b) -> (c,d))
				r = cloneObj(im);
     				MyRect z = setRectfromList(rc);

				if (!strcmp(cmd.string,"DRAWLINE"))
				{
					drawline(r.imgptr, z.x, z.y, z.width, z.height, 255);
				}
				else
				{
					drawrect(r.imgptr, z.x, z.y, z.width, z.height, 255);
				}
			}
			freeobj(&im);
			freeobj(&rc);
			return r;
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
	return makestack(toint(a),0);
}

tOBJ opipe(tOBJ a)
{
	if (a.type!=INTGR) return emptyObj();
	return makestack(toint(a),1);
}

tOBJ opush(tOBJ o, Dict *e)
{
	tOBJ a=ocar(o); o=ocdr(o);
	tOBJ b= emptyObj();

	if (a.type != SYM)
		return emptyObj();

	a=dict_getc(e, a.string); //copy

	if (a.type==STACK) 
	{
		b=eval(ocar(o),e); 

		if (!push((tStackp)a.stk, b))
		{
			freeobj(&b);
			b= emptyObj();
		}
	}
	return b;
}

tOBJ opop(tOBJ o, Dict *e)
{
	tOBJ a=ocar(o); 
	tOBJ r= emptyObj();

	if (a.type != SYM)
		return r;

	a=dict_getc(e, a.string); //copy

	if (a.type!=STACK) return r;
	r = pop((tStackp)a.stk);
	return r;
}

tOBJ opeek(tOBJ o, Dict *e)
{
	tOBJ a= ocar(o); o=ocdr(o);
	tOBJ b= ocar(o);
	tOBJ r= emptyObj();

	if (a.type != SYM)
		return r;

	a=dict_getc(e, a.string); //copy

	if (a.type!=STACK) return r;
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
	if (v.type==SYM)
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
	if (v.type==SYM) 
	{ 
		m = rbmload(v.string) ;
		rbmprint (m);
		r.type=RBM;
		r.mot = m;
		r.q=0;
	}
	return r;
}

tOBJ omatr(tOBJ n, tOBJ v, tOBJ f)
{
	if (n.type==SYM && v.type==INTGR && f.type==SYM)
	{
		// MAT 'LOAD  8 "text.txt"
		// MAT 'STORE 8 "test.txt"
		if (!strcmp(n.string,"LOAD"))
		{
			matrixload(toint(v), f.string);
		}
		if (!strcmp(n.string,"STORE"))
		{
			matrixstore(toint(v), f.string);
		}
	}

	if (n.type==SYM && v.type==FLOAT)
	{
		// MAT 'I 1.0
		tOBJ img=dict_getc(env.dict, n.string);
		if (img.type==IMAG)
		{
			int h=img.imgptr->h;
			int w=img.imgptr->w;
			tOBJ mat  = emptyTObj(FMAT2);
			mat.fmat2 = newmatrix(w,h);
			//cpy into mat values from img.impgptr->data
			for (int i=0;i<h; i++)
			{
				for (int j=0; j<w; j++)
				{
					mat.fmat2->fstore[j+i*w] = img.imgptr->data[j+i*w];
				}
			}
			return mat;

		}
	}

	if ((n.type==SYM || n.type==IMAG) && v.type==INTGR && f.type==INTGR)
	{
		// MAT 'I 5 5
		tOBJ img;

		if (n.type==SYM)
			img=dict_getc(env.dict, n.string);
		else
			img=n;

		if (img.type==IMAG)
		{
			int h=img.imgptr->h;
			int w=img.imgptr->w;
			int nh=toint(f);
			int nw=toint(v);

			if (nh>h) nh=h;
			if (nw>w) nw=w;

			tOBJ mat  = emptyTObj(FMAT2);
			mat.fmat2 = newmatrix(nw,nh);

			//cpy into mat values from img.impgptr->data
			int x,y;
			for (int i=0;i<h; i++)
			{
				y=(i*nh)/h;
				for (int j=0; j<w; j++)
				{
					x=(j*nw)/w;
					mat.fmat2->fstore[x+y*nw] += img.imgptr->data[j+i*w];
				}
			}
			float mx=fmatmax(mat.fmat2);
			if (mx != 0.0) for (x=0; x<nh*nw;x++) mat.fmat2->fstore[x] /= mx;
			
			return mat;

		}
	}

	return emptyObj();
}


/***********************************************************************

key functions

************************************************************************/

tOBJ formula(tOBJ ae, Dict *e)
{
	// ( V op V )
	// V op V op V
	// F (arg) -tbd
	// LET A = 6
	// LET A = 1 + 2
	// LET A = A + 2 + 4

	if (oatom(ae).number==1) {return ae;}

	tOBJ ops=makestack(10,0);
	tOBJ opd=makestack(10,0);

	while (1)
	{
		if (onull(ae).number==1) {freeobj(&ops); freeobj(&opd); return emptyObj();}

		if (oatom(ocar(ae)).number==1) 
			push (opd.stk, ocar(ae)); 
		else
			push (opd.stk, formula(ocar(ae),e));

		ae = ocdr(ae);
		while (1)
		{
			int wa=-1,wb=-1;
			if (dbg) { /*okey(makeint(2)); */ println ("ae =", ae); }

			if (onull(ae).number==1 && stacksize(ops.stk)==0 ) 
			{
				tOBJ r = pop(opd.stk);
				freeobj(&ops); 
				freeobj(&opd); 
				freeobj(&ae);
				return r;
			}

			tOBJ a = ocar(ae);
			tOBJ b = peek(ops.stk,0); 

			if (dbg) {println ("a =", a); println ("b =", b);}

			if (a.type==SYM)
			{
				wa=getOPlevel(e, a.string); 
				if (wa<0)
				{ 
					tOBJ nae = ocons(a,ae);
					freeobj(&ae);
					ae=nae;
					wa=0;
				}
			}

			if (b.type==SYM)
			{
				wb=getOPlevel(e, b.string);
				freeobj(&b);
			}

			if (onull(ae).number==1 ||  (wa <= wb))
			{
 				if (dbg) {printf ("P1 : %d %d\n",wa,wb); }

				tOBJ opc =  peek(ops.stk,0);
				if (dbg) println ("opc =", opc);

				if (opc.type==SYM)
					wa=getOPtype(e, opc.string);
				else
					wa=0;

				freeobj(&a);

				tOBJ a1=pop(opd.stk);
				tOBJ b1=pop(opd.stk);

				a = eval(a1,e);
				b = eval(b1,e);

				tOBJ ans= omath(a, b, wa);

				push(opd.stk,ans);
				
				tOBJ x = pop(ops.stk); 
				freeobj(&x);
				freeobj(&opc);
				freeobj(&a);
				freeobj(&b);
				freeobj(&a1);
				freeobj(&b1);
			}
			else
			{
				if (dbg) {printf ("P2 : %d %d\n",wa,wb); }
				push(ops.stk,a);
				freeobj(&a);
				break;
			}

		}
		ae = ocdr(ae);
	}
	freeobj(&ops); 
	freeobj(&opd); 
}


tOBJ olet(tOBJ o, Dict *e) 
{
	//LET VAR = FORMULA
	tOBJ var = ocar(o); o=ocdr(o);
	tOBJ eq =  ocar(o); o=ocdr(o);
	if (eq.type != SYM && strcmp(eq.string,"=")) {printf ("err\n"); return emptyObj();}
	eq = formula(o,e);
	dict_update(e, var.string, eq);
	return eq;
}


tOBJ oquote(tOBJ o, Dict *e) 
{
	return ocar(o);
}

tOBJ oeval(tOBJ o, Dict *e) 
{
	tOBJ a=eval(ocar(o),e);
	tOBJ c=eval(a,e);
	freeobj(&a);
	return c;
}

tOBJ oinc(tOBJ o, Dict *e) 
{
	tOBJ var = ocar(o); 
	tOBJ expr =omath(dict_getk(e,var.string), makeint(1), PLUS);
	dict_update(e, var.string, expr);
	return expr;
}

tOBJ odec(tOBJ o, Dict *e) 
{
	tOBJ var = ocar(o); 
	tOBJ expr =omath(dict_getk(e,var.string), makeint(1), MINUS);
	dict_update(e, var.string, expr);
	return expr;
}

extern int readLine(char *line);

tOBJ oread(tOBJ o, Dict *e) 
{
	tOBJ var = eval(ocar(o),e);
	if (var.type==SYM)   {tOBJ t = tokenise(var.string); freeobj(&var); return t; }
	if (var.type==EMPTY) 
	{
		char ibf[1024];
		printf("?");
		readLine(ibf);
		return tokenise(ibf);
	}
        return emptyObj();
}

tOBJ osetx(tOBJ o, Dict *e, int f) 
{
	tOBJ exp = emptyObj();
	while (o.type != EMPTY)
	{
		tOBJ var = ocar(o); o=ocdr(o);
		tOBJ z=ocar(o);
		freeobj(&exp);
		exp = eval(z,e); 
		o=ocdr(o);

		if (f) {tOBJ t=var; var=eval(var,e); freeobj(&t); }

		if (var.type==SYM)
			dict_update(e, var.string, exp);
		else
		if (var.type==CELL && exp.type==CELL)
		{
			tOBJ v1, v2, t=exp;
			while (var.type != EMPTY)
			{
				v1 = ocar(var); var=ocdr(var);
				v2 = ocar(exp); exp=ocdr(exp); //leak
				if (v1.type==SYM)
					dict_update(e, v1.string, v2);
			}
			freeobj(&t);
			return emptyObj();
		}
	}
	return exp;
}

tOBJ oset(tOBJ o, Dict *e) 
{
	return osetx(o, e, 1);
}

tOBJ osetq(tOBJ o, Dict *e) 
{
	return osetx(o, e, 0);
}

tOBJ owith(tOBJ o, Dict *e) 
{
        while (o.type != EMPTY)
        {
                tOBJ var = ocar(o); o=ocdr(o);
		if (var.type==CELL) {
			tOBJ n   = ocar(var);
                        tOBJ val = eval(ocar(ocdr((var))),e); 
                        if (n.type==SYM)
			{
                                dict_add(e, n.string, val);
			}
			freeobj(&val);
		}
		else
		{
                        dict_add(e, var.string, emptyObj());
		}
        }
        return emptyObj();
}


tOBJ oif(tOBJ o, Dict *e) 
{
        tOBJ test   = ocar(o); o=ocdr(o);
	test = eval(test,e);
        tOBJ conseq = ocar(o); o=ocdr(o);
        tOBJ alt    = ocar(o); 

	if (       (test.type == INTGR && test.number != 0 ) 
		|| (test.type == FLOAT && test.floatpoint != 0.0 ) 
                || (test.type != INTGR && test.type != FLOAT && test.type!=EMPTY)) 
        {
		freeobj(&test);
                return eval(conseq,e);
        }
        else
        {
		freeobj(&test);
                return eval(alt,e);
        }
}

tOBJ oform(tOBJ o, Dict *e) 
{
	return formula(o,e);
}


tOBJ ofunc(tOBJ o, Dict *e) 
{
	//!FUNC ABC (X Y) (PLUS X Y)
	tOBJ fn = ocar(o); o=ocdr(o);
	o.type  = LAMBDA;
	set(e, fn.string, o);
	printf ("Function %s\n", fn.string);
	return o;
}

tOBJ olambda(tOBJ o, Dict *e) 
{
	//!LAMBA (X Y) (PLUS X Y)
	o.type  = LAMBDA;
	return o;
}

tOBJ oclear(tOBJ o, Dict *e) 
{
	int n=e->sz;
	deldict(e);
	e = newdict(n);
	if (n> 200) loadop(e);
	return emptyObj();
}





