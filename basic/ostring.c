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


#include "oobj.h"

extern int dbg;

/**********************************************************/
/*  strings                                               */
/**********************************************************/

char *newstring(char *s)
{
	char *r;
	int n=strlen(s);
	if (dbg) printf ("New String [%d]\n",n);
	r=(char *)malloc(n*sizeof(char)+1);
	strcpy(r, s);
	return r;
}

char *newstring1(int n)
{
	char *r;
	if (dbg) printf ("New String [%d]\n",n);
	r=(char *)malloc(n*sizeof(char)+1);
	return r;
}

void delstring(char *s)
{
	if (dbg) printf ("Del String \n");
	free(s);
}

tOBJ makestring(char *s)
{
	tOBJ r;
	r.type=STR;
	r.string = newstring(s);
	r.cnt=0;
	return r;
}


