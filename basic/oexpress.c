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

#include "mem.h"
#include "oobj.h"
#include "ocells.h"
#include "ostring.h"
#include "odict.h"
#include "ofunction.h"
#include "oexpress.h"


extern int readLine(char *line); //basic.c

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

void *readmatrix(char **str, char tc )
{
	float ts[10000];
	char *s = *str;
	char buffer[MAXSTRING];
	tOBJ v;
	int i=0;
	int j=0;
	int c=0;

	while (s!=0 && *s != 0)
	{
		s = readword(s, buffer);

		if (buffer[0]==tc && buffer[1]==0) 
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
		printf ("Bad matrix defintion %d,%d,%d ?\n", c, i,j);
		return NULL;
	}

	if (tc ==']')
	{
		//create matrix
		fMatrix *m = newmatrix(i, j+1);	
		//copy in ts[0-c];
		for (i=0; i<c; i++) m->fstore[i]=ts[i];
		*str=s;
		return m;
	}

	if (tc =='}')
	{
		//create matrix

		oImage *m = makeimage(j+1, i);
	
		//copy in ts[0-c];

		for (i=0; i<c; i++) m->data[i]=(int)(ts[i]+0.5);
		*str=s;
		return m;
	}
	return NULL;
}

tOBJ tokenise(char *s)
{
	char buffer[1024];

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
				top->head.fmat2=(fMatrix *)readmatrix(&s, ']');			
			}
			else
			if (buffer[0]=='{')
			{
				top->head.type=IMAG;	
				top->head.imgptr=(oImage *)readmatrix(&s, '}');			
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
	if (dbg) printf ("\nparse  %s\n", s); 
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

	if (dbg) {println("Lambda args=",x); println("values=",arg); }

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
		if (p->type != NA && p->type != CBR && p->type != OBR)
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
			return (*p->foo.func)(emptyObj());
		case 1:	{
			tOBJ a=eval(ocar(o),e);
			r=(*p->foo.func)(a);
			freeobj(&a);
			return r;
			}
		case 2:	
			{
			tOBJ a = eval(ocar(o),e); o=ocdr(o);
			tOBJ b = eval(ocar(o),e); 
			r = (*p->foo.func2)(a,b);
			freeobj(&b);
			freeobj(&a);
			return r;
			}
		case 3:	
			{
			tOBJ a = eval(ocar(o),e); o=ocdr(o);
			tOBJ b = eval(ocar(o),e); o=ocdr(o);
			tOBJ c = eval(ocar(o),e); 
			r = (*p->foo.func3)(a,b,c);
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
			r= (*p->foo.func5)(a,b,c,d,g);
			freeobj(&g);
			freeobj(&d);
			freeobj(&c);
			freeobj(&b);
			freeobj(&a);
			return r;
			}
		default:
			return (*p->foo.funce)(o,e);
			break;
		}		
	}

	if (h.type==SYM)
		printf ("? unknown symbol [%s]\n", h.string);
	else
		printf ("? unknown (%s)\n",objtype(h));

	if (f) freeobj(&h);

	return emptyObj();
}

tOBJ eval2(tOBJ o, Dict *e)
{
	if (o.q==1 || o.type==INTGR || o.type==FLOAT || o.type == FMAT2 || o.type == EMPTY ||o.type==FUNC || o.type==LAMBDA|| o.type==STACK || o.type == IMAG)
	{
		o.q=0;
		if (o.type==SYM)
			return cloneObj(o);

		if (o.type==CELL || o.type==FMAT2 || o.type==STACK || o.type==IMAG)
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

tOBJ eval(tOBJ o, Dict *e)
{
	if (dbg) {bas_mem(); println ("eval - ",o); }

	tOBJ r=eval2(o,e);

	if (dbg) { bas_mem(); println ("ret  - ",r);}
	return r;
}

tOBJ env;
static int  intf=1;

tOBJ eval_oxpr(char *s);
void extend(char *s);

void init_extend(char *fn)
{
	if (intf)
	{
		if (fn==NULL) fn="boot.l";
		intf=0; 
		env = makedict(200);
		loadop(env.dict);

		if (dbg==0)
		{
			set(env.dict, "PI",  makefloat (3.1415926));	
			//seed the RND Gen
			srand ( (unsigned)time ( NULL ) );

			extend("FUNC FACT (N) (IF (<= N 1) 1 (* N (FACT (- N 1))))");

			if (access(fn,F_OK) != -1)
			{
				oload(makestring(fn));
			}
		}
	}
}

tOBJ eval_oxpr(char *s)
{
	init_extend(NULL);
	tOBJ e=parse(s);
	tOBJ f = eval(e, env.dict);
	freeobj(&e);
	return f;
}

void extend(char *s)
{
	init_extend(NULL);
	int tc=memlc;
	tOBJ e=parse(s);
	tOBJ v = eval(e, env.dict);
	println (" = ", v);
	freeobj(&v);
	freeobj(&e);
#ifdef MEM_DEBUG
printf("-- used %d (%d) \n", memlc-tc,mem_counter);
#endif
}

int countb(char *s)
{
	char c;
	int n=0;
	int sf=0;

	while ( (c=*s++) != 0)
	{
		if (c=='"')        sf=!sf;
		if (c=='(' && !sf) n++;
		if (c==')' && !sf) n--;		

	}
	return n;
}

#define MAX 255
void repl()
{
	char inputbuffer[MAX];

	if (dbg) { testme(0); 	sigcatch();  }
	//if (dbg) { testme(17); 	sigcatch();  }

	init_extend(NULL);
	while (1)
	{
		printf("!");
		inputbuffer[0]=0;
		readLine(&inputbuffer[0]);
		if (countb(inputbuffer)!=0)
		{
			printf("Mis-matched brackets\n");
			continue;
		}
		if (inputbuffer[0]==';') continue;
		if (!strcasecmp(inputbuffer,".")) return;
		extend(inputbuffer);
	}
}


