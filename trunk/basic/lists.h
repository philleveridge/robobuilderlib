/**********   lists *************************/
#ifndef LISTS_H
#define LISTS_H

#define SCENESZ 128
#define MAXLIST 5
#define LISTMEM 128

extern int				scene[];	  // generic array
extern int				nis;

extern char arrayname;
extern int  listread   (char ln, int n);
extern void listdelete (char ln, int indx1, int indx2, int selflg);
extern void listset    (char ln, int ind, long v, int insrtflg);
extern int  listreadc  (char ln);
extern int  list_eval  (char, char *p, int);
extern int  listsize   (char ln);
extern void listinit   ();
extern void listdump   ();

#endif
