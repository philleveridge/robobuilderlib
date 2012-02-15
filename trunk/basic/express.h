#ifndef EXPRESS_H
#define EXPRESS_H

enum {STRING, NUMBER, ARRAY, ERROR, CONDITION } ;

extern long variable[];

#define MAX_DEPTH  10

extern unsigned char 	eval_expr(char **str, long *res);
extern int		eval_list(char *p);
extern long		math(long n1, long n2, char op);
extern void		showvars();

#ifdef PARSE

typedef struct object {
        int   type;
        union { int	q;
		float   floatpoint; 
                int     number; 
                char    *string;
                void    *cell;
                void    *func;
                };
} tOBJ;

typedef struct cell { 
        tOBJ                    head;           
        struct cell*    tail;           
} tCELL, *tCELLp; 

typedef tOBJ (*PFP)(tCELLp);

enum  TOKTYP	{NUMI, NUMF, DELI, ALPHA, OPR};
enum  MATHOP	{NA, PLUS, MINUS, DIVD, MULT, LAND, LOR, OBR, CBR, LT, GT, EQL, NEQ, COMMA};
enum  TYPE	{SYM, INTGR, BOOLN, FUNC, FLOAT, STR, CELL, EMPTY};

tOBJ eval_oxpr(char *s);

#endif

#endif
