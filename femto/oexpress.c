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
#include <strings.h>
#include <math.h>
#include <stdlib.h> 
#include <unistd.h>
#include <time.h>
#include "linux.h"
#endif

#ifdef AVR
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h> 
#include <avr/interrupt.h>
#include "rprintf.h"
#endif


#include "oobj.h"
#include "ocells.h"
#include "ostring.h"
#include "odict.h"
#include "ofunction.h"

#include "oexpress.h"

#include "femto.h"


char *readword(char *s, char *w)
{
       char c;
       int n=0;
       int sf=0;
       while (*s != '\0')
       {
              c=*s++;
              if (c==13 || c==10 || c==9) continue; // ignore tabs

              if (c== '"')
              {
                     if (sf==0)
                     {
                            sf=1;
                            *w++ = c;
                            continue;
                     }
                     else
                            break;
              }

              if ( strchr("`'*/:,;[]{}() ",c)>0  && sf==0)
              {
                     if (c != ' ' && n==0)
                     {
                            *w++=c;
                            while (*s==13 || *s==10 || *s==' ') s++; //skip trailing w/s
                     }
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
	if ((*s=='+' || *s=='-') && *(s+1)!=0)  s++;
	while (*s !=0)
	{
		if ((*s<'0' || *s>'9') && *s!='.')
			return 0;
		s++;			
	}
	return 1;	
}

/*
!setq  MC [2.0 3.6;2.7 1.2]
!setq  mx (+ [1.0 2.0;3.0 4.0] [2.0 3.6;2.7 1.2])
!let ms = mx + mc
!print (* [1.0 2.0;3.0 4.0] [2.0 3.6;2.7 1.2])
!print (trn [1.0 2.0 3.0 4.0])
!print (trn [1.0 2.0;3.0 4.0])
!PRINT (* [1.0 2.0;3.0 4.0] [1.0 2.0;3.0; 4.0])
!PRINT (* [1.0 2.0;3.0 4.0] [1.0 2.0;3.0 4.0])
!PRINT (* [1.0 2.0;3.0 4.0] 0.2)
*/

fMatrix *readmatrix(char **str )
{
	fMatrix *m;
	float ts[100];
	char *s = *str;
	char buffer[MAXSTRING];
	tOBJ v;
	int i=0;
	int j=0;
	int c=0;

	while (s!=0 && *s != 0)
	{
		s = readword(s, buffer);

		if (!strcmp(buffer,"]")) 
			break;

		if (!strcmp(buffer,";")) 
		{
			i=0; j++;
			continue;
		}

		v = makenumfstr(buffer);
		ts[c] = tofloat(v);
		c++;
		i++;
	}

	if (c != i*(j+1))
	{
		rprintf ("Bad matrix defintion %d,%d,%d ?\n", c, i,j);
		return NULL;
	}

	//create matrix

	m = newmatrix(i, j+1);
	
	//copy in ts[0-c];

	for (i=0; i<c; i++) m->fstore[i]=ts[i];

	*str=s;
	return m;
}

tOBJ tokenise(char *s)
{
	char buffer[255];

	tOBJ r= makeCell();
	tCELLp top = (tCELLp)r.cell; 
	top->head = emptyObj();
	top->tail=0;

	while (*s != '\0')
	{		
		s = readword(s, buffer);

		if (buffer[0]== 0) 
		{
			continue;
		}
		else
		if (isnum(buffer))
		{
			top->head = makenumfstr(buffer);
		}
		else
		{
			if (buffer[0]=='"')
			{
				top->head = makestring(buffer+1);
				top->head.q=1;
			}
			else
			if (buffer[0]=='[')
			{
				top->head.type=FMAT2;	
				top->head.fmat2=readmatrix(&s);			
			}
			else{
				top->head = makestring(buffer);
			}
		}

		top->tail=NULL;	

		if (s != NULL && *s != '\0')
		{
			tCELLp next=newCell();
			next->head=emptyObj();
			top->tail=next;	
			top=next;
		}	
	}

	return r;
}

tOBJ read_from(tOBJ *r)
{
	tOBJ h, n, L= emptyObj();
	int qf=0;

	while (r->type!=EMPTY)
	{
		h=ocar(*r);
		*r=ocdr(*r);

		if (h.type==SYM && h.q==1)
		{	
			qf=1;	
		}

		if (h.type==SYM && !strcmp(h.string, "'"))
		{	
			qf=1;	
		}
		else
		if (h.type==SYM && !strcmp(h.string, "("))
		{	
			tOBJ t=read_from(r);
	
			t.q=qf;	
			n=append(L,t );
			qf=0;
			freeobj(&t);
			freeobj(&L);
			L=n;
		}
		else
		if (h.type==SYM && !strcmp(h.string, ")"))
		{
			break;
		}
		else
		{
			h.q=qf;	qf=0;
			n=append(L,h);
			freeobj(&L);
			L=n;	
		}
	}
	return L;
}

tOBJ parse(char *s)
{
	tOBJ L,r,t;	
	r = tokenise(s);
	t = r;
	L = read_from(&r);
	freeobj(&t);	
	return L;
}

extern int retflg;
tOBJ callfn(tOBJ  fn, tOBJ x, Dict *env)
{
	tOBJ arg, body;
	tOBJ e= makedict2(env,0);

	fn.type=CELL;
	arg=ocar(fn);
	body=ocdr(fn);

	while (onull(arg).number==0)
	{
		tOBJ n=ocar(arg);
		tOBJ v;

		if (oatom(x).number)
		{
			v=eval(x,env);
		}
		else
		{
			v=eval(ocar(x),env);
		}

		if (n.type==SYM)
		{
			dict_add((Dict*)e.dict, n.string, v);
		}

		arg=ocdr(arg);
		x=ocdr(x);

		freeobj(&v);
	}
	if (dbg) dict_print((Dict *)e.dict,1);

	retflg=0;
	arg = obegin(body,(Dict *)e.dict);
	retflg=0;

	if (arg.cnt>0)
	{
		arg=cloneObj(arg);
	}

	freeobj(&e);

	return arg;
}

tOBJ procall (tOBJ h, tOBJ o, Dict *e )
{
	tOBJ r;
	int f=0;
	if (h.type==EMPTY) return h;

	if (h.type==SYM && dict_contains(e, h.string))
	{
		h=dict_getk(e,h.string);
		f=1; 
	}

	if (h.type==LAMBDA)
	{
		r= callfn(h, o, e);
		if (f) freeobj(&h);
		return r;
	}

	if (h.type==FUNC)
	{
		tOPp p = (tOPp)h.fptr;
		if (p->type != NA)
		{
			tOBJ a = eval(ocar(o),e); o=ocdr(o);
			tOBJ b = eval(ocar(o),e); 					
			tOBJ r = omath(a,b,p->type);
			freeobj(&b);
			freeobj(&a);
			return r;
		}
		else//call
		switch(p->nop)
		{
		case 0:
			return (*p->func)(emptyObj());
		case 1:	{
			tOBJ a=eval(ocar(o),e);
			r=(*p->func)(a);
			freeobj(&a);
			return r;
			}
		case 2:	
			{
			tOBJ a = eval(ocar(o),e); o=ocdr(o);
			tOBJ b = eval(ocar(o),e); 
			r = (*p->func2)(a,b);
			freeobj(&b);
			freeobj(&a);
			return r;
			}
		case 3:	
			{
			tOBJ a = eval(ocar(o),e); o=ocdr(o);
			tOBJ b = eval(ocar(o),e); o=ocdr(o);
			tOBJ c = eval(ocar(o),e); 
			r = (*p->func3)(a,b,c);
			freeobj(&c);
			freeobj(&b);
			freeobj(&a);
			return r;
			}
		case 5:	
			{
			tOBJ a = eval(ocar(o),e); o=ocdr(o);
			tOBJ b = eval(ocar(o),e); o=ocdr(o);
			tOBJ c = eval(ocar(o),e); o=ocdr(o);
			tOBJ d = eval(ocar(o),e); o=ocdr(o);
			tOBJ g = eval(ocar(o),e); 
			r= (*p->func5)(a,b,c,d,g);
			freeobj(&g);
			freeobj(&d);
			freeobj(&c);
			freeobj(&b);
			freeobj(&a);
			return r;
			}
		default:
			return (*p->funce)(o,e);
			break;
		}		
	}

	if (h.type==SYM)
		rprintf ("? unknown symbol [%s]\n", h.string);
	else
		rprintf ("? unknown (%s)\n",objtype(h));

	if (f) freeobj(&h);

	return emptyObj();
}

tOBJ eval(tOBJ o, Dict *e)
{
	tOBJ h;

	if (o.q==1 || o.type==INTGR || o.type==FLOAT || o.type == FMAT2 || o.type == EMPTY ||o.type==FUNC || o.type==LAMBDA|| o.type==STACK)
	{
		o.q=0;
		if (o.type==SYM)
			return cloneObj(o);

		if (o.type==CELL || o.type==FMAT2 || o.type==STACK)
			o.cnt++;

		return o;
	}

	if (o.type==SYM)
	{
		tOBJ a=dict_getc(e, o.string);
		if (a.type==CELL)
			return a;
		else	
			return cloneObj(a);
	}

	return procall (ocar(o), ocdr(o), e );	//lookup procedure
}


tOBJ env;
static int  intf=1;

void extend(char *s);

extern int getopsize();

void init_extend()
{
	if (intf)
	{
		intf=0; 
		env = makedict(getopsize() + 10); 
		loadop(env.dict);

		set(env.dict, "PI",  makefloat (3.1415926));	
		//seed the RND Gen
		//srand ( (unsigned)time ( NULL ) );

		//extend("DEF FACT (N) (IF (<= N 1) 1 (* N (FACT (- N 1))))");
	}
}


#define MAX 100
char buffer[MAX];

void extend(char *s)
{
	init_extend();
	tOBJ e=parse(s);
	tOBJ v = eval(e, env.dict);

	println (" = ",v);

	freeobj(&v);
	freeobj(&e);
}


void repl()
{
	init_extend();
	while (1)
	{
		buffer[0]=0;
		readLine(&buffer[0], MAX);
		if (buffer[0]==';') continue;
		extend(buffer);
	}
}


