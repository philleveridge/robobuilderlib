#ifndef OSTACK_H
#define OSTACK_H

#include "oobj.h"

typedef struct Stack {
	unsigned char type;
	int  size;
	int  noe;
	tOBJ *objarray;
} tStack, *tStackp;

extern tOBJ makestack	(int n, unsigned char t);
extern void delstack	(tStackp p);

extern int  push 	(tStackp p, tOBJ a);
extern tOBJ pop  	(tStackp p);
extern tOBJ peek 	(tStackp p, int n);
extern void clear	(tStackp p);
extern void stackprint	(tStackp p);
extern int  stacksize 	(tStackp p);
extern tStackp clonestack(tStackp x);

#endif
