/*
Colour mapper

*/

#include <math.h>
#include <string.h>

#include "express.h"
#include "functions.h"
#include "lists.h"

#include "cmap.h"

extern int dbg;

Run    		rle[50];
Region 		reg[10];
ColourState 	color[10];
int 		colourmap[3][256];

int 		num_runs, 
		num_regions, 
		max_runs=50, 
		max_regions=10, 
		max_area, 
		num_colors=0;

/***************************************

colourmap utils

***************************************/

int mapThresh (int *tmap, int a1, int a2, int cn)
{
	int v=1<<(cn-1);

	for (int i=a1; i<a2; i++)
	{
		tmap[i] |= v;
	}	
	return 0;
}

void clrmap()
{
	for (int i=0; i<256; i++) {
		colourmap[0][i]=0;
		colourmap[1][i]=0;
		colourmap[2][i]=0;
	}
}

void add_thresh(int n, int minR, int maxR, int minG, int maxG, int minB, int maxB)
{
	printf ("Add threshold [%d], (%d,%d) (%d,%d) (%d,%d)\n", n, minR,  maxR,  minG, maxG,  minB,  maxB);

	if (n>=0 && n<8 && minR<maxR && minG<maxG && minB <maxB)
	{
		mapThresh(colourmap[0], minR, maxR, n);
		mapThresh(colourmap[1], minG, maxG, n);
		mapThresh(colourmap[2], minB, maxB, n);
	}
}

int testmap(int r,int g, int b)
{
	return (colourmap[0][r] &  colourmap[1][g] & colourmap[2][b] );
}

/***************************************

Show / debug output utils

***************************************/


void showmap()
{
	for (int i=0; i<256; i++) {
		printf ("%d (%04x %04x %04x)\n", i, colourmap[0][i], colourmap[1][i], colourmap[2][i]);
	}
}

void show_run(int n)
{
	Run *p=&rle[0];
	for (int i=0;i<n;i++)
	{
		printf ("%d) x=%d,y=%d,w=%d,c=%d,p=%d,n=%d\n", i, p->x,p->y,p->width,p->color,p->parent,p->next);
		p++;
	}	
}

void show_reg(int n)
{
	Region *p=&reg[0];
	for (int i=0;i<n;i++)
	{
		printf ("%d) [%d] x1=%d,y1=%d,x2=%d,y2=%d a=%d (%f,%f)\n", 
			i, 
			p->color, 
			p->x1,p->y1,p->x2,p->y2,
			p->area,
			p->cen_x, p->cen_y);
		p++;
	}	
}

void show_colors(int n)
{
	ColourState *c=&color[0];

	for(int i=0; i<num_colors; i++)
	{
		printf ("%s : n=%d [%d %d %d] Area>=%d\n", c->name, c->num, c->color.red, c->color.green, c->color.blue, c->min_area);
		Region *p=c->list;
		while (p != NULL) 
		{
			printf ("   x1=%d,y1=%d,x2=%d,y2=%d\n",p->x1,p->y1,p->x2,p->y2);
			p=p->next;
		}	
	}		
}

void showImage(int n)
{
	switch (n) {
	case 0: printf ("[runs=%d, reg=%d, cols=%d]\n", num_runs, num_regions, num_colors);
		break;
	case 1: show_run(num_runs); 
		break;
	case 2: show_reg(num_regions); 
		break;
	case 3: show_colors(num_colors); 
		break;
	case 4: showmap();
		break;
	}
}


/***************************************

colour state management

***************************************/

void get_color_region(int n)
{	
	if (n<=0 && n>num_colors)
	{
		return;
	}

  	ColourState *c=&color[n-1];

	int t=0;

	//printf ("%s : n=%d [%d %d %d] Area>=%d\n", c->name, c->num, c->color.red, c->color.green, c->color.blue, c->min_area);
	Region *p=c->list;
	while (p != NULL) 
	{
		scene[t++]=p->area;
		scene[t++]=p->x1;
		scene[t++]=p->y1;
		scene[t++]=p->x2;
		scene[t++]=p->y2;
		scene[t++]=(int)(0.5+p->cen_x);
		scene[t++]=(int)(0.5+p->cen_y);
		p=p->next;
	}
	nis=t;		
}

void clear_colors()
{
	printf ("Clr colors\n");
	num_colors=0;
}

int add_color(char *s, int r, int g, int b, int min)
{
	printf ("Add color %s %d,%d,%d >%d\n",s,r,g,b,min);
	int i;
	for (i=0;i<num_colors;i++)
	{
		if (strcmp(s,color[i].name)==0)
		{
			printf ("Colour already exists - try Clr colors\n");
			return -1;
		}
	}

	ColourState *c=&color[num_colors++];
	c->min_area=min;
	strncpy(c->name,s,6);
	*(c->name+6)='\0';
        c->color.red=r; c->color.green=g; c->color.blue=b;
	c->num=0;
	c->list=NULL;
	return num_colors;
}

/**************


***************/

extern unsigned char BMap[];
extern int Height;
extern int Width;
extern int Depth;
extern int *frame;

void ThresholdImage(int sz, int *frame) 
{
     unsigned char *img = &BMap[0];

     if (dbg) printf("threshold img [%d,%d,%d]\n" ,Height, Width,Depth);

     for (int c=0; c<sz*sz; c++) frame[c]=0;

     for (int y=0; y<Height; y++)
     {
	 int j=(sz*y)/Height;
	 for (int x=0; x<Width; x++)
	 {
	     int i=(sz*x)/Width;

	     unsigned char b=*(img++);
	     unsigned char g=*(img++);
	     unsigned char r=*(img++);

	     int v = testmap(r, g, b);
	     frame[j*sz + i] |= v;
	}
     }
}

/***************************************

Encode runs

***************************************/

int EncodeRuns2(unsigned char *img, int height, int width, int max_runs) 
{
//  tmap_t m,save;
//  tmap_t *row;
	int x,y,j,l;
	unsigned char m, save, *row;
	Run r;

	if (dbg) printf("encode runs [%d,%d]\n",height,width);

	m=img[0];
	r.next = 0;
	save=m;

	j = 0;
	for(y=0; y<height; y++)
	{
		row = &img[y * width];

		// restore previous terminator and store next
		// one in the first pixel on the next row
		row[0] = save;
		save = row[width];
		row[width] = 255;

		r.y = y;

		x = 0;

		while(x < width)
		{
			m = row[x];
			r.x = x;

			l = x;
			while(row[x] == m) x++;

			if(m!=0 || x>=width)
			{
				r.color = m; 
				r.width = x - l;
				r.parent = j;
				rle[j++] = r;

				if (dbg) printf("run (%d,%d):%d %d\n",r.x,r.y,r.width,r.color);

				if(j >= max_runs)
				{
					row[width] = save;
					return(j);
				}
			}
		}
	}
	return j;
}


int EncodeRuns(int height, int width, int max_runs) //rmap,cmap,img.width,img.height,max_runs);
{
//  tmap_t m,save;
//  tmap_t *row;
	int x,y,j,l;
	int m, save, *row, *map;
	Run r;

	if (dbg) printf("encode runs [%d,%d]\n",height,width);

	map=&scene[0];
	m=scene[0];
	r.next = 0;

	// initialize terminator restore
	save = map[0];

	j = 0;
	for(y=0; y<height; y++)
	{
		row = &map[y * width];

		// restore previous terminator and store next
		// one in the first pixel on the next row
		row[0] = save;
		save = row[width];
		row[width] = 255;

		r.y = y;

		x = 0;

		while(x < width)
		{
			m = row[x];
			r.x = x;

			l = x;
			while(row[x] == m) x++;

			if(m!=0 || x>=width)
			{
				r.color = m; 
				r.width = x - l;
				r.parent = j;
				rle[j++] = r;

				if (dbg) printf("run (%d,%d):%d %d\n",r.x,r.y,r.width,r.color);

				if(j >= max_runs)
				{
					row[width] = save;
					return(j);
				}
			}
		}
	}
	return j;
}

/***************************************

 Connect components using four-connecteness so that the runs each
 identify the global parent of the connected region they are a part
 of.  It does this by scanning adjacent rows and merging where
 similar colors overlap.  Used to be union by rank w/ path
 compression, but now is just uses path compression as the global
 parent index is a simpler rank bound in practice.
 WARNING: This code is complicated.  I'm pretty sure it's a correct
   implementation, but minor changes can easily cause big problems.
   Read the papers on this library and have a good understanding of
   tree-based union find before you touch it

***************************************/

void ConnectComponents(int num) //rmap,num_runs)

{
  int l1,l2;
  Run r1,r2;
  int i,j,s;

  Run *map = rle;

  if (dbg) printf("connect !\n");

  // l2 starts on first scan line, l1 starts on second
  l2 = 0;
  l1 = 1;
  while(map[l1].y == 0) l1++; // skip first line

  // Do rest in lock step
  r1 = map[l1];
  r2 = map[l2];
  s = l1;
  while(l1 < num){
    
    if (dbg) {
	printf("%6d:(%3d,%3d,%3d) %6d:(%3d,%3d,%3d)\n",
	   l1,r1.x,r1.y,r1.width,
	   l2,r2.x,r2.y,r2.width);
	}
    

    if(r1.color==r2.color && r1.color){
      // case 1: r2.x <= r1.x < r2.x + r2.width
      // case 2: r1.x <= r2.x < r1.x + r1.width
      if((r2.x<=r1.x && r1.x<r2.x+r2.width) ||
	 (r1.x<=r2.x && r2.x<r1.x+r1.width)){
        if(s != l1){
          // if we didn't have a parent already, just take this one
          map[l1].parent = r1.parent = r2.parent;
          s = l1;
        }else if(r1.parent != r2.parent){
          // otherwise union two parents if they are different

          // find terminal roots of each path up tree
          i = r1.parent;
          while(i != map[i].parent) i = map[i].parent;
          j = r2.parent;
          while(j != map[j].parent) j = map[j].parent;

          // union and compress paths; use smaller of two possible
          // representative indicies to preserve DAG property
          if(i < j){
	    map[j].parent = i;
            map[l1].parent = map[l2].parent = r1.parent = r2.parent = i;
          }else{
            map[i].parent = j;
            map[l1].parent = map[l2].parent = r1.parent = r2.parent = j;
          }
        }
      }
    }

    // Move to next point where values may change
    i = (r2.x + r2.width) - (r1.x + r1.width);
    if(i >= 0) r1 = map[++l1];
    if(i <= 0) r2 = map[++l2];
  }

  // Now we need to compress all parent paths
  for(i=0; i<num; i++){
    j = map[i].parent;
    map[i].parent = map[j].parent;
  }
}

// sum of integers over range [x,x+w)
int range_sum(int x,int w)
{
  return(w*(2*x + w-1) / 2);
}

#define min(a, b)  (a<b)?a:b
#define max(a, b)  (a>b)?a:b

/***************************************

 Takes the list of runs and formats them into a region table,
 gathering the various statistics along the way.  num is the number
 of runs in the rmap array, and the number of unique regions in
 reg[] (bounded by max_reg) is returned.  Implemented as a single
 pass over the array of runs.

***************************************/

int ExtractRegions(int max_reg, int num) //reg,max_regions,rmap,num_runs);
{
  int b,i,n,a;
  Run r;
  Run *rmap = rle;
  n = 0;

  if (dbg) printf("extract !\n");

  for(i=0; i<num; i++){
    if(rmap[i].color){
      r = rmap[i];
      if(r.parent == i){
        // Add new region if this run is a root (i.e. self parented)
        rmap[i].parent = b = n;  // renumber to point to region id
        reg[b].color = r.color;
        reg[b].area = r.width;
        reg[b].x1 = r.x;
        reg[b].y1 = r.y;
        reg[b].x2 = r.x + r.width;
        reg[b].y2 = r.y;
        reg[b].cen_x = range_sum(r.x,r.width);
        reg[b].cen_y = r.y * r.width;
	reg[b].run_start = i;
	reg[b].iterator_id = i; // temporarily use to store last run
        n++;
        if(n >= max_reg) return(max_reg);
      }else{
        // Otherwise update region stats incrementally
        b = rmap[r.parent].parent;
        rmap[i].parent = b; // update parent to identify region id
        reg[b].area += r.width;
        reg[b].x2 = max(r.x + r.width,reg[b].x2);
        reg[b].x1 = min((int)r.x,reg[b].x1);
        reg[b].y2 = r.y; // last set by lowest run
        reg[b].cen_x += range_sum(r.x,r.width);
        reg[b].cen_y += r.y * r.width;
	// set previous run to point to this one as next
	rmap[reg[b].iterator_id].next = i;
	reg[b].iterator_id = i;
      }
    }
  }

  // calculate centroids from stored sums
  for(i=0; i<n; i++){
    a = reg[i].area;
    reg[i].cen_x = (float)reg[i].cen_x / a;
    reg[i].cen_y = (float)reg[i].cen_y / a;
    rmap[reg[i].iterator_id].next = 0; // -1;
    reg[i].iterator_id = 0;
    reg[i].x2--; // change to inclusive range
  }

  return(n);
}

/***************************************

 Splits the various regions in the region table a separate list for
 each color.  The lists are threaded through the table using the
 region's 'next' field.  Returns the maximal area of the regions,
 which can be used later to speed up sorting.

***************************************/
int SeparateRegions(ColourState *color, int colors, int num)
{
  Region *p;
  int i; // ,l;
  int c;
  int area,max_area;

  if (dbg) printf("separate regions %d,%d!\n",num,colors);

  // clear out the region list head table
  for(i=0; i<colors; i++){
    color[i].list = (void *)0;
    color[i].num  = 0;
  }

  // step over the table, adding successive
  // regions to the front of each list
  max_area = 0;
  for(i=0; i<num; i++){
    p = &reg[i];
    c = p->color-1;
    area = p->area;

    if(area >= color[c].min_area){
      if(area > max_area) max_area = area;
      color[c].num++;
      p->next = color[c].list;
      color[c].list = p;
    }
  }

  return(max_area);
}


/***************************************

// Sorts a list of regions by their area field.
// Uses a linked list based radix sort to process the list.

***************************************/

#define CMV_RBITS 6
#define CMV_RADIX (1 << CMV_RBITS)
#define CMV_RMASK (CMV_RADIX-1)

Region *SortRegionListByArea(Region *list,int passes)
{
  Region *tbl[CMV_RADIX],*p,*pn;
  int slot,shift;
  int i,j;

  // Handle trivial cases
  if(!list || !list->next) return(list);

  // Initialize table
  for(j=0; j<CMV_RADIX; j++) tbl[j] = NULL;

  for(i=0; i<passes; i++){
    // split list into buckets
    shift = CMV_RBITS * i;
    p = list;
    while(p){
      pn = p->next;
      slot = ((p->area) >> shift) & CMV_RMASK;
      p->next = tbl[slot];
      tbl[slot] = p;
      p = pn;
    }

    // integrate back into partially ordered list
    list = NULL;
    for(j=0; j<CMV_RADIX; j++){
      p = tbl[j];
      tbl[j] = NULL; // clear out table for next pass
      while(p){
        pn = p->next;
        p->next = list;
        list = p;
        p = pn;
      }
    }
  }

  return(list);
}

/***************************************

// Sorts entire region table by area, using the above
// function to sort each threaded region list.

***************************************/

void SortRegions(ColourState *color,int colors,int max_area)
{
  int i,p;
  if (dbg) printf("sort regions %d !\n",max_area);

  // do minimal number of passes sufficient to touch all set bits
  p = 0;
  while(max_area != 0){
    max_area >>= CMV_RBITS;
    p++;
  }

  // sort each list
  for(i=0; i<colors; i++){
    color[i].list = SortRegionListByArea(color[i].list,p);
  }
}


/***************************************

Depreacted - new code moved to oImage library

***************************************/

void output_frame(int sz)
{
	for (int i=0; i<sz; i++)
	{
		for (int j=0; j<sz; j++)
		{
		     printf ("%3d ", frame[j+i*sz]);
		}
		printf ("\n");
	}
}

int processFrame(int sz, int *data)
{
  int x=0;

  frame=data;

  if (sz==0)
  {
      if (nis>0 ) x=sqrt(nis);
      if (x>0 && x*x != nis) {
        printf("Error! Current array must be square.\n");
        return(1);
      }
  }
  else
  {
      x=sz;
      ThresholdImage(x, data); 
      if (dbg) output_frame(x);
  }

  num_runs = EncodeRuns(x,x,max_runs); 

  if(num_runs == max_runs){
    printf("WARNING! Exceeded maximum number of runs in EncodeRuns.\n");
  }

  if (dbg) show_run(num_runs);

  ConnectComponents(num_runs); 

  if (dbg) show_run(num_runs);

  num_regions = ExtractRegions(max_regions, num_runs); 

  if (dbg) show_reg(num_regions);

  for(int i=0; i<num_colors; i++){
    color[i].list = NULL;
    color[i].num  = 0;
  }

  if(num_regions == max_regions){
    printf("WARNING! Exceeded maximum number of regions in ExtractRegions.\n");
    return(1);
  }

  max_area = SeparateRegions(&color[0], num_colors,num_regions); 
  SortRegions(&color[0], num_colors, max_area); 

  return(0);
}





