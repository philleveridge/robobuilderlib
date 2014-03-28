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
#include "mem.h"

/**********************************************************/
/*  Dictionary                                            */
/**********************************************************/

extern int dbg;

Dict * newdict(int sz)
{
	Dict *n;
	if (dbg>1) printf ("New dictionary (%d)\n",sz);
	n = (Dict *)bas_malloc(sizeof(Dict));
	if (sz==0) sz=100; //default
	n->sz   = sz;
	n->ip   = 0;
	n->cf	= 0;
	n->db   = (Kvp *)bas_malloc((n->sz) * sizeof(Kvp));
	n->outer= NULL;
	return n;
}

int deldict(Dict *x)
{
	if (dbg) printf ("Delete dictionary\n");
	if (x != NULL)
	{
		for (int i=0; i<x->ip; i++)
		{
			bas_free(x->db[i].key);
			freeobj(&x->db[i].value);
		}
		bas_free(x->db);
		bas_free(x);
		x=NULL;
	}
	return 0;
}

Dict *clonedict(Dict *x)
{
	Dict *r=newdict(x->sz);
	r->ip   = x->ip;
	r->cf	= x->cf;
	r->outer= x->outer;

	for (int i=0; i<r->ip; i++)
	{
		int n = strlen (x->db[i].key) + 1;         
		r->db[i].key = bas_malloc(n*sizeof(char)); 

		strncpy(r->db[i].key, x->db[i].key, n);
		r->db[i].value = cloneObj(x->db[i].value);
	}
	return r;
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


char *covertToUpper(char *str){
    char *newstr, *p;
    p = (newstr = strdup(str));
    while(*p++=toupper(*p));

    return newstr;
}

int dict_add(Dict *d, char *key, tOBJ value)
{
	if (d==NULL) return 0;

	key = covertToUpper(key);

	//if(dbg) printf ("Add %s %d %d\n", key, d->ip, d->sz);
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
	int n = strlen(key) + 1 ;
	d->db[i].key = (char *)bas_malloc(sizeof(char)*n);
	strncpy(d->db[i].key , key, n);
	d->db[i].value = cloneObj(value);
	(d->ip)++;
	return 1;
}
 

// listed sorted for binary search
int dict_find(Dict *d, char *key)
{
	int low=0;
	int high=d->ip-1;

	if (d==NULL) return -1;

	key = covertToUpper(key);

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
		return cloneObj(d->db[i].value);

	return dict_getk(d->outer,key);
}

tOBJ dict_getc(Dict *d, char *key)
{
	int i;
	if (d==NULL) 
		return emptyObj();

	i=dict_find(d, key);

	if (i>=0) 
		return copyObj(d->db[i].value);

	return dict_getc(d->outer,key);
}


tOBJ dict_get(Dict *d, int indx)
{
	if (d==NULL) return emptyObj();	
	if (indx >=0 && indx <d->ip)
		return cloneObj(d->db[indx].value);
	return emptyObj();
}

int dict_updateonly(Dict *d, char *key, tOBJ value)
{
	int i;
	if (d==NULL) return 0;
	i=dict_find(d, key);
	if (i>=0) 
	{
		tOBJ t = d->db[i].value;
		d->db[i].value = cloneObj(value); // clone it?
		freeobj(&t);
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

int dict_size(Dict *d)
{
	if (d==NULL) return 0;
	return d->ip;
}

char *dict_findval(Dict *d, tOBJ x, int n)
{
	for (int i=0; i<d->ip; i++)
	{
		if (compareObj(x, d->db[i].value))
		{
			if (n<=0) return d->db[i].key;
			n--;
		}
	}
	return NULL;
}



