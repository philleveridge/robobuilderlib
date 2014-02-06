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

#include "oobj.h"
#include "ocells.h"
#include "ostring.h"
#include "odict.h"
#include "ostack.h"
#include "fmatrix.h"

#include "opflow.h"

//  OPFLOW [1 2;3 4] [1 2;3 5]

tOBJ OpticFlow(tOBJ im1, tOBJ im2)
{
	//dbg=1;
	int gaus_sigma = 1; 
	tOBJ r=emptyObj();
	if (im1.type != FMAT2 || im1.type != FMAT2)
	{
		return r;
	}
	//%%I precompute the priramyds using my code from the previous assignment
	int width =im1.fmat2->w;
	int height=im1.fmat2->h;

	//%%Derivate Variables:
   
	fMatrix *img1 =im1.fmat2; 
	fMatrix *img2 =im2.fmat2;       
	fMatrix *Dx_1; 
	fMatrix *Dy_1;            
	fMatrix *Dx_2;             
	fMatrix *Dy_2;           
	fMatrix *Ix;              
	fMatrix *Iy;    
	fMatrix *It; 

	//%%Optical flow variables
	int neighborhood_size=5;                     
	fMatrix *A=newmatrix(2, 2);            
	fMatrix *B=newmatrix(1, 2); 
	fMatrix *output1 =newmatrix(width, height);        
	fMatrix *output2 =newmatrix(width, height); 

	//%%Kernel Variables:
	int Kernel_Size = 6*gaus_sigma+1; 
	int i,j,m,n,k = (Kernel_Size-1)/2;  
	float Pi=3.1415;

	fMatrix *gaus_kernel_x = gausian(Kernel_Size, gaus_sigma);
	fMatrix *gaus_kernel_y = ftranspose2(gaus_kernel_x); 

	fMatrix *kernel        = newmatrix(Kernel_Size,Kernel_Size);

	if (dbg) { 
		printf ("GKX\n"); fmatprint2(gaus_kernel_x); printf ("\n"); 
		printf ("GKY\n"); fmatprint2(gaus_kernel_y); printf ("\n"); }
 
	//%%Compute x and y derivates for both images:
 
	Dx_1 = fconvolve(img1, gaus_kernel_x);
	Dy_1 = fconvolve(img1, gaus_kernel_y);
	Dx_2 = fconvolve(img2, gaus_kernel_x);
	Dy_2 = fconvolve(img2, gaus_kernel_y);

	//printf (" %d %d %d %d\n", Dx_1, Dy_1,Dx_2, Dy_2);

	Ix = fmatscale(fadd2(Dx_1, Dx_2, '+' ), 0.5); 
	Iy = fmatscale(fadd2(Dy_1, Dy_2, '+' ), 0.5);

/*	if (dbg) 
	{ 
		printf ("IX\n");  fmatprint2(Ix);    printf ("\n"); 
		printf ("IY\n");  fmatprint2(Iy);    printf ("\n"); 

		printf ("DX2\n"); fmatprint2(Dx_2);  printf ("\n"); 
		printf ("DY2\n"); fmatprint2(Dy_2);  printf ("\n"); 
		printf ("DX1\n"); fmatprint2(Dx_1);  printf ("\n"); 
 		printf ("DY1\n"); fmatprint2(Dy_1);  printf ("\n"); 
	}
*/
	//%%Build a gaussian kernel to smooth images for computing It


	for (i=1; i<=Kernel_Size; i++)
	{
	    	for (j=1; j<=Kernel_Size; j++)
		{
			fset2(kernel, j-1, i-1, (1/(2*Pi*(gaus_sigma*gaus_sigma))) * exp (-((i-k-1)*(i-k-1) + (j-k-1)*(j-k-1))/(2*gaus_sigma*gaus_sigma)));
		}
	}

	fMatrix *sim1 = fconvolve(img1, kernel);
	fMatrix *sim2 = fconvolve(img2, kernel);
	It = fadd2(sim2, sim1, '-' );

	for (i=(neighborhood_size/2); i<height-(neighborhood_size/2); i++)
	{
		for (j=(neighborhood_size/2); j<width-(neighborhood_size/2); j++)
		{
			fmatzero(A);
			fmatzero(B);
  
			for (m=i-(neighborhood_size/2); m<i+(neighborhood_size/2); m++)
			{
				for (n=j-(neighborhood_size/2); n<j+(neighborhood_size/2); n++)
				{
					fset2(A,0,0, fget2(A,0,0) + fget2(Ix,n,m)*fget2(Ix,n,m));
					fset2(A,0,1, fget2(A,0,1) + fget2(Ix,n,m)*fget2(Iy,n,m));
					fset2(A,1,0, fget2(A,1,0) + fget2(Ix,n,m)*fget2(Iy,n,m));
					fset2(A,1,1, fget2(A,1,1) + fget2(Iy,n,m)*fget2(Iy,n,m));

					fset2(B,0,0, fget2(B,0,0) + fget2(Ix,n,m)*fget2(It,n,m));
					fset2(B,0,1, fget2(B,0,1) + fget2(Iy,n,m)*fget2(It,n,m));
				}
			}

        		//Ainv=pinv(A); %%Pseudo inverse
                	//result=Ainv*(-B);
			fMatrix *result = fmultiply2(inverse(A), fmatscale(B,-1));
                	//output1(i,j)=result(1,1);
                	//output2(i,j)=result(2,1);
			fset2(output1,j,i,fget2(result,0,0));
			fset2(output2,j,i,fget2(result,0,1));
		}
	}

	// output1=flipud(output1); % Flip matrix in up/down direction
	// output2=flipud(output2); % Same
	// construct r = (output1 output2)

	if (dbg) ;
	{ 
		printf ("O1\n"); fmatprint2(output1);  printf ("\n"); 
 		printf ("O2\n"); fmatprint2(output2);  printf ("\n");  
	}

	tOBJ t1, t2;
	t1.type=FMAT2;
	t1.fmat2=output1;
	t2.type=FMAT2;
	t2.fmat2=output2;
	r = append (r, t1);
	r = append (r, t2);

	//cleanup

	dbg=0;

	delmatrix(Dy_1);             
	delmatrix(Dx_2);             
	delmatrix(Dy_2);            
	delmatrix(Ix);              
	delmatrix(Iy);    
	delmatrix(It);
	delmatrix(A);            
	delmatrix(B); 
	delmatrix(sim1); 
	delmatrix(sim2); 

	return r;
}

