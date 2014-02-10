#ifndef MEM_H
#define MEM_H

extern void *bas_malloc (int);
extern void *bas_realloc(int);
extern void *bas_free   (int);
extern void  bas_show();
extern int   bas_mem();

extern void  testme();
#endif
