/*
Colour mapper

*/


int mapThresh (int *tmap, int a1, int a2, int cn)
{
	int v=1<<cn;

	for (int i=a1; i<a2; i++)
	{
		tmap[i] |= v;
	}	
	return 0;
}

int colourmap[3][256];

int initmap()
{
	for (int i=0; i<256; i++) {colourmap[0][i]=0;colourmap[1][i]=0;colourmap[2][i]=0;}
	// 
	mapThresh(colourmap[0], 20, 50, 1);
	mapThresh(colourmap[1], 50,150, 1);
	mapThresh(colourmap[2], 10, 70, 1);

	mapThresh(colourmap[0], 70, 90, 2);
	mapThresh(colourmap[1], 60, 80, 2);
	mapThresh(colourmap[2],  0, 40, 2);

	return 0;
}

int testmap(int r,int g, int b)
{
	return (colourmap[0][r] &  colourmap[1][g] & colourmap[2][b] );
}

/*************

Find and map reqions

based on CM v2.1

**************/

typedef struct rgb{
  unsigned char red,green,blue;
} RGB;

typedef struct run{
  short x,y,width;    // location and width of run
  int color;          // which color(s) this run represents
  int parent,next;    // parent run and next run in run list
} Run;

typedef struct image {
  unsigned char *buf;
  int width,height,pitch;
  int field;
} Image;


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
  char *name;        // color's meaningful name (e.g. orange ball, goal)
} ColourState ;

void ThresholdImage() //cmap,img,tmap)
{
}

int EncodeRuns() //rmap,cmap,img.width,img.height,max_runs);
{
	return 0;
}

void ConnectComponents() //rmap,num_runs)
{
}

int ExtractRegions() //reg,max_regions,rmap,num_runs);
{
	return 0;
}

int SeparateRegions() //color,num_colors,reg,num_regions);
{
	return 0;
}

void SortRegions() //color,num_colors,max_area)
{
}

int processFrame()
{
  int num_runs, num_regions, max_runs, max_regions, max_area, num_colors=0;
  ColourState color[10];

  ThresholdImage(); //cmap,img,tmap);

  num_runs = EncodeRuns(); //rmap,cmap,img.width,img.height,max_runs);

  if(num_runs == max_runs){
    printf("WARNING! Exceeded maximum number of runs in CMVision::EncodeRuns.\n");
  }

  ConnectComponents(); //rmap,num_runs);

  num_regions = ExtractRegions(); //reg,max_regions,rmap,num_runs);

  for(int i=0; i<num_colors; i++){
    color[i].list = (Region *)0;
    color[i].num  = 0;
  }

  if(num_regions == max_regions){
    printf("WARNING! Exceeded maximum number of regions in CMVision::ExtractRegions.\n");
    return(1);
  }

  max_area = SeparateRegions(); //color,num_colors,reg,num_regions);
  SortRegions(); //color,num_colors,max_area);

  return(0);
}





