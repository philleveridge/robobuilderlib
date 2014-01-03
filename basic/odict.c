#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "win.h"
#endif

#ifdef LINUX
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "linux.h"
#endif

#include "odict.h"

/**********************************************************/
/*  Dictionary                                            */
/**********************************************************/

extern int dbg;

Dict * newdict()
{
	Dict *n;
	if (dbg) printf ("New dictionary\n");
	n = (Dict *)malloc(sizeof(Dict));
	n->sz=100;
	n->ip=0;
	n->db = (Kvp *)malloc((n->sz) * sizeof(Kvp));
	return n;
}

int deldict(Dict *x)
{
	if (dbg) printf ("Delete dictionary\n");
	return 0;
}

tOBJ makedict()
{
	tOBJ r=emptyObj();
	Dict *f=newdict();
	r.type=DICT;
	r.dict=f;
	return r;
}

int dict_add(Dict *d, char *key, tOBJ value)
{
	tOBJ t1;

	if (d==NULL) return 0;

	if(dbg) printf ("Add %d %d\n", d->ip, d->sz);

	if ((d->ip) < (d->sz-1))
	{
		char *cp = d->db[d->ip].key;
		strncpy(cp, key, 32);

		//print(value);printf (" ... ");

		t1 = cloneObj(value);

		//print(t1);printf (" ...\n ");

		d->db[d->ip].value = t1; // should be clone;
		(d->ip)++;
	}
	return 0;
}	

int dict_find(Dict *d, char *key)
{
	int i=0;
	if (d==NULL) return -1;

	while (i<d->ip)
	{
		if (strcmp(key, d->db[i].key)==0)
		{
			return i;
		}
		i++;
	}
	return -1;
}

int dict_contains(Dict *d, char *key)
{
	if (d==NULL) return 0;
	return dict_find(d, key)>=0;
}

tOBJ dict_getk(Dict *d, char *key)
{
	int i=dict_find(d, key);

	if (i>=0) return d->db[i].value;
	return emptyObj();
}

tOBJ dict_get(Dict *d, int indx)
{
	if (indx >=0 && indx <d->ip)
		return d->db[indx].value;
	return emptyObj();
}


int dict_update(Dict *d, char *key, tOBJ value)
{
	int i;
	if (d==NULL) return 0;
	i=dict_find(d, key);
	if (i>=0) 
	{
		d->db[i].value = cloneObj(value); // clone it?
		return 1;
	}
	dict_add(d, key, value);
	return 0;
}

int dict_print(Dict *d)
{
	int i=0;

	if (d==NULL) return 0;

	while (i<d->ip)
	{
		tOBJ t;
		char *st="";
		t = d->db[i].value;

		switch (t.type)
		{  
		case INTGR: st="Int   "; break;

		case FLOAT: st="Float "; break;

		case STR:   st="String"; break;

		case FMAT2: st="Matrix"; break;

		case CELL:  st="List  "; break;

		case SYM:   st="Symbol"; break;

		case LAMBDA:st="Lambda"; break;

		case BOOLN: st="bool   ";break;

		case EMPTY: st="Empty  ";break;

		case FUNC:  break;
		}

		printf ("%7s [%s] ", d->db[i].key, st );

		if (t.type==LAMBDA)
		{
			t.type=CELL;
			print(t);
		}
		else
		{
			print (d->db[i].value);
		}
		printf ("\n");
		i++;
	}
	return 0;
}



