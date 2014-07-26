
/*
% Harris detector
% The code calculates
% the Harris Feature Points(FP) 
*/

#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "win.h"
#endif

#ifdef LINUX
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "linux.h"
#endif

#include "harris.h"


fMatrix *harris_detect(oImage * im)
{
	fMatrix *r=NULL;
	int nop=1;

	r=newmatrix(2,nop);

	return r;
}



