#ifndef EXPRESS_H
#define EXPRESS_H

enum {STRING, NUMBER, ARRAY, ERROR, CONDITION } ;

#ifdef WIN32 
#define MAX_DEPTH  500
#endif

#ifdef LINUX | IMAGE
#define MAX_DEPTH  500
#endif

//#ifdef IMAGE
//#define MAX_DEPTH  500
//#endif

#ifdef AVR
#define MAX_DEPTH  25
#endif

extern unsigned char 	eval_expr(char **str, long *res);
extern int		eval_list(char *p);
extern long		math(long n1, long n2, char op);
extern void		showvars();
extern void		setvar(char n, long v);
extern long		getvar(char n);

typedef struct mat {
	unsigned char h; 
	unsigned char w;
} Matrix;



#ifdef PARSE

#include <stdio.h>
#include "linux.h"
#include "oobj.h"

#endif

#endif
