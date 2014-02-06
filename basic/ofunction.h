#ifndef OFUNCTIONS_H
#define OFUNCTIONS_H

#include "oobj.h"
#include "odict.h"

extern tOBJ eval_oxpr(char *s);
extern tOBJ eval(tOBJ o, Dict *e);

enum  MATHOP	{NA, PLUS, MINUS, DIVD, MULT, LAND, LOR, OBR, CBR, LT, GT, EQL, NEQ, COMMA, PROD, POWR, GTE, LTE, LMOD};

typedef struct ops {
        char		*name;
	int		level;
	unsigned char   type;
	int		nop;
	union {
		tOBJ	(*func)(tOBJ);
		tOBJ	(*func2)(tOBJ, tOBJ);
		tOBJ	(*func3)(tOBJ, tOBJ, tOBJ);
		tOBJ	(*func5)(tOBJ, tOBJ, tOBJ, tOBJ, tOBJ);
		tOBJ	(*funce)(tOBJ, Dict *);
	};
} tOP, *tOPp;

extern tOP  oplist[];
extern void loadop 	(Dict *e);
extern tOBJ getOP2	(Dict *e, char *str);
extern int  getOPlevel	(Dict *e, char *str);
extern int  getOPtype	(Dict *e, char *str);

extern int  set   	(Dict *en, char *name, tOBJ r);
extern tOBJ setb   	(tOBJ  s, tOBJ r);
extern tOBJ get   	(Dict *ep, char *name);
extern tOBJ getb   	(tOBJ  r);
extern tOBJ omath 	(tOBJ o1, tOBJ o2, int op);

extern tOBJ oabs	(tOBJ  r);
extern tOBJ oacos	(tOBJ  r);
extern tOBJ oacx	(tOBJ  r);
extern tOBJ oacy	(tOBJ  r);
extern tOBJ oacz	(tOBJ  r);
extern tOBJ oand    	(tOBJ  r, Dict *);
extern tOBJ oappend 	(tOBJ  r, Dict *);
extern tOBJ oapply	(tOBJ  r, tOBJ);
extern tOBJ oapp	(tOBJ  r);
extern tOBJ oasso	(tOBJ  r);
extern tOBJ oatan	(tOBJ  r);
extern tOBJ oatom	(tOBJ  r);
extern tOBJ oatos	(tOBJ  r);
extern tOBJ obegin  	(tOBJ  r, Dict *);
extern tOBJ obf		(tOBJ  r);
extern tOBJ obreak	(tOBJ  r);
extern tOBJ ocar	(tOBJ  r);
extern tOBJ ocaar	(tOBJ  r);
extern tOBJ ocadr	(tOBJ  r);
extern tOBJ ocadar	(tOBJ  r);
extern tOBJ ocdr	(tOBJ  r);
extern tOBJ ocol	(tOBJ  r);
extern tOBJ ocond   	(tOBJ  r, Dict *);
extern tOBJ ocons	(tOBJ  r, tOBJ a);
extern tOBJ oconv	(tOBJ  r, tOBJ);
extern tOBJ ocos	(tOBJ  r);
extern tOBJ odefn	(tOBJ  r); 
extern tOBJ odict	(tOBJ  r);
extern tOBJ odsig	(tOBJ  r);
extern tOBJ oexit	(tOBJ  r);
extern tOBJ oexp	(tOBJ  r);
extern tOBJ oeye	(tOBJ  r, tOBJ s);
extern tOBJ ofore    	(tOBJ  r, Dict *);
extern tOBJ ofor     	(tOBJ  r, Dict *);
extern tOBJ ogaus     	(tOBJ  ,tOBJ, tOBJ);
extern tOBJ ogausk     	(tOBJ  ,tOBJ);
extern tOBJ oget     	(tOBJ  r);
extern tOBJ ogetservo	(tOBJ  r, tOBJ );
extern tOBJ ohsum2   	(tOBJ  r);
extern tOBJ ohsum   	(tOBJ  r);
extern tOBJ oimg    	(tOBJ  r, Dict *); 
extern tOBJ oint    	(tOBJ  r); 
extern tOBJ oimp    	(tOBJ  r, tOBJ, tOBJ);
extern tOBJ okey    	(tOBJ  r);
extern tOBJ olast   	(tOBJ  r);
extern tOBJ olen	(tOBJ  r);
extern tOBJ olet	(tOBJ  r); 
extern tOBJ olft	(tOBJ  r, tOBJ); 
extern tOBJ olist   	(tOBJ  r, Dict *);
extern tOBJ oload	(tOBJ  r);
extern tOBJ olog	(tOBJ  r);
extern tOBJ omapcar	(tOBJ  r, tOBJ);
extern tOBJ omatr	(tOBJ  r, tOBJ, tOBJ);
extern tOBJ omats	(tOBJ  r, tOBJ, tOBJ, tOBJ, tOBJ);
extern tOBJ omat	(tOBJ  r, tOBJ, tOBJ);
extern tOBJ omax	(tOBJ  r, tOBJ s);
extern tOBJ omcond	(tOBJ  r, tOBJ, tOBJ, tOBJ, tOBJ);
extern tOBJ omdet	(tOBJ  r); 
extern tOBJ omemb	(tOBJ  r, tOBJ);
extern tOBJ omid	(tOBJ  r, tOBJ, tOBJ); 
extern tOBJ ominv	(tOBJ  r);
extern tOBJ onot	(tOBJ  r);
extern tOBJ onth	(tOBJ  r, tOBJ);
extern tOBJ onull	(tOBJ  r);
extern tOBJ oor     	(tOBJ  r, Dict *);
extern tOBJ opeek   	(tOBJ  r, tOBJ);
extern tOBJ oplus   	(tOBJ  r, Dict *);
extern tOBJ opose	(tOBJ  r);
extern tOBJ opop	(tOBJ  r);
extern tOBJ opr    	(tOBJ  r, Dict *);
extern tOBJ oprt	(tOBJ  r);
extern tOBJ opsd	(tOBJ  r);
extern tOBJ opush	(tOBJ  a, tOBJ b);
extern tOBJ oputch	(tOBJ  a);
extern tOBJ oqt		(tOBJ  r); 
extern tOBJ orbmrf	(tOBJ  r);
extern tOBJ orep	(tOBJ  r, tOBJ, tOBJ);
extern tOBJ oreturn	(tOBJ  r);
extern tOBJ orev	(tOBJ  r);
extern tOBJ orex	(tOBJ  r, tOBJ); 
extern tOBJ orgt	(tOBJ  r, tOBJ); 
extern tOBJ ornd	(tOBJ  r);
extern tOBJ orow	(tOBJ  r);
extern tOBJ orshp	(tOBJ  r, tOBJ, tOBJ);
extern tOBJ osave	(tOBJ  r, tOBJ);
extern tOBJ oserv	(tOBJ  r);
extern tOBJ oset	(tOBJ  r);
extern tOBJ osetservo	(tOBJ  r, tOBJ, tOBJ);
extern tOBJ osig	(tOBJ  r);
extern tOBJ osign	(tOBJ  r);
extern tOBJ osin 	(tOBJ  r);
extern tOBJ osqrt	(tOBJ  r);
extern tOBJ ostack	(tOBJ  r);
extern tOBJ ostoa	(tOBJ  r);
extern tOBJ osubst	(tOBJ  r);
extern tOBJ osum	(tOBJ  r);
extern tOBJ otan	(tOBJ  r);
extern tOBJ otrn	(tOBJ  r);
extern tOBJ otype	(tOBJ  r);
extern tOBJ ovsum2	(tOBJ  r);
extern tOBJ ovsum	(tOBJ  r);
extern tOBJ owait	(tOBJ  a);
extern tOBJ owhile  	(tOBJ  r, Dict *);
extern tOBJ owhs	(tOBJ  r);
extern tOBJ ozerob	(tOBJ  r, tOBJ, tOBJ, tOBJ, tOBJ);
extern tOBJ ozerod	(tOBJ  r);
extern tOBJ ozero	(tOBJ  r, tOBJ s);
extern tOBJ print	(tOBJ  r);

#endif
