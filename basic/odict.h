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
	int cf;
	int ip;
	Kvp *db;
	struct dict *outer;
} Dict;

extern Dict *newdict		(int n);
extern int   deldict		(Dict *x);
extern tOBJ  makedict		(int n);
extern tOBJ  makedict2		(Dict *e,int n);
extern int   dict_add		(Dict *d, char *key, tOBJ value);
extern int   dict_find		(Dict *d, char *key);
extern int   dict_contains	(Dict *d, char *key);
extern tOBJ  dict_getk		(Dict *d, char *key);
extern tOBJ  dict_get		(Dict *d, int indx);
extern int   dict_update	(Dict *d, char *key, tOBJ value);
extern int   dict_updateonly    (Dict *d, char *key, tOBJ value);
extern int   dict_print		(Dict *d, int f);
extern Dict  *clonedict         (Dict *x);

#endif

