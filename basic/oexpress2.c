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

#include "trex.h"

#ifdef _UNICODE
#define trex_sprintf swprintf
#else
#define trex_sprintf sprintf
#endif

extern tOBJ ocar(tOBJ  r);
extern tOBJ ocdr(tOBJ  r);
extern tOBJ emptyObj();
extern tOBJ print(tOBJ r);
extern tOBJ makeCell();
extern tOBJ makeCell2(tOBJ a, tCELLp b);
extern tOBJ readcells();
extern tOBJ cloneObj(tOBJ z);

extern tOBJ makestring(char *s);
extern void printtype(FILE *fp, tOBJ r);

extern tOBJ makedict ();
extern int  dict_find(Dict *d, char *key);
extern int  dict_add (Dict *d, char *key, tOBJ value);
extern int  dict_update(Dict *d, char *key, tOBJ value);
extern tOBJ dict_getk  (Dict *d, char *key);


extern int toint(tOBJ v);
extern int getOP(char *str);

typedef struct ops {
        char	*name;
		int		level;
		unsigned char type;
		int		nop;
		tOBJ	(*func)(tOBJ);
} tOP, *tOPp;

extern tOP oplist[] ;
extern int dbg;

//> !!( CAR 1 2 BC )
//[(] [CAR] [1] [2] [BC] [)] 

void pp(tOBJ x, int n)
{
	int i;
	while (x.type != EMPTY)
	{
		tOBJ e = ocar(x);
		x=ocdr(x);
		for (i=0;i<n; i++) printf (" "); print(e); printf("\n");
		if (e.type==CELL)
			pp(e,n+2);
	}
}

char *readword(char *s, char *w)
{
	char c;
	int n=0;
	int sf=0;
	while (*s != '\0')
	{
		c=*s++;
		if (c== '"')
		{
			if (sf==0)
			{
				sf=1;
				*w++ = c;
				continue;
			}
			else
			{
				break;
			}
		}
		if ( strchr("+-*/=:,;[]{}() ",c)>0  && sf==0)
		{
			if (c != ' ' && n==0) 
				*w++=c;
			if (c != ' ' && n>0) 
				s--;
			if (c== ' ' && n==0) 
				continue;
			break;
		}
		*w++ = c;
		n++;
	}
	*w='\0';
	return s;
}

int isnum(char *s)
{
	if (*s=='+' || *s=='-') s++;
	while (*s !=0)
	{
		if ((*s<'0' || *s>'9') && *s!='.')
			return 0;
		s++;			
	}
	return 1;	
}

tOBJ makenumfstr(char *s)
{
	tOBJ r;
	r.type=EMPTY;
	int n=0;
	float f=0.0, rm=1.0;
	int sgn=1;
	int flt=0;
	if (*s == '-') {sgn=-1; s++;}

	while (*s != '\0')
	{
		if (*s=='.' && flt==0) 
		{
			s++;
			flt=1;
			f=n;
		}
		if (*s<'0' || *s>'9') 
			break;

		if (flt==0)
		{
			n=n*10+*s-'0';
		}
		else
		{
			rm=rm/10;
			f=f+rm*(*s-'0');
		}
		s++;
	}

	if (flt==1)
	{
		r.floatpoint=f*sgn;
		r.type=FLOAT;
	}
	else
	{
		r.number=n*sgn;
		r.type=INTGR;
	}
	return r;
}

tOBJ tokenise(char *s)
{
	char buffer[1024];
	int n;
	int tf=1;

	tOBJ r= makeCell();
	tCELLp top = (tCELLp)r.cell; 
	tCELLp prv = (tCELLp)0;
	top->head = emptyObj();
	top->tail=0;

	while (*s != '\0')
	{		
		tCELLp next;
		s = readword(s, buffer);

		if (isnum(buffer))
		{
			top->head = makenumfstr(buffer);
		}
		else
		{
			if (buffer[0]=='"')
			{
				top->head = makestring(buffer+1);
			}
			else
			{
				top->head = makestring(buffer);
				top->head.type=SYM;
			}
		}

		top->tail=NULL;	

		if (tf && s != NULL && *s != '\0')
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


tOBJ append(tOBJ a, tOBJ b)
{
	// {} A  -> {A}
	// {B} A -> (B A}
	// B A   -> NILL
	//!!(CAR (1 2) 3 4)

	tOBJ r;	
	tCELLp m,n,p;

	if (a.type == EMPTY) 
	{
		r=makeCell();
		((tCELLp)(r.cell))->head = cloneObj(b);
		((tCELLp)(r.cell))->tail = NULL;
		return r;
	}

	if (a.type != CELL)
		return emptyObj();
	
	r=cloneObj(a);
	
	// walk to end
	n=NULL;
	p = r.cell;

	while (p!= NULL)
	{
		n=p;
		p=p->tail;
	}

	// add 'b'

	m = newCell();
	n->tail=m;
	m->head = cloneObj(b);
	m->tail = NULL;

	return r;
}

tOBJ read_from(tOBJ *r)
{
	tOBJ h, L = emptyObj();

	while (r->type!=EMPTY)
	{
		h=ocar(*r);
		*r=ocdr(*r);

		if (h.type==SYM && !strcmp(h.string, "{"))
		{			
			L=append(L, read_from(r));
		}
		else
		if (h.type==SYM && !strcmp(h.string, "}"))
		{
			break;
		}
		else
			L=append(L,h);	
	}
	return L;
}

tOBJ parse(char *s)
{
	tOBJ L,r;	
	if (dbg) printf ("parse 2 %s\n", s); 
	r = tokenise(s);
	L = read_from(&r);	
	if (dbg) pp(L,0);
	return L;
}

tOBJ eval(tOBJ o, Dict *e);
extern tOBJ omath(tOBJ o1, tOBJ o2, int op);

tOBJ formula(tOBJ o, Dict *e)
{
	tOBJ r=eval(o, e);
/*
	// ( V op V )
	// V op V op V
	// F (arg)
	// 
	tOBJ r=emptyObj();
	while (o.type != EMPTY)
	{
		tOBJ n=ocar(o);
		o=ocdr(o);

		//is it a function?

		//is it an op ?

		//recursive descent		

		if (n.type==STR && !strcmp(n.string,"(")))
		{
		}

		// end and return

		if (n.type==STR && !strcmp(n.string,")")))
		{
		}

		//

	}
*/
	return r;
}

tOBJ eval(tOBJ o, Dict *e)
{
	tOBJ r=emptyObj();
	tOBJ h;

	if (dbg) {printf ("eval - ");printtype(stdout, o); printf (" - "); print (o); printf ("\n");}

	if (o.type == EMPTY)
	{
	 	return o;
	}

	if (o.type==INTGR || o.type==FLOAT || o.type==STR)
		return o;


	if (o.type==SYM)
	{
		return dict_getk(e, o.string);
	}

	h=ocar(o);
	o=ocdr(o);

	if (h.type==INTGR || h.type==FLOAT) //?
		return h; //?

	if (h.type==SYM)
	{

	if (dbg) {printf ("eval - ");printtype(stdout,h); printf (" - "); print (h); printf ("\n");}

		if (!strcmp(h.string,"QT"))
		{
			return ocar(o);
		}
		else
		if (!strcmp(h.string,"IF"))
		{
			tOBJ test   = ocar(o); o=ocdr(o);
			tOBJ conseq = ocar(o); o=ocdr(o);
			tOBJ alt    = ocar(o); 
			if (toint(eval(test,e))==1)
			{
				return eval(conseq,e);
			}
			else
			{
				return eval(alt,e);
			}
		}
		else
		if (!strcmp(h.string,"SET!"))
		{
			tOBJ var = ocar(o); o=ocdr(o);
			tOBJ exp = ocar(o); 
			dict_update(e, var.string, exp);
		}
		else
		if (!strcmp(h.string,"LET"))
		{
			//LET VAR = FORMULA
			tOBJ var = ocar(o); o=ocdr(o);
			tOBJ eq =  ocar(o); o=ocdr(o);
			if (eq.type != STR && strcmp(eq.string,"=")) {printf ("err\n"); return emptyObj();}
			tOBJ form = ocar(o); 
			eq = formula(ocar(o),e);
			dict_update(e, var.string, eq);
			return eq;
		}
		else
		if (!strcmp(h.string,"PLUS"))
		{
			//LET VAR = FORMULA
			tOBJ var1 = ocar(o); o=ocdr(o);
			tOBJ var2 =  ocar(o); 
			return omath(eval(var1,e), eval(var2,e), 1);
		}
		else
		if (!strcmp(h.string,"PR") || !strcmp(h.string,"PRINT"))
		{
			while (o.type != EMPTY)
			{
				tOBJ exp = ocar(o); o=ocdr(o);	
				r=eval(exp,e);
				print(r);
			}
			return r;
		}
		else
		if (!strcmp(h.string,"DEF"))
		{
			tOBJ var = ocar(o); o=ocdr(o);
			tOBJ exp    = ocar(o); 
			dict_update(e, var.string, eval(exp,e));
		}
		else
		if (!strcmp(h.string,"LAM"))
		{
			//tOBJ vars = ocar(o); o=ocdr(o);
			//tOBJ exp  = ocar(o); 
			//define lambda (a) b procedure
			//call
		}
		else
		if (!strcmp(h.string,"BEG"))
		{
			while (o.type != EMPTY)
			{
				tOBJ exp = ocar(o); o=ocdr(o);	
				r=eval(exp,e);
			}
			return r;
		}
		else
		{
			//lookup procedure
			int op = getOP(h.string);
			if (op>=0)
			{
				//call
				switch(oplist[op].nop)
				{
				case 0:
					return (*oplist[op].func)(emptyObj());
				case 1:	
					//return (*oplist[op].func)(o);
					return (*oplist[op].func)(eval(ocar(o),e));
				default:
					break;
				}
			}
			printf ("? unknown symbol [%s]\n", h.string);
		}
	}
	return r;
}

extern tOBJ env;

void extend2(char *x)
{
	tOBJ v,file;

	v=eval(parse(x), env.dict);

	printf (" = ");print(v);printf ("\n");
}

