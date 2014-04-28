#ifndef MEM_H
#define MEM_H

extern unsigned int 	mem_counter;
extern int 		memlc;

extern void *bas_malloc (size_t size);
extern void *bas_realloc(size_t size);
extern void  bas_free   (void *ptr);
extern void  bas_show();
extern int   bas_mem();

extern void  testme();
#endif
