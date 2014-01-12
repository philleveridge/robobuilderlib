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

Dict * newdict(int sz)
{
	Dict *n;
	if (dbg) printf ("New dictionary (%d)\n",sz);
	n = (Dict *)malloc(sizeof(Dict));
	if (sz==0) sz=100; //default
	n->sz=sz;
	n->ip=0;
	n->db = (Kvp *)malloc((n->sz) * sizeof(Kvp));
	n->outer=NULL;
	return n;
}

int deldict(Dict *x)
{
	if (dbg) printf ("Delete dictionary\n");
	return 0;
}

tOBJ makedict(int n)
{
	tOBJ r=emptyObj();
	Dict *f=newdict(n);
	r.type=DICT;
	r.dict=f;
	return r;
}

tOBJ makedict2(Dict *e)
{
	tOBJ r= makedict(0);
	((Dict *)(r.dict))->outer=e;
	return r;
}

int dict_add(Dict *d, char *key, tOBJ value)
{
	tOBJ t1;

	if (d==NULL) return 0;

	if(dbg) printf ("Add %d %d\n", d->ip, d->sz);

	if ((d->ip) < d->sz)
	{
		char *cp = d->db[d->ip].key;
		strncpy(cp, key, 32);

		t1 = cloneObj(value);

		d->db[d->ip].value = t1; // should be clone;
		(d->ip)++;
		return 1;
	}
	printf ("?error no space in dict \n");
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
	if (d==0) 
		return (1==0);

	return (dict_find(d, key)>=0)?1:dict_contains(d->outer, key);

}
tOBJ dict_getk(Dict *d, char *key)
{
	int i;
	if (d==NULL) 
		return emptyObj();

	i=dict_find(d, key);

	if (i>=0) 
		return d->db[i].value;

	return dict_getk(d->outer,key);
}

tOBJ dict_get(Dict *d, int indx)
{
	if (d==NULL) return emptyObj();	
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
//	else
//	{
//		if (dict_update(d->outer, key, value))
//			return 1;
//	}
	dict_add(d, key, value);
	return 0;
}

int dict_print(Dict *d)
{
	int i=0;

	if (d==NULL) return 0;

	while (i<d->ip)
	{
		tOBJ t  = d->db[i].value;
		char *st=objtype(t);

		printf ("%7s [%s] ", d->db[i].key, st );

		if (t.type==LAMBDA)
		{
			t.type=CELL;
			print(t);
		}
		else 
		if (t.type==DICT || t.type==RBM || t.type==FMAT2)
		{
			printf (" +++");
		}
		else
		{
			print (d->db[i].value);
		}
		printf ("\n");
		i++;
	}
	printf ("--\n");
	dict_print(d->outer);
	return 0;
}



