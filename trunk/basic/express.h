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
//
// The new expression parser
// work in progress
//

typedef struct fmat {
	int h; 
	int w;
	float *fstore;
} fMatrix;


typedef struct object {
        int   type;
	unsigned char cnt;
        union { 
		int	q;
		float   floatpoint; 
                int     number; 
                char    *string;
                void    *cell;
                void    *func;
		char    fmat;
		fMatrix fmat2;
                };
} tOBJ;

typedef struct cell { 
        tOBJ                    head;           
        struct cell*    tail;           
} tCELL, *tCELLp; 

typedef tOBJ (*PFP)(tCELLp);

enum  TOKTYP	{NUMI, NUMF, DELI, ALPHA, OPR, STRNG, MLIST};
enum  MATHOP	{NA, PLUS, MINUS, DIVD, MULT, LAND, LOR, OBR, CBR, LT, GT, EQL, NEQ, COMMA};
enum  TYPE	{SYM, INTGR, BOOLN, FUNC, FLOAT, STR, CELL, EMPTY, FMAT, FMAT2};

tOBJ eval_oxpr(char *s);

#endif

#endif
