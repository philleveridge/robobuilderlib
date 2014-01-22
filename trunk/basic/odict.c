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
#include "ofunction.h"

/**********************************************************/
/*  Dictionary                                            */
/**********************************************************/

extern int dbg;

Dict * newdict(int sz)
{
	Dict *n;
	if (dbg>1) printf ("New dictionary (%d)\n",sz);
	n = (Dict *)malloc(sizeof(Dict));
	if (sz==0) sz=100; //default
	n->sz   = sz;
	n->ip   = 0;
	n->db   = (Kvp *)malloc((n->sz) * sizeof(Kvp));
	n->outer= NULL;
	return n;
}

int deldict(Dict *x)
{
	if (dbg) printf ("Delete dictionary\n");
	if (x != NULL)
	{
		free(x->db);
		free(x);
		x=NULL;
	}
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

tOBJ makedict2(Dict *e, int n)
{
	tOBJ r= makedict(n);
	((Dict *)(r.dict))->outer=e;
	return r;
}


int dict_add(Dict *d, char *key, tOBJ value)
{
       tOBJ t1;

       if (d==NULL) return 0;

       if(dbg>1) printf ("Add %s %d %d\n", key, d->ip, d->sz);
       int i=0;

       if (d->ip==d->sz) {printf ("?error no space in dict [%s] %d/%d\n",key,d->ip,d->sz); return 0;}

       while (i < d->ip)
       {
              int c =strcmp(key, d->db[i].key);
              if (c==0) {printf ("Already exists (%s %d)\n",key,i); i=d->ip; break;}
              if (c>0) {i++; continue;}
              if (c<0)
              {      //insert gap here
                     for (int j=d->ip; j>i ;j--)
                     	d->db[j]=d->db[j-1];
                     break;
              }
       }
       //insert at i
       char *cp = d->db[i].key;
       strncpy(cp, key, 32);
       t1 = cloneObj(value);
       d->db[i].value = t1; // should be clone;
       (d->ip)++;
       return 1;
}
 

// listed sorted for binary search
int dict_find(Dict *d, char *key)
{
	int low=0;
	int high=d->ip-1;

	if (d==NULL) return -1;

	while (high>=low) 
	{
		int i=low+(high-low)/2;
		int c=strcmp(key, d->db[i].key);

		if (dbg>2) printf ("%s [%d], [%d,%d,%d]\n", d->db[i].key, c, low, i,high);
		if (c==0)
		{
		     return i;
		}
		if (c>0)
		{
		     low=i+1;
		}
		if (c<0)
		{
		     high=i-1;
		}
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

int dict_updateonly(Dict *d, char *key, tOBJ value)
{
	int i;
	if (d==NULL) return 0;
	i=dict_find(d, key);
	if (i>=0) 
	{
		d->db[i].value = cloneObj(value); // clone it?
		return 1;
	}
	else
	{
		return dict_updateonly(d->outer, key, value);
	}
}

int dict_update(Dict *d, char *key, tOBJ value)
{
	if (d==0) return 0;
	return dict_updateonly(d,key,value) || dict_add(d, key, value);
}

int dict_print(Dict *d, int f)
{
	int i=0;

	if (d==NULL) return 0;

	while (i<d->ip)
	{
		tOBJ t  = d->db[i].value;
		char *st=objtype(t);

		if (f && t.type==FUNC) {i++; continue;}

		printf ("%7s [%s] ", d->db[i].key, st );

		if (t.type==LAMBDA)
		{
			t.type=CELL;
			print(ocar(t));
		}
		else 
		if (t.type==DICT || t.type==RBM || t.type==FMAT2 )
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
	if (dbg>1)
	{
		printf ("--\n");
		dict_print(d->outer,f);
	}
	return 0;
}



