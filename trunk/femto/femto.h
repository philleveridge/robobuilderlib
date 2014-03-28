#ifndef FEMTO_H
#define FEMTO_H


#include "oobj.h"
#include "ocells.h"
#include "ostring.h"
#include "odict.h"
#include "ostack.h"
#include "ostring.h"
#include "ofunction.h"

#include "oexpress.h"

extern tOBJ env;   

extern void 	readLine	(char *, int n);

#define DEBUG(a)   if(dbg) {a;}


//battery.c
extern volatile WORD   gticks;
extern volatile WORD   g10MSEC;
extern volatile WORD   g10Mtimer;
extern volatile BYTE   gSEC;
extern volatile BYTE   gMIN;
extern void     delay_ms(int ms);

//main.c
extern void putByte (BYTE b);
extern int  getByte ();

#define NULL (void *)0

#endif

