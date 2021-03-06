/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   faceDetection.cpp
 *
 *  Author          :   Francesco Comaschi (f.comaschi@tue.nl)
 *
 *  Date            :   November 12, 2012
 *
 *  Function        :   Main function for face detection
 *
 *  History         :
 *      12-11-12    :   Initial version.
 *
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  If not, see <http://www.gnu.org/licenses/>
 *
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Happy coding!
 */

#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "stdio-wrapper.h"
#include "haar.h"

#define INPUT_FILENAME "Face.pgm"
#define OUTPUT_FILENAME "Output.pgm"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

extern int dbg;
int    vj     (char *f);
int    vj_img (int w, int h, int max, unsigned char *A);
MyRect vj_get (int i) ;

#ifdef __cplusplus
}
#endif

extern int iter_counter;

int initf=0;

void vj_init()
{
	if (initf==0)
	{
		initf=1;
		if (dbg) printf("-- read text Classifier --\r\n");
		readTextClassifier();
	}
}

void vj_Del()
{
	if (initf)
	{
		/* delete image and free classifier */
		releaseTextClassifier();
		initf=0;
	}
}

std::vector<MyRect>  vj (MyImage imageObj) 
{
	int mode = 1;
	int i;
	MyImage *image = &imageObj;

	if (dbg) printf("-- entering main function --\r\n");
	vj_init();

	/* detection parameters */
	float scaleFactor = 1.2;
	int minNeighbours = 1;

	if (dbg)printf("-- loading cascade classifier --\r\n");

	myCascade cascadeObj;
	myCascade *cascade = &cascadeObj;
	MySize minSize = {20, 20};
	MySize maxSize = {0, 0};

	/* classifier properties */
	cascade->n_stages=25;
	cascade->total_nodes=2913;
	cascade->orig_window_size.height = 24;
	cascade->orig_window_size.width = 24;

	if (dbg) printf("-- detecting faces --\r\n");

	iter_counter=0;

	std::vector<MyRect> result = detectObjects(image, minSize, maxSize, cascade, scaleFactor, minNeighbours);

	return result;
}

std::vector<MyRect> lastresult;

MyRect vj_get(int i) {return lastresult[i];}

int vj_img (int w, int h, int max, unsigned char *A) 
{
	if (dbg) printf("-- entering main -img function --\r\n");

	vj_init();

	MyImage imageObj;
	imageObj.width    =w;
	imageObj.height   =h;
	imageObj.data     =A;
	imageObj.maxgrey  =max;
	imageObj.flag     =1;

	lastresult = vj (imageObj);

	return lastresult.size();
}

int vj (char *file)  // pgm file
{
	int i, mode = 1;

	if (dbg) printf("-- loading image --\r\n");

	MyImage imageObj;
	MyImage *image = &imageObj;

	if (readPgm(file, image) == -1)
	{
		printf( "Unable to open input image %s\n",file);
		return 1;
	}

	vj_init();

	std::vector<MyRect> result = vj (imageObj);

	for(i = 0; i < result.size(); i++ )
	{
		MyRect r = result[i];
		drawRectangle(image, r);
	}

	if (dbg) printf("-- saving output --\r\n"); 
	writePgm((char *)OUTPUT_FILENAME, image); 
	if (dbg) printf("-- image saved --\r\n");

	vj_Del();
	freeImage(image);
	return 0;
}
