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


#include "oobj.h"
#include "ocells.h"
#include "ostring.h"
#include "odict.h"
#include "ofunction.h"

#include "oexpress.h"


extern int dbg;
extern int readLine(char *line);

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
		printf ("Bad matrix defintion %d,%d,%d ?\n", c, i,j);
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

if (dbg) printf ("\nend of input\n\n");

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
	//freeobj(&h);
if (dbg) printf ("\nend of read\n\n");

	return L;
}

tOBJ parse(char *s)
{
	tOBJ L,r,t;	
	if (dbg) printf ("\nparse 2 %s\n", s); 
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
		tOBJ n=ocar(arg);tOBJ v;

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
	}
	if (dbg) dict_print((Dict *)e.dict,1);

	retflg=0;
	arg = obegin(body,(Dict *)e.dict);
	retflg=0;

	return arg;
}

tOBJ procall (tOBJ h, tOBJ o, Dict *e )
{
	if (h.type==EMPTY) return h;

	if (h.type==SYM && dict_contains(e, h.string))
	{
		h=dict_getk(e,h.string);
	}

	if (h.type==LAMBDA)
	{
		return callfn(h, o, e);
	}

	if (h.type==FUNC)
	{
		tOPp p = (tOPp)h.fptr;
		if (p->type != NA && p->type != CBR && p->type != OBR)
		{
			tOBJ a = ocar(o); o=ocdr(o);
			tOBJ b = ocar(o); 					
			return omath(eval(a,e),eval(b,e),p->type);
		}
		else//call
		switch(p->nop)
		{
		case 0:
			return (*p->func)(emptyObj());
		case 1:	
			return (*p->func)(eval(ocar(o),e));
		case 2:	
			{
			tOBJ a = ocar(o); o=ocdr(o);
			tOBJ b = ocar(o); 
			return (*p->func2)(eval(a,e),eval(b,e));
			}
		case 3:	
			{
			tOBJ a = ocar(o); o=ocdr(o);
			tOBJ b = ocar(o); o=ocdr(o);
			tOBJ c = ocar(o); 
			return (*p->func3)(eval(a,e),eval(b,e),eval(c,e));
			}
		case 5:	
			{
			tOBJ a = ocar(o); o=ocdr(o);
			tOBJ b = ocar(o); o=ocdr(o);
			tOBJ c = ocar(o); o=ocdr(o);
			tOBJ d = ocar(o); o=ocdr(o);
			tOBJ g = ocar(o); 
			return (*p->func5)(eval(a,e),eval(b,e)
				,eval(c,e),eval(d,e),eval(g,e));
			}
		default:
			return (*p->funce)(o,e);
			break;
		}		
	}

	if (h.type==SYM)
		printf ("? unknown symbol [%s]\n", h.string);
	else
		printf ("? unknown (%s)\n",objtype(h));

	return emptyObj();
}

tOBJ eval2(tOBJ o, Dict *e)
{
	tOBJ h;

	if (o.q==1 || o.type==INTGR || o.type==FLOAT || o.type == FMAT2 || o.type == EMPTY ||o.type==FUNC || o.type==LAMBDA)
	{
		o.q=0;
		return cloneObj(o);
	}

	if (o.type==SYM)
	{
		return dict_getk(e, o.string);
	}

	h=ocar(o);
	o=ocdr(o);

	//lookup procedure

	tOBJ n= procall (h, o, e );

	//freeobj(&h);
	
	return n;

}

tOBJ eval(tOBJ o, Dict *e)
{
	if (dbg) {println ("eval - ",o); }
 	bas_mem();
	tOBJ r=eval2(o,e);
 	bas_mem();
	if (dbg) {println ("ret  - ",r);}
	return r;
}

tOBJ env;
static int  intf=1;

tOBJ eval_oxpr(char *s);

void init_extend()
{
	if (intf)
	{
		intf=0; 
		env = makedict(500);
		loadop(env.dict);

		if (dbg==0)
		{
			set(env.dict, "PI",  makefloat (3.1415926));	
			//seed the RND Gen
			srand ( (unsigned)time ( NULL ) );
			eval_oxpr("FUNC FACT (N) (IF (<= N 1) 1 (* N (FACT (- N 1))))");

			if (access("boot.l",F_OK) != -1)
			{
				oload(makestring("boot.l"));
			}
		}
	}
}

tOBJ eval_oxpr(char *s)
{
	init_extend();
	tOBJ e=parse(s);
	tOBJ f = eval(e, env.dict);
	freeobj(&e);
	return f;
}

void extend(char *s)
{
	tOBJ v=eval_oxpr(s);
	println (" = ", v);
	freeobj(&v);
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

	if (dbg) { for (int g=6; g<8; g++) testme(g); 	sigcatch();  }

	init_extend();
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


