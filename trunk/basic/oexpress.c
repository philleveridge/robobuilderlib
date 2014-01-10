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

char *readword(char *s, char *w)
{
	char c;
	int n=0;
	int sf=0;
	while (*s != '\0')
	{
		c=*s++;
		if (c==13 || c==10) {continue;} // ignore CR & LFs

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

		if (	(c=='<' && *s =='>'  && n==0) 
			|| (c=='<' && *s =='=' && n==0) 
			|| (c=='.' && *s =='*' && n==0) 
			|| (c=='.' && *s =='^' && n==0) 
			|| (c=='>' && *s =='=' && n==0) )
		{
			*w++ = c;
			*w++=*s++;
			*w='\0';
			return s;
		}

		if ( strchr("`'+-*/=:,;[]{}()<> ",c)>0  && sf==0)
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

		if (!strcmp(buffer,"-")) 
		{
			s = readword(s, buffer+1);
		}

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
		tCELLp next;
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
			}
			else
			if (buffer[0]=='[')
			{
				top->head.type=FMAT2;	
				top->head.fmat2=readmatrix(&s);			
			}
			else{
				top->head = makestring(buffer);
				top->head.type=SYM;
			}
		}

		top->tail=NULL;	

		if (s != NULL && *s != '\0')
		{
			next=newCell();
			next->head=emptyObj();
			top->tail=next;	
			top=next;
		}	
	}
	return r;
}

tOBJ read_from(tOBJ *r)
{
	tOBJ h, L = emptyObj();
	int qf=0;

	while (r->type!=EMPTY)
	{
		h=ocar(*r);
		*r=ocdr(*r);

		if (h.type==SYM && !strcmp(h.string, "'"))
		{	
			qf=1;	
		}
		else
		if (h.type==SYM && !strcmp(h.string, "{"))
		{	
			tOBJ t=read_from(r);
	
			t.q=qf;	
			L=append(L,t );
			qf=0;
		}
		else
		if (h.type==SYM && !strcmp(h.string, "}"))
		{
			break;
		}
		else
		{
			h.q=qf;	qf=0;
			L=append(L,h);	
		}
	}
	return L;
}

tOBJ parse(char *s)
{
	tOBJ L,r;	
	if (dbg) printf ("parse 2 %s\n", s); 
	r = tokenise(s);
	L = read_from(&r);	
	return L;
}


tOBJ formula(tOBJ *o, Dict *e)
{
	// ( V op V )
	// V op V op V
	// F (arg) -tbd
	// 
	tOBJ r=emptyObj();
	tOBJ v1=emptyObj();
	tOBJ v2=emptyObj();

	int op=0;
	int p=0;

	tOBJ n=emptyObj();;

	while (1)
	{
		if (dbg) {print (v1); print (v2);print (n); print (*o); printf(" p=%d\n",p); }

		if (p!=0)
		{
			n=ocar(*o);
			*o=ocdr(*o);

			if (n.type==SYM)
			{
				//is it a function?
				op = getOP(n.string);
				if (op>=0 &&  (oplist[op].type==NA || oplist[op].type==CBR)) return r; //fucntion not op
			}
			else if (n.type==EMPTY)
			{
				return v1;
			}
			else
			{
				printf("?format ?\n");
				return r;
			}
		}

		n=ocar(*o);
		*o=ocdr(*o);

		if (n.type==INTGR || n.type==FLOAT || n.type==STR)
			v2=n;
		else if (n.type==SYM)
		{
			if (!strcmp(n.string,"("))
			{
				v2=formula(o,e);
			}
			else
			{
				//is it a variable?
				v2=dict_getk(e, n.string);
			}
		}
		else if (n.type==EMPTY)
		{
			return v1;
		}
		else
		{
			printf("?format   ?\n");
			return r;
		}

		if (p==0)
		{
			v1=v2;
			p=1;
		}
		else
		{
			if (op>=0) r=omath(v1,v2, oplist[op].type);
			v1=r;
		}
	}
	return r;
}


tOBJ callfn(tOBJ  fn, tOBJ x, Dict *env)
{
	tOBJ r, arg, body;
	tOBJ e= makedict2(env);

	fn.type=CELL;

	arg=ocar(fn);
	body=ocdr(fn);

	if (dbg) println("Lambda args=",x);

	while (onull(arg).number==0)
	{
		tOBJ n=ocar(arg);tOBJ v;

		if (oatom(x).number)
		{
			v=eval(x,(Dict *)e.dict);
		}
		else
		{
			v=eval(ocar(x),(Dict *)e.dict);
		}

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
		r = eval(x, (Dict *)e.dict);
		body = ocdr(body);
		} while (onull(body).number==0);
	}	
	return r;
}

tOBJ procall (tOBJ h, tOBJ o, Dict *e )
{

	if (h.type==LAMBDA)
	{
		//!ABC 2 4
		return callfn(h, o, e);
	}

	if (h.type==FUNC)
	{
		return (*h.func)(eval(o,e));
	}


	if (h.type!=SYM)
	{
		return emptyObj();
	}

	//lookup procedure
	int op = getOP(h.string);
	if (op>=0)
	{
		if (oplist[op].type != NA && oplist[op].type != CBR && oplist[op].type != OBR)
		{
			tOBJ a = ocar(o); o=ocdr(o);
			tOBJ b = ocar(o); 					
			return omath(eval(a,e),eval(b,e),oplist[op].type);
		}
		else//call
		switch(oplist[op].nop)
		{
		case 0:
			return (*oplist[op].func)(emptyObj());
		case 1:	
			return (*oplist[op].func)(eval(ocar(o),e));
		case 2:	
			{
			tOBJ a = ocar(o); o=ocdr(o);
			tOBJ b = ocar(o); 
			return (*oplist[op].func2)(eval(a,e),eval(b,e));
			}
		case 3:	
			{
			tOBJ a = ocar(o); o=ocdr(o);
			tOBJ b = ocar(o); o=ocdr(o);
			tOBJ c = ocar(o); 
			return (*oplist[op].func3)(eval(a,e),eval(b,e),eval(c,e));
			}
		case 5:	
			{
			tOBJ a = ocar(o); o=ocdr(o);
			tOBJ b = ocar(o); o=ocdr(o);
			tOBJ c = ocar(o); o=ocdr(o);
			tOBJ d = ocar(o); o=ocdr(o);
			tOBJ g = ocar(o); 
			return (*oplist[op].func5)(eval(a,e),eval(b,e)
				,eval(c,e),eval(d,e),eval(g,e));
			}
		default:
			return (*oplist[op].funce)(o,e);
			break;
		}
	}
	printf ("? unknown symbol [%s]\n", h.string);
	return emptyObj();
}

tOBJ eval(tOBJ o, Dict *e)
{
	tOBJ r=emptyObj();
	tOBJ h;

	if (dbg) {println ("eval - ",o);}

	if (o.q==1 || o.type==INTGR || o.type==FLOAT || o.type==STR || o.type == FMAT2 || o.type == EMPTY )
	{
		o.q=0;
		return o;
	}

	if (o.type==SYM)
	{
		return dict_getk(e, o.string);
	}

	if (o.type==LAMBDA)
	{
		return o;
	}

	if (o.type==FUNC)
	{
		return o;
	}

	h=ocar(o);
	o=ocdr(o);

	if (h.type==SYM)
	{
		if (dict_contains(e, h.string))
		{
			h=dict_getk(e,h.string);
		}

		if (!strcmp(h.string,"QT"))
		{
			return ocar(o);
		}
		else
		if (!strcmp(h.string,"EVAL"))
		{
			return eval(eval(ocar(o),e),e);
		}
		else
		if (!strcasecmp(h.string,"SETQ"))
		{
			tOBJ var = ocar(o); o=ocdr(o);
			tOBJ exp = ocar(o); 
			dict_update(e, var.string, exp);
			return exp;
		}
		else
		if (!strcasecmp(h.string,"READ"))
		{
			tOBJ var = eval(ocar(o),e);
			if (var.type==STR) return tokenise(var.string);
		}
		else
		if (!strcasecmp(h.string,"SET"))
		{
			tOBJ var = ocar(o); o=ocdr(o);
			tOBJ exp = eval(ocar(o),e); 
			dict_update(e, var.string, exp);
			return exp;
		}
		else
           	if (!strcasecmp(h.string,"IF"))
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
		if (!strcasecmp(h.string,"LET"))
		{
			//LET VAR = FORMULA
			tOBJ var = ocar(o); o=ocdr(o);
			tOBJ eq =  ocar(o); o=ocdr(o);
			if (eq.type != STR && strcmp(eq.string,"=")) {printf ("err\n"); return emptyObj();}
			eq = formula(&o,e);
			dict_update(e, var.string, eq);
			return eq;
		}
		else
		if (!strcasecmp(h.string,"FUNC") || !strcasecmp(h.string,"DEFN"))
		{
			//!FUNC ABC {X Y} {PLUS X Y}
			tOBJ fn = ocar(o); o=ocdr(o);
			o.type  = LAMBDA;
			set(e, fn.string, o);
			return o;
		}
		else
		if (!strcasecmp(h.string,"LAMBDA"))
		{
			//!LAMBA {X Y} {PLUS X Y}
			o.type  = LAMBDA;
			return o;
		}
		else
		{
			//lookup procedure
			return procall (h, o, e );
		}
	}
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
		env = makedict();

		set(env.dict, "PI",  makefloat (3.1415926));
	
		//seed the RND Gen
		srand ( (unsigned)time ( NULL ) );

		eval_oxpr("FUNC FACT {N} {IF {<= N 1} 1 {* N {FACT {- N 1}}}}");
	}
}

tOBJ eval_oxpr(char *s)
{
	init_extend();
	return eval(parse(s), env.dict);
}

void extend(char *s)
{
	println (" = ", eval_oxpr(s));
}

extern int readLine(char *line);

#define MAX 255
void repl()
{
	char inputbuffer[MAX];
	while (1)
	{
		printf("!");
		readLine(&inputbuffer[0]);
		if (!strcasecmp(inputbuffer,"q")) return;
		extend(inputbuffer);
	}
}


