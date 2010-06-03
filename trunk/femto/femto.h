#define bool  int
#define true  1
#define false 0
#define null  (void *)0

#define DEBUG(a)   if(dbg) {a;}

enum  TYPE {SYMBOL, INT, BOOL, FUNCTION, SPECIAL, FLOAT, STRING, CELL, EMPTY, ERROR};

typedef struct object {
	int   type;
	bool  q;
	union { float 	floatpoint; 
	        int 	number; 
			char 	*string;
			void 	*cell;
			void 	*func;
		};
} tOBJ;

typedef struct cell { 
	tOBJ 			head;		
	struct cell*  	tail;		
} tCELL, *tCELLp; 

typedef tOBJ (*PFP)(tCELLp);

tOBJ throw(int n);
extern int dbg;

extern void printline(char *c);  
extern void printint(int);  	
extern void printstr(char*);	
extern void printnumber(int n, int w, char pad)  ;