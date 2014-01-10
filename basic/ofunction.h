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

extern tOP oplist[];

extern int  set   (Dict * en, char *name, tOBJ r);
extern tOBJ get   (Dict *ep, char *name);
extern int  getOP (char *str);
extern tOBJ omath (tOBJ o1, tOBJ o2, int op);

extern tOBJ osin ();
extern tOBJ ocos(tOBJ  r);
extern tOBJ otan(tOBJ  r);
extern tOBJ osqrt(tOBJ  r);
extern tOBJ olog(tOBJ  r);
extern tOBJ oexp(tOBJ  r);
extern tOBJ oacos(tOBJ  r);
extern tOBJ oatan(tOBJ  r);
extern tOBJ oabs(tOBJ  r);
extern tOBJ oacx(tOBJ  r);
extern tOBJ oacy(tOBJ  r);
extern tOBJ oacz(tOBJ  r);
extern tOBJ osig(tOBJ  r);
extern tOBJ ornd(tOBJ  r);
extern tOBJ odsig(tOBJ  r);
extern tOBJ opsd(tOBJ  r);
extern tOBJ omax(tOBJ  r, tOBJ s);

extern tOBJ omat(tOBJ  r, tOBJ, tOBJ);
extern tOBJ omats(tOBJ  r, tOBJ, tOBJ, tOBJ, tOBJ);
extern tOBJ omatr(tOBJ  r, tOBJ, tOBJ);

extern tOBJ otrn(tOBJ  r);
extern tOBJ oapply(tOBJ  r, tOBJ);
extern tOBJ omapcar(tOBJ  r, tOBJ);
extern tOBJ orshp(tOBJ  r, tOBJ, tOBJ);
extern tOBJ orep(tOBJ  r, tOBJ, tOBJ);
extern tOBJ ozero(tOBJ  r, tOBJ s);
extern tOBJ ominv(tOBJ  r);
extern tOBJ oeye(tOBJ  r, tOBJ s);
extern tOBJ ovsum(tOBJ  r);
extern tOBJ ohsum(tOBJ  r);
extern tOBJ ovsum2(tOBJ  r);
extern tOBJ ohsum2(tOBJ  r);
extern tOBJ osum(tOBJ  r);
extern tOBJ oconv(tOBJ  r, tOBJ);
extern tOBJ oimp(tOBJ  r, tOBJ, tOBJ);
extern tOBJ omcond(tOBJ  r, tOBJ, tOBJ, tOBJ, tOBJ);
extern tOBJ ozerob(tOBJ  r, tOBJ, tOBJ, tOBJ, tOBJ);
extern tOBJ ozerod(tOBJ  r);
extern tOBJ ocar(tOBJ  r);
extern tOBJ ocdr(tOBJ  r);
extern tOBJ ecar(tOBJ  r);
extern tOBJ ecdr(tOBJ  r);
extern tOBJ otype(tOBJ  r);
extern tOBJ olen(tOBJ  r);
extern tOBJ oapp(tOBJ  r);
extern tOBJ ocons(tOBJ  r, tOBJ a);
extern tOBJ olast(tOBJ  r);
extern tOBJ osubst(tOBJ  r);
extern tOBJ orev(tOBJ  r);
extern tOBJ onull(tOBJ  r);
extern tOBJ omemb(tOBJ  r);
extern tOBJ oatom(tOBJ  r);
extern tOBJ oasso(tOBJ  r);
extern tOBJ onot(tOBJ  r);
extern tOBJ okey(tOBJ  r);
extern tOBJ owait(tOBJ a);
extern tOBJ oset(tOBJ  r);
extern tOBJ oget(tOBJ  r);
extern tOBJ oserv(tOBJ  r);
extern tOBJ oputch(tOBJ a);

extern tOBJ opose(tOBJ  r);
extern tOBJ owhs(tOBJ  r);
extern tOBJ oexit(tOBJ  r);
extern tOBJ obf(tOBJ  r);
extern tOBJ oload(tOBJ  r);
extern tOBJ osave(tOBJ  r, tOBJ);
extern tOBJ odict(tOBJ  r);

extern tOBJ oprt(tOBJ  r);
extern tOBJ olet(tOBJ  r); 
extern tOBJ odefn(tOBJ  r); 
extern tOBJ oqt(tOBJ  r); 
extern tOBJ orex(tOBJ  r, tOBJ); 
extern tOBJ olft(tOBJ  r, tOBJ); 
extern tOBJ orgt(tOBJ  r, tOBJ); 
extern tOBJ omid(tOBJ  r, tOBJ, tOBJ); 
extern tOBJ omdet(tOBJ  r); 
extern tOBJ print(tOBJ r);
extern tOBJ orbmrf(tOBJ r);

extern tOBJ onth(tOBJ  r, tOBJ);

extern tOBJ ocond   (tOBJ  r, Dict *);
extern tOBJ obegin  (tOBJ  r, Dict *);
extern tOBJ oand    (tOBJ  r, Dict *);
extern tOBJ oor     (tOBJ  r, Dict *);
extern tOBJ oplus   (tOBJ  r, Dict *);
extern tOBJ olist   (tOBJ  r, Dict *);
extern tOBJ oappend (tOBJ  r, Dict *);
extern tOBJ opr     (tOBJ  r, Dict *);
extern tOBJ oimg    (tOBJ  r, Dict *); // {"UNLO" "LOAD" "FILT" "RAW" "THRE" "COLO" "PROC" "REG" "SHOW" "DEBU"}

extern tOBJ ofor    (tOBJ  r, Dict *);
extern tOBJ ofore   (tOBJ  r, Dict *);
extern tOBJ owhile  (tOBJ  r, Dict *);


#endif
