#ifndef CMAP_H
#define CMAP_H

/*************

Find and map reqions

based on CM v2.1

**************/

#include "oobj.h"
#include "ocells.h"

typedef struct rgb{
  unsigned char red,green,blue;
} RGB;

typedef struct run{
  short x,y,width;    // location and width of run
  int color;          // which color(s) this run represents
  int parent,next;    // parent run and next run in run list
} Run;

typedef struct region{
  int color;         // id of the color
  int x1,y1,x2,y2;   // bounding box (x1,y1) - (x2,y2)
  float cen_x,cen_y; // centroid
  int area;          // occupied area in pixels
  int run_start;     // first run index for this region
  int iterator_id;   // id to prevent duplicate hits by an iterator
  struct region *next;      // next region in list
} Region;

typedef struct color_class_state {
  Region *list;      // head of region list for this color
  int num;           // number of regions of this color
  int min_area;      // minimum area for a meaningful region
  RGB color;         // example color (such as used in test output)
  char name[10];     // color's meaningful name (e.g. orange ball, goal)
} ColourState ;

extern int 		EncodeRuns		(int height, int width, int max_runs) ;
extern int 		EncodeRuns2		(unsigned char *img, int height, int width, int max_runs) ;

extern void 		ConnectComponents	(int num) ;

extern int 		ExtractRegions		(int max_reg, int num);
extern int 		SeparateRegions		(ColourState *color, int colors, int num) ;
extern void 		SortRegions		(ColourState *color,int colors,int max_area);
extern Region *		SortRegionListByArea	(Region *list,int passes);

extern int 		testmap			(int r,int g, int b);

extern void 		show_run		(int n);
extern void 		show_reg		(int n);

extern tOBJ  		color_regions		();

extern int  		no_regions		();
extern int 		no_colours		();

extern ColourState 	color[];

#define MAX_RUNS    50
#define MAX_REGIONS 10
#define MAX_COLOURS 10



#endif
