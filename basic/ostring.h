#ifndef OSTRING_H
#define OSTRING_H

#define MAXSTRING 1024

/**********************************************************/
/*  strings                                              */
/**********************************************************/

extern char *newstring	(char *s);
extern char *newstring1	(int n);
extern void delstring	(char *s);
extern tOBJ makestring	(char *s);
extern tOBJ makenumfstr	(char *s);
extern void upperstring	(tOBJ r);
extern void printstring	(FILE *fp, char *cp);
#endif
