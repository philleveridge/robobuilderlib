/**********   lists *************************/
#ifndef LISTS_H
#define LISTS_H

#ifdef WIN32
//Bigger for MATRIX processing support
#define SCENESZ 10000   //8K
#define MAXLIST 26 
#define LISTMEM 65536  //64K
#endif

#ifdef IMAGE
//Bigger for image processing support
#define SCENESZ 10000   //8K
#define MAXLIST 26 
#define LISTMEM 65536  //64K
#endif

#ifdef AVR
#define SCENESZ 128
#define MAXLIST 5
#define LISTMEM 128
#endif


extern int  scene[];	  // generic array
extern int  nis;

extern char arrayname;
extern int  listread   (char ln, int n);
extern void listdelete (char ln, int indx1, int indx2, int selflg);
extern void listset    (char ln, int ind, long v, int insrtflg);
extern int  listreadc  (char ln);
extern int  list_eval  (char, char *p, int);
extern int  listsize   (char ln);
extern void listinit   ();
extern void listdump   ();
extern int *listarray  (char ln);
//extern int  listcreate (char ln, int size, int type);
extern int  listcreate (char ln, int size);
extern void listcopy   (char ln1, char ln2);

#endif
