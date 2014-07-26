//kmeans

#ifndef KMEANS_H
#define KMEANS_H

#include "oimg.h"

extern fMatrix *kmeans_mat(fMatrix *fm, int k);
extern fMatrix *kmeans_img(oImage *img, int k, int th);

#endif
