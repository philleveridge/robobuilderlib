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
extern int  *frame;

/* new matrix functions */

fMatrix readmatrix(char *s);
fMatrix newmatrix(int c, int r);
fMatrix fadd2(fMatrix *A, fMatrix *B, char op);  
int 	fmatprint(FILE *fp, fMatrix *A);
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


#define iswhite(c)   (c == ' ' || c == '\t' || c==13 || c==10)
#define isnumdot(c)  ((c >= '0' && c <= '9') || c == '.')
#define isnumber(c)  (c >= '0' && c <= '9')

#define isdelim(c)  (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '<'  || c == '>'  \
                    || c == '^' || c == '(' || c == ')' || c == ',' || c == '='  || c == '$' || c=='@' || c=='{' || c== '}')

#define isalpha(c)  ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')


#define MAXSTACK 50
#define MAXSTRING 1024

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
		tOBJ	(*func)(tOBJ);
} tOP, *tOPp;

tOBJ osin ();
tOBJ ocos(tOBJ  r);
tOBJ otan(tOBJ  r);
tOBJ osqrt(tOBJ  r);
tOBJ olog(tOBJ  r);
tOBJ oexp(tOBJ  r);
tOBJ oacos(tOBJ  r);
tOBJ oatan(tOBJ  r);
tOBJ oabs(tOBJ  r);
tOBJ oacx(tOBJ  r);
tOBJ oacy(tOBJ  r);
tOBJ oacz(tOBJ  r);
tOBJ osig(tOBJ  r);
tOBJ ornd(tOBJ  r);
tOBJ odsig(tOBJ  r);
tOBJ opsd(tOBJ  r);
tOBJ omax(tOBJ  r);
tOBJ omat(tOBJ  r);
tOBJ otrn(tOBJ  r);
tOBJ oapply(tOBJ  r);
tOBJ orshp(tOBJ  r);
tOBJ orep(tOBJ  r);

tOBJ ozero(tOBJ  r);
tOBJ oeye(tOBJ  r);
tOBJ ovsum(tOBJ  r);
tOBJ ohsum(tOBJ  r);
tOBJ ovsum2(tOBJ  r);
tOBJ ohsum2(tOBJ  r);
tOBJ oconv(tOBJ  r);
tOBJ oimp(tOBJ  r);
tOBJ ocond(tOBJ  r);
tOBJ ozerob(tOBJ  r);
tOBJ ozerod(tOBJ  r);

tOBJ ocar(tOBJ  r);
tOBJ ocdr(tOBJ  r);

tOBJ ecar(tOBJ  r);
tOBJ ecdr(tOBJ  r);

tOBJ otype(tOBJ  r);
tOBJ olen(tOBJ  r);
tOBJ oapp(tOBJ  r);
tOBJ olist(tOBJ  r);
tOBJ ocons(tOBJ  r);
tOBJ olast(tOBJ  r);
tOBJ osubst(tOBJ  r);
tOBJ orev(tOBJ  r);
tOBJ onull(tOBJ  r);
tOBJ omemb(tOBJ  r);
tOBJ oatom(tOBJ  r);
tOBJ oasso(tOBJ  r);
tOBJ oexec(tOBJ  r);
tOBJ opr(tOBJ  r);
tOBJ oset(tOBJ  r);
tOBJ oget(tOBJ  r);
tOBJ oserv(tOBJ  r);
tOBJ onth(tOBJ  r);
tOBJ opose(tOBJ  r);
tOBJ owhs(tOBJ  r);
tOBJ oexit(tOBJ  r);
tOBJ obf(tOBJ  r);

tOBJ oload(tOBJ  r);
tOBJ osave(tOBJ  r);
tOBJ odict(tOBJ  r);

tOBJ omatr(tOBJ  r);// {"LOAD" "STOR"}
tOBJ oimg(tOBJ  r); // {"UNLO" "LOAD" "FILT" "RAW" "THRE" "COLO" "PROC" "REG" "SHOW" "DEBU"}

tOBJ oprt(tOBJ  r);
tOBJ olet(tOBJ  r); 
tOBJ odefn(tOBJ  r); 
tOBJ odo(tOBJ  r); 
tOBJ oqt(tOBJ  r); 

tOBJ omath(tOBJ o1, tOBJ o2, int op);
tOBJ print(tOBJ r);
tOBJ get(Dict *ep, char *name);


tOP oplist[] = { 
/* 0 */	{"+",    10, PLUS,  2, NULL},
	{"-",    10, MINUS, 2, NULL},
	{"/",    20, DIVD,  2, NULL},
	{"*",    20, MULT,  2, NULL},
/* 5 */	{".*",   20, PROD,  2, NULL},
	{".^",   20, POWR,  2, NULL},
	{"AND",  8,  LAND,  2, NULL},
	{"OR",   8,  LOR,   2, NULL},
/*10 */	{"<",    5,  LT,    2, NULL},
	{">",    5,  GT,    2, NULL},
	{"=",    5,  EQL,   2, NULL},
	{"<>",   5,  NEQ,   2, NULL},
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
/*40 */	{"H2SM", 40, NA,    1, ohsum2},  //function <fMatrix>
	{"V2SM", 40, NA,    1, ovsum2},  //function <fMatrix>
	{"APPL", 40, NA,    2, oapply}, //function two args   <fMatrix>
	{"CONV", 40, NA,    2, oconv},  //function three args   <fMatrix>
	{"IMP",  40, NA,    3, oimp},   //function two args   <string>, <int>, <int>
	{"COND", 40, NA,    5, ocond},  //function four args   <fmatrix>, <min>, <max> <value>
	{"ZERB", 40, NA,    4, ozerob}, //function four args   <fmatrix>, <int> <int> Mint> <int>
	{"ZERD", 40, NA,    1, ozerod},  //function 1 args   <fmatrix>
//LIST BASED
	{"CAR",  40, NA,    1, ecar},  //function single arg
	{"CDR",  40, NA,    1, ecdr},  //function single arg

/*50 */	{"TYPE", 40, NA,    1, otype},  //function single arg
	{"APPD", 40, NA,    1, oapp},  //function single arg
	{"LIST", 40, NA,    1, olist},  //function single arg
	{"CONS", 40, NA,    1, ocons},  //function single arg
	{"LEN",  40, NA,    1, olen},  //function single arg
	{"SUBS", 40, NA,    1, osubst},  //function single arg
	{"LAST", 40, NA,    1, olast},  //function single arg
	{"REV",  40, NA,    1, orev},  //function single arg
	{"NULL",  40, NA,   1, onull},  //function single arg
	{"MEMB",  40, NA,   1, omemb},  //function single arg
/*60 */	{"ASSN",  40, NA,   1, oasso},  //function single arg
	{"ATOM",  40, NA,   1, oatom},  //function single arg
	{"EXEC",  40, NA,   1, oexec},  //function single arg
	{"SET",   40, NA,   1, oset}, //function single arg
	{"GET",   40, NA,   1, oget}, //function single arg
	{"SERV",  40, NA,   0, oserv}, //function single arg
	{"POSE",  40, NA,   1, opose}, //function single arg
	{"NTH",   40, NA,   1, onth}, //function single arg
	{"PR",    40, NA,   1, opr},   //function single arg
	{"WHOS",  40, NA,   0, owhs},
/*70 */	{"BF",    40, NA,   1, obf},
	{"MAT",   40, NA,   2, omatr},// 
	{"DICT",  40, NA,   1, odict},// 
	{"LOAD",  40, NA,   1, oload},//
	{"SAVE",  40, NA,   1, osave},//
	{"DO",    40, NA,   1, oexec},//
	{"DEFN",  40, NA,   1, odefn},//
	{"QT",    40, NA,   1, oqt},//
	{"EXIT",  40, NA,   0, oexit}
 
/* TBD - move all to function
	{"LET",     40, NA,  0, olet},
	{"IMAG",    40, NA,  1, oimg}, // {"UNLO" "LOAD" "FILT" "RAW" "THRE" "COLO" "PROC" "REG" "SHOW" "DEBU"}
	{"PRIN",    40, NA,  1, oprt},
*/

};



/**********************************************************/
/*  strings                                              */
/**********************************************************/

char *newstring(char *s)
{
	char *r;
	int n=strlen(s);
	if (dbg) printf ("New String [%d]\n",n);
	r=(char *)malloc(n*sizeof(char)+1);
	strcpy(r, s);
	return r;
}

char *newstring1(int n)
{
	char *r;
	if (dbg) printf ("New String [%d]\n",n);
	r=(char *)malloc(n*sizeof(char)+1);
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
/*  CELLS                                              */
/**********************************************************/
int freeobj(tOBJ *b);

tOBJ emptyObj()
{
	tOBJ r;
	r.type=EMPTY;
	return r;
}

int compareObj(tOBJ a, tOBJ b)
{
	if (a.type != b.type) return 0; //false;

	//SYM, INTGR, BOOLN, FUNC, FLOAT, STR, CELL, EMPTY, FMAT2};
	if (a.type == INTGR) return a.number==b.number;
	if (a.type == FLOAT) return a.floatpoint==b.floatpoint;
	if (a.type == STR)   return !strcmp(a.string,b.string);
	if (a.type == EMPTY) return 1;
	return 0;
}

tCELLp newCell()
{
	if (dbg) printf ("New CELL\n");	
	return (tCELLp)malloc(sizeof(tCELL));
}

void delCell(tCELLp p)
{
	if (dbg)  printf ("Delete CELL\n");	
	freeobj(&p->head);
	if (p->tail !=NULL) delCell((tCELLp)p->tail);
	free(p);
}

tOBJ makeCell()
{
	tOBJ r; tCELLp p;
	r.type=CELL;
	p = newCell();
	p->head=emptyObj();
	p->tail=0;
	r.cell=p;
	r.cnt=0;
	return r;
}


tOBJ cloneObj(tOBJ z);

tOBJ makeCell2(tOBJ a, tCELLp b)
{
	// a + {b} => {a b}
	tOBJ r; tCELLp p;
	r.type=CELL;
	p = newCell();
	p->head=cloneObj(a);  //clone ?
	p->tail=b;
	r.cell=p;
	r.cnt=0;
	return r;
}

tOBJ makeCell3(tOBJ a, tCELLp b)
{
	// a + {b} => {b a}
	tOBJ r; tCELLp p;
	r.type=CELL;
	p = newCell();
	p->head=cloneObj(a);  //clone ?
	p->tail=NULL;

	while (b->tail != NULL) b=b->tail;
	r.cell=b;
	return r;
}

tCELLp cloneCell(tCELLp p)
{
	tCELLp top, t,r=newCell();
	top=r;
	while (p!=NULL)
	{
		r->head =cloneObj(p->head);  //cloneObj??
		p=p->tail;
		t=r;
		if (p!=NULL)
		{
			r=newCell();
			t->tail=r;
		}
		else
			t->tail=NULL;
	}
		
	return top;
}

tOBJ cloneObj(tOBJ z)
{
	tOBJ r = z; // needs fixing for dynamic typees

	if (r.type==STR)
	{
		r.string = newstring(z.string);
	}
	if (r.type==CELL || r.type==LAMBDA)
	{
		r.cell = cloneCell((tCELLp)z.cell);
	}
	if (r.type==FMAT2)
	{
		//TBD
	}
	return r;
}


tOBJ readcells();

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

tOBJ cnvtInttoList(int an, int *array)
{
	tOBJ r,n,top;
	int i;

	if (an<=0) return emptyObj();
	top=makeCell2( makeint(array[0]), NULL);
	r=top;

	for (i=1; i<an; i++)
	{
		n=makeCell2(makeint(array[i]), NULL);
		((tCELLp)(r.cell))->tail = n.cell;
		r=n;
	}
	((tCELLp)(n.cell))->tail = 0;
	return top;
}

tOBJ cnvtBytetoList(int an, BYTE *array)
{
	tOBJ r,n,top;
	int i;

	if (an<=0) return emptyObj();
	top=makeCell2( makeint(array[0]), NULL);
	r=top;

	for (i=1; i<an; i++)
	{
		n=makeCell(makeint((int)array[i]), NULL);
		((tCELLp)(r.cell))->tail = n.cell;
		r=n;
	}
	((tCELLp)(n.cell))->tail = 0;
	return top;
}

tOBJ cnvtFloattoList(int an, float *array)
{
	tOBJ r,n,top;
	int i;

	if (an<=0) return emptyObj();
	top=makeCell2(makefloat(array[0]), NULL);
	r=top;

	for (i=1; i<an; i++)
	{
		n=makeCell2(makefloat(array[i]), NULL);
		((tCELLp)(r.cell))->tail = n.cell;
		r=n;
	}
	((tCELLp)(n.cell))->tail = 0;
	return top;
}

/**********************************************************/
/*  Dictionary                                            */
/**********************************************************/


Dict * newdict()
{
	Dict *n;
	if (dbg) printf ("New dictionary\n");
	n = (Dict *)malloc(sizeof(Dict));
	n->sz=100;
	n->ip=0;
	n->db = (Kvp *)malloc((n->sz) * sizeof(Kvp));
	return n;
}

int deldict(Dict *x)
{
	if (dbg) printf ("Delete dictionary\n");
	return 0;
}

tOBJ makedict()
{
	tOBJ r=emptyObj();
	Dict *f=newdict();
	r.type=DICT;
	r.dict=f;
	return r;
}

int dict_add(Dict *d, char *key, tOBJ value)
{
	tOBJ t1;

	if (d==NULL) return 0;

	if(dbg) printf ("Add %d %d\n", d->ip, d->sz);

	if ((d->ip) < (d->sz-1))
	{
		char *cp = d->db[d->ip].key;
		strncpy(cp, key, 32);

		//print(value);printf (" ... ");

		t1 = cloneObj(value);

		//print(t1);printf (" ...\n ");

		d->db[d->ip].value = t1; // should be clone;
		(d->ip)++;
	}
	return 0;
}	

int dict_find(Dict *d, char *key)
{
	int i=0;
	if (d==NULL) return -1;

	while (i<d->ip)
	{
		if (strcmp(key, d->db[i].key)==0)
		{
			return i;
		}
		i++;
	}
	return -1;
}

int dict_contains(Dict *d, char *key)
{
	if (d==NULL) return 0;
	return dict_find(d, key)>=0;
}

tOBJ dict_getk(Dict *d, char *key)
{
	int i=dict_find(d, key);

	if (i>=0) return d->db[i].value;
	return emptyObj();
}

tOBJ dict_get(Dict *d, int indx)
{
	if (indx >=0 && indx <d->ip)
		return d->db[indx].value;
	return emptyObj();
}


int dict_update(Dict *d, char *key, tOBJ value)
{
	int i;
	if (d==NULL) return 0;
	i=dict_find(d, key);
	if (i>=0) 
	{
		d->db[i].value = cloneObj(value); // clone it?
		return 1;
	}
	dict_add(d, key, value);
	return 0;
}

int dict_print(Dict *d)
{
	int i=0;

	if (d==NULL) return 0;

	while (i<d->ip)
	{
		tOBJ t;
		char *st="";
		t = d->db[i].value;

		switch (t.type)
		{  
		case INTGR: st="Int   "; break;

		case FLOAT: st="Float "; break;

		case STR:   st="String"; break;

		case FMAT2: st="Matrix"; break;

		case CELL:  st="List  "; break;

		case SYM:   st="Symbol"; break;

		case LAMBDA:st="Lambda"; break;

		case BOOLN: st="bool   ";break;

		case EMPTY: st="Empty  ";break;

		case FUNC:  break;
		}

		printf ("%7s [%s] ", d->db[i].key, st );

		if (t.type==LAMBDA)
		{
			t.type=CELL;
			print(t);
		}
		else
		{
			print (d->db[i].value);
		}
		printf ("\n");
		i++;
	}
	return 0;
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
	if (b->type==CELL)
	{
		if (b->cnt==0) delCell(b->cell);
	}
	if (b->type==DICT)
	{
		if (b->cnt==0) deldict(b->dict);
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

	if (*e == '{')
	{
		return CLIST;
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
		{
			push((*oplist[i].func)(pop()));
		}
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
				r = get(NULL,tokbuff);
				push(r);
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
			case CLIST:
				r = readcells();
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
						push((*oplist[op].func)(emptyObj()));
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

/* CELLS - linked list of objects
i
!{1 2 3}
!{"list" "of" "things"}
!{1 {2 4} 2 3 {4 {2}}}
*/
tOBJ readcells()
{
	int tf=1;
	tOBJ r= makeCell();
	tCELLp top = (tCELLp)r.cell; 
	tCELLp prv = (tCELLp)0;
	top->head = emptyObj();
	top->tail=0;
	e++;

	while (tf)
	{
		int tk = get_token(0);	
		tCELLp next;

		switch (tk)
		{
		case ALPHA:
			//top->head = get(tokbuff);
			top->head = makestring(tokbuff);
			top->head.type=SYM;
			break;
		case STRNG:
			top->head = makestring(tmpstr);
			break;
		case NUMI:
			top->head = makeint(tnum);
			break;
		case NUMF:
			top->head = makefloat(tfloat);
			break;
		case CLIST:
			top->head = readcells();
			break;
		case MLIST: 
			{
			int toop=oop;
			oop=0;
			top->head.type   = FMAT2;
			top->head.fmat2 = readmatrix(tmpstr);
			oop=toop;
			break;	
			}
		case OPR:
			{
				tOBJ o;
				int op = getOP(tokbuff);
				o.type=FUNC;
				o.func=oplist[op].func;

				if (o.func==NULL)
				{
					o=makestring(tokbuff);
				}
				top->head = o;

			}
			break;
		case DELI:
			if (strcmp(tokbuff,"}")!=0)
			{
				printf ("Invalid character [%s]\n",tokbuff);
			}
			if (prv !=NULL) prv->tail=0;
			tf=0;
			break;
		}

		if (tf)
		{
			next=newCell();
			next->head=emptyObj();
			top->tail=next;	
			prv=top;
			top=next;
		}
	}
	return r;
}

tOBJ stringtocells(char *s)
{
	tOBJ r=emptyObj();
	char savebuff[MAXSTRING];
	int toop;
	char *t;
	strncpy(savebuff, exprbuff, MAXSTRING); 
	toop=oop;
	t=s;

	e=s;
	r = readcells();

	strncpy(exprbuff, savebuff, MAXSTRING); 
	oop=toop;
	e=t;
	return r;
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

	while (e!=0 && *e != 0)
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

		if (*e==';' || *e==13 || *e==10)
		{
			//
			j++; i=0; e++; c++;
			if (*e==13 || *e==10) e++;
			v=eval_oxpr(e);	
			ts[c] = tofloat(v);
		}

		if (p!=0) e=p;

		if (dbg) printf ("[%d,%d] = %f  (%s)\n", i,j,ts[c], e);

		while (*e==' ') e++; 

		if (*e==';' || *e==13 || *e==10) { j++; i=0; e++; } else {i++;}

		while (*e==' ' || *e==13 || *e==10) e++; 

		c++;

		if (*e==']') break;
	}

	if (c != i*(j+1))
	{
		printf ("Bad matrix defintion %d,%d,%d ?\n", c, i,j);
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

void printtype(FILE *fp, tOBJ r)
{
    if (r.type == INTGR)
    {
            fprintf(fp, "%d", r.number);
    }
    else if (r.type == FLOAT)
    {
            fprintf(fp, "%f", r.floatpoint);
    }
    else if (r.type == STR)
    {
		char ch, *cp=r.string;
		while ( (ch=*cp++) != '\0')
		{
			if ((ch=='\\') && ((*cp)=='n') )
			{ 
				fputc(13,fp); 
				fputc(10,fp); 
				cp++;
				continue;
			}
			fputc(ch,fp);
		}
    }
    else if (r.type == SYM)
    {
            fputs(r.string,fp);
    }
    else if (r.type == BOOLN)
    {
        if ( r.number==0)
                fprintf(fp, "False");
        else
                fprintf(fp, "True");
    }
    else if (r.type == EMPTY)
    {
         fprintf(fp, "NIL");
    }
    else if (r.type == LAMBDA)
    {
         fprintf(fp, "LAMBDA");   
    }
    else if (r.type == FUNC)
    {
         fprintf(fp, "FUNCTION");   
    }
    else if (r.type == FMAT2)
    {
	fmatprint2(&r.fmat2);	 
    }
    else  if (r.type == DICT)
    {
	dict_print(r.dict);
    }
    else  if (r.type == CELL)
    {
	if (r.q)  fprintf(fp, "'");

	if (r.cell != NULL)
	{
		 fprintf(fp, "CELL");	
	}
	else
	{
		 fprintf(fp, "null");
	}
    }  
    else
    {               
         fprintf(fp, "? error - type\n");   
    }
    return;
}

tOBJ fprint(FILE *fp, tOBJ r)
{
    if (r.type == CELL)
    {
        struct cell  *c = r.cell;
        fprintf(fp,"{");  
        print(c->head);
                
        while (c->tail != (void *)0)
        {
                c=c->tail;
                rprintfStr(" ");
                print(c->head);
        }
        fprintf(fp,"}");
    }
    else
    {
		printtype(fp, r);
    }
    return r;
}

tOBJ print(tOBJ r)
{
	return fprint(stdout, r);
}

tOBJ println(char *s, tOBJ r)
{
	printf ("%s", s); print(r); printf("\n");
	return r;
}

/**********************************************************/
/*  Access variables (read/write)                         */
/**********************************************************/

tOBJ env;
int eif=0;

tOBJ initEnv()
{
	if (eif==0) {
		env = makedict();
	}
	return env;
}

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

	return dict_getk((Dict *)(env.dict), name);
}

int set(Dict * en, char *name, tOBJ r)
{
	int n = 0;

	if (en != NULL && dict_contains(en, name))
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

	n = dict_update((Dict *)(env.dict), name, r);
	return n;
}

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
	r.floatpoint=exp(tofloat(pop()));
	return r;
}

tOBJ osqrt(tOBJ a)
{
	//binary signmoid function
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=sqrt(tofloat(pop()));
	return r;
}

tOBJ ornd(tOBJ a)
{
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=(float)rand()/RAND_MAX;
	return r;
}

tOBJ omax(tOBJ a)
{
	tOBJ r=pop();
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
tOBJ omat(tOBJ a) 
{
	//binary signmoid function
	tOBJ r,m;
	int row,col;
	r=makefloat(0.0);
	row=toint(a);
	col=toint(pop());
	m=pop();

	if (m.type==FMAT2) 
	{
		r.floatpoint = fget2(&(m.fmat2),col,row);
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

tOBJ oapply(tOBJ a)
{
	//apply (<matrix>, <string>)
	tOBJ r,b, v;
	int i=0, j=0, w, h;
	char *expr;

	r.type=EMPTY;
	b=pop();

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
			v=eval_oxpr(expr);
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

tOBJ orshp(tOBJ a)
{	// RSHP(M,x,y)
	tOBJ r,m;
	int row,col;
	r.type=EMPTY;

	row=toint(a);
	col=toint(pop());

	m=pop();
	if (m.type==FMAT2)
	{
		r.type=FMAT2;
		r.fmat2 = fmatrshp(&m.fmat2, col,row);
	}
	return r;
}


tOBJ orep (tOBJ a)
{	// REP(M,x,y)
	tOBJ r,m;
	int row,col;
	r.type=EMPTY;

	row=toint(a);
	col=toint(pop());

	m=pop();
	if (m.type==FMAT2)
	{
		r.type=FMAT2;
		r.fmat2 = freplicate2(&m.fmat2, col,row);
	}
	return r;
}


tOBJ ozero (tOBJ a)
{
	tOBJ r;
	int row,col;
	r.type=EMPTY;

	row=toint(a);
	col=toint(pop());

	r.type=FMAT2;
	r.fmat2 = newmatrix(col, row);
	return r;
}

tOBJ oeye (tOBJ a)
{
	tOBJ r;
	int i;
	int row,col;
	r.type=EMPTY;

	row=toint(a);
	col=toint(pop());

	r.type=FMAT2;
	r.fmat2 = newmatrix(col, row);
		
	for (i=0; i<col; i++)
	{
		if (i<row) 
			r.fmat2.fstore[i+i*col] = 1.0;
	}
	return r;
}

tOBJ oconv (tOBJ b)
{
	tOBJ r, a;

	r.type=EMPTY;

	a=pop();

	if (a.type != FMAT2 && b.type!=FMAT2)
		return r;

	r.type=FMAT2;
	r.fmat2=fconvolve2(&a.fmat2,&b.fmat2) ;
		
	return r;
}

tOBJ ocond(tOBJ a)
{
	// COND (Matrix,lv, uv, nv1, nv2)
	tOBJ r;
	int i;
	float lv=0,uv=0,nv1=0,nv2=0;
	nv2 = tofloat(a);
	nv1 = tofloat(pop());
	uv = tofloat(pop());
	lv = tofloat(pop());
	a = pop();
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

tOBJ ozerob(tOBJ a)
{
	// ZER (Matrix,c1,r1,c2,r2)
	tOBJ r;
	int c1=0,c2=0,r1=0,r2=0;
	r2 = toint(a);
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

tOBJ oimp (tOBJ c)
{
	//IMP("A",2,2);

	tOBJ r, a, b;

	r.type=EMPTY;

	b=pop(); a=pop();

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

tOBJ ecar (tOBJ a)
{
	return ocar(ocar(a));
}

tOBJ ecdr (tOBJ a)
{
	return ocdr(ocar(a));
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

tOBJ olist (tOBJ a)
{	
	//!LIST {1 2 3} -> {{1 2 3}}
	tOBJ r=makeCell2(a, NULL);	
	return r;
}

tOBJ ocons (tOBJ n)
{
	//!CONS {1 {2 3}} -> {1 2 3}
	tOBJ r=emptyObj();
	if (n.type==CELL)
	{
		tOBJ a = ocar(n);
		tOBJ b = ocar(ocdr(n));
		if (b.type==CELL)
		{
			r=makeCell2(a, b.cell);
		}
	}
	return r;
}

tOBJ oapp (tOBJ a)
{
	//!APPD {{1 2} {3 4}} -> {1 2 3 4}
	tOBJ r=emptyObj();
	tOBJ n;

	if (a.type==CELL)
	{
		tCELLp p= a.cell;
		r=makeCell();
		n=r;
		do
		{
			tOBJ h=p->head;
			tCELLp l;
			if (h.type != CELL)
			{
				printf("not a list?\n");
				break;
			}
			l=h.cell;
			do //add each elemnt to list
			{
				tCELLp z=n.cell;
				z->head=l->head;
				l=l->tail;
				if (l!=NULL || p->tail != NULL) 
				{
					n=makeCell();
					z->tail=n.cell;	
				}
				else
				{
					z->tail=NULL;
				}
			}
			while (l != NULL);		
			p=p->tail;
		}
		while (p !=NULL);
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
	return (makeint(a.type==CELL));
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



/**********************************************************/
/* SERVO routines                                        */
/**********************************************************/

extern BYTE cpos[];
extern BYTE	nos;


tOBJ oserv(tOBJ a)
{
	//> !SERV
	tOBJ r;
	readservos();
	r=cnvtBytetoList(nos, cpos);	
	return r;
}

int cnvtListtoByte(tOBJ lst, int an, BYTE *array)
{
	int cnt=0;
	if (lst.type != CELL) return 0;
	while (cnt<an && onull(lst).number==0)
	{
		array[cnt++] = toint(ocar(lst));
		lst=ocdr(lst); 
	}
	
	return cnt;
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

tOBJ callfn(tOBJ  fn, tOBJ x);

tOBJ exec(tOBJ a, Dict *e)
{
	//!EXEC {CDR {1 2 3 4}}
	//!EXEC {FM {1}}
	//!EXEC {+ 1 2}
	//!EXEC {PR 1 2 3 }
	//!LET ZZ={1 2 3 4}
	//!CDR ZZ
	//!EXEC {CDR ZZ}

	if (dbg) println("exec=",a);

	if (a.type==CELL)
	{
		tOBJ r=emptyObj();
		tOBJ fn=ocar(a);

		tOBJ arg=ocdr(a);

		if (dbg) println("fn=",fn); 
		if (dbg) println("arg=",arg);

		tOBJ param=emptyObj();
		tOBJ prev;
		tCELLp pn=NULL;
		while (arg.type != EMPTY)
		{
			tOBJ a1=ocar(arg);
			tOBJ b1=exec(a1, e);
			param = makeCell2(b1, pn);
			pn=param.cell;
			arg=ocdr(arg);
		}

//println("param=",param);
		arg=orev(param);
//println("arg=",arg);		
		

		if (fn.type == SYM)
		{
			fn = get(e, fn.string);
		}
		if (fn.type==FUNC)
		{
			return (*fn.func)(arg);
		}
		else if (fn.type==STR)
		{
			int op=PLUS;
			tOBJ o1=ocar(arg);
			tOBJ o2=ocar(ocdr(arg));

			if (fn.string[0]=='+') op=PLUS;
			return omath(o1, o2, op);
		}
		else if (fn.type==LAMBDA)
		{
			return callfn(fn,arg);
		}
	}
	if (a.type==SYM)
	{
		a = get(e, a.string);
	}
	return a;
}

tOBJ callfn(tOBJ  fn, tOBJ x)
{
	tOBJ r, arg, body;
	tOBJ e= makedict();

	if (dbg) printf ("lambda\n");
	fn.type=CELL;
	arg=ocar(fn);
	body=ocdr(fn);

	if (dbg) println("args=",x);

	//bind ags and params
	x=ocar(x);

	while (onull(arg).number==0)
	{
		tOBJ n=ocar(arg);
		tOBJ v=ocar(x);

		if (n.type==STR || n.type==SYM)
			dict_add((Dict*)e.dict, n.string, v);
		arg=ocdr(arg);
		x=ocdr(x);
	}
	if (dbg) dict_print((Dict *)e.dict);

	r=emptyObj();
	if (body.type==CELL)
	{
		do {
		x = ocar(body);
		r = exec(x, (Dict *)e.dict);
		body = ocdr(body);
		} while (onull(body).number==0);
	}	

	return r;
}

tOBJ odefn(tOBJ  r)
{
/*   !DEFN {"n" {args} {body}}
i
!DEFN {FM {x} {PR X "+" 1 " = " {+ x 1}}}
!EXEC {FM {1}}
*/
	tOBJ fn   = ocar(r);
	tOBJ fbdy  = ocdr(r);
	fbdy.type = LAMBDA;
	set(NULL, fn.string, fbdy);
	return emptyObj();
}

tOBJ oexec(tOBJ a)
{
	return exec(a,NULL);
}


tOBJ oload(tOBJ  n)
{
	//!LOAD "fn"
	// file start with [ is a matrix (note: no ']')
	// file start with { x } is a list
	// else string
	tOBJ r=emptyObj();
       	FILE *fp;
	int i,t;
	char *s = n.string;
	int cn=0;
	int sz=1024;
	char *m=malloc(sz);
	int ch;

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
		m[cn-1]=0;	
		r.type=FMAT2;
		r.fmat2 = readmatrix(&m[1]);
		break;
	case '{' : 
		r = stringtocells(m); 
		break;
	default:
		r = makestring(m);
		break;
	}
	return r;

}


tOBJ osave(tOBJ  n)
{
	//!SAVE {"fn" object}
	tOBJ f=ocar(n);
	tOBJ r=ocar(ocdr(n));

       	FILE *fp;
	int i,t;
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

tOBJ oqt(tOBJ  r)
{
	//!QT {}
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

Extended input

************************************************************************/

tOBJ omatr(tOBJ a)
{
	tOBJ n=pop();
	tOBJ v=pop();
	int val;
	// {"LOAD" "STOR"}
	if (v.type==STR && n.type==INTGR)
	{
		if (!strcmp(v.string,"LOAD"))
		{
			val=toint(n);
			v=get(NULL, "DFN");
			matrixload(val, v.string);
		}
		if (!strcmp(v.string,"STOR"))
		{
			val=toint(n);
			v=get(NULL, "DFN");
			matrixstore(val, v.string);
		}
	}
	return (emptyObj());
}

tOBJ oimg(tOBJ v)
{
	//!IMG {"command" {parameters}}
	//comand : {"UNLO" "LOAD" "FILT" "RAW" "THRE" "COLO" "PROC" "REG" "SHOW" "DEBU"}
	
	tOBJ cmd = ocar(v);
	if (cmd.type == STR)
	{
		//tOBJ l = stringtocells("");
	}
	return (emptyObj());
}

static int  intf=1;

int get_str_token(char *s)
{
	int tk = get_token(0);
	
	if (s==(char *)0 && tk==ALPHA)
		return 1;

	if (tk==ALPHA && !strcmp(tokbuff,s))
		return 1;
	return 0;
}



void extend(char *x)
{
	tOBJ v,file;

	if (intf) 
	{ 	// set up defaults
		intf=0; 

		//set up glnal environment
		initEnv();

		set(NULL, "PI",  makefloat (3.1415926));
		set(NULL, "DFN", makestring("data.txt"));
		set(NULL, "IFN", makestring("test.jpg"));
		
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
				set(NULL, var,v);
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
				R=get(NULL, var);
				if (R.type == FMAT2)
				{
					if (dbg) printf ("DBG var=%s %d,%d, %f\n",var,c,r,f);
					fset2(&R.fmat2,c,r,f);
				}
				return;
			}
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
			file = get(NULL, "IFN");	
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

			file = get(NULL, "IFN");
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
			file = get(NULL, "IFN");
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
				set(NULL, "CID", v);
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

	e=x;
	v=eval_oxpr(e);
	print(v);
	rprintfCRLF();
	freeobj(&v);
}



