#ifndef OEXPRESS_H
#define OEXPRESS_H

extern tOBJ env;   

extern tOBJ    parse       (char *s);
extern fMatrix *readmatrix  (char **str );
extern tOBJ    procall     (tOBJ h, tOBJ o, Dict *e );
extern tOBJ    tokenise    (char *s);
extern tOBJ    formula     (tOBJ ae, Dict *e);

#endif

