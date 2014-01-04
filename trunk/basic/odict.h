#ifndef ODICT_H
#define ODICT_H

/**********************************************************/
/*  Dictionary                                            */
/**********************************************************/

#include "oobj.h"

typedef struct kvp {
	char key[32];
	tOBJ value;
} Kvp;

typedef struct dict {
	int sz;
	int ip;
	Kvp *db;
	struct dict *outer;
} Dict;

extern Dict *newdict		();
extern int   deldict		(Dict *x);
extern tOBJ  makedict		();
extern tOBJ  makedict2(		Dict *e);
extern int   dict_add		(Dict *d, char *key, tOBJ value);
extern int   dict_find		(Dict *d, char *key);
extern int   dict_contains	(Dict *d, char *key);
extern tOBJ  dict_getk		(Dict *d, char *key);
extern tOBJ  dict_get		(Dict *d, int indx);
extern int   dict_update	(Dict *d, char *key, tOBJ value);
extern int   dict_print		(Dict *d);

#endif

