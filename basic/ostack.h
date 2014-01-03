#ifndef OSTACK_H
#define OSTACK_H

#define MAXSTACK 50

#include "oobj.h"

extern int push(tOBJ a);
extern tOBJ pop();
extern tOBJ peek(int n);
extern void clear();
extern void stackprint();
extern int stacksize();

#endif
