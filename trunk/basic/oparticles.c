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

#include "mem.h"
#include "ocells.h"
#include "oparticles.h"

const double pi                = 3.1415926;
const double twopi             = 6.28318530718;
 
double rgauss(double mean, double variance)
{
	static int hasSpare = 0;
	static double rand1, rand2;

	if (variance==0.0) return mean;

	if(hasSpare)
	{
		hasSpare = 0;
		return mean + sqrt(variance * rand1) * sin(rand2);
	}
 
	hasSpare = 1;
 
	rand1 = rand() / ((double) RAND_MAX);
	if(rand1 < 1e-100) rand1 = 1e-100;
	rand1 = -2 * log(rand1);
	rand2 = (rand() / ((double) RAND_MAX)) * twopi;
 
	return mean + sqrt(variance * rand1) * cos(rand2);
}

double Gaussian(double mu, double sigma, double  x)
{        
        //calculates the probability of x for 1-dim Gaussian with mean mu and var. sigma
        return exp(- pow((mu - x),2) / pow(sigma,2) / 2.0) / sqrt(twopi * pow(sigma,2));
}

/**********************************************************/
/*  Plans                                                 */
/**********************************************************/

extern tOBJ ocar(tOBJ);
extern tOBJ ocdr(tOBJ);
extern tOBJ ocons(tOBJ  r, tOBJ a);
extern tOBJ olen (tOBJ a);

tOBJ gpath(int x, int y, int mx, int my)
{
	tOBJ r=emptyObj();
	tOBJ c=emptyObj();
	tOBJ t=emptyObj();
	if (x>0	)   { c = cnvtIItoList(x-1,y); t = ocons(c, r); freeobj(&r); freeobj(&c); r=t;}
	if (y>0	)   { c = cnvtIItoList(x,y-1); t = ocons(c, r); freeobj(&r); freeobj(&c); r=t;}
	if (x<mx-1) { c = cnvtIItoList(x+1,y); t = ocons(c, r); freeobj(&r); freeobj(&c); r=t;}
	if (y<my-1) { c = cnvtIItoList(x,y+1); t = ocons(c, r); freeobj(&r); freeobj(&c); r=t;}
	return r;	
}

tOBJ  Choice(tOBJ f) 
{
        //(FOREACH x f  (IF (< (CADR x) MP) (SETQ MP (CADR x) BS x) ))   BS)
	int mp=99;
	tOBJ bs=emptyObj();
	while (f.type==CELL)
	{
		tOBJ x=ocar(f);
		f=ocdr(f);
		if (toint(ocar(ocdr(x))) < mp)
		{
			mp=toint(ocar(ocdr(x)));
			freeobj(&bs);
			bs=x;
		}
	}
	return bs;
}

tOBJ  Remove(tOBJ a, tOBJ b) 
{
	//(FOREACH z b (IF (NOT (= (CAR z) (CAR a))) (SETQ nn (CONS z nn))))nn)
	tOBJ nn=emptyObj();
	tOBJ t1=emptyObj();
	tOBJ z=emptyObj();
	while (b.type==CELL)
	{
		z=ocar(b);
		b=ocdr(b);
		if (!compareObj(ocar(z), ocar(a)))
		{
			t1 = ocons(z, nn);
			freeobj(&nn);
			nn=t1;
		}
	}
	return nn;
}

int heuristic(int x, int y, int gx, int gy)
{
	return abs(x-gx)+abs(y-gy);
}

fMatrix *Astar(fMatrix *grid, int sx, int sy, int gx, int gy)
{
	// Input MxM matrix Output - 2 x N matrix
	// PLAN 'SMOOTH
	// PLAN [ 0 1 0 ; 0 1 0 ; 0 0 0] '(0 0) '(2 2)
	// PLAN [ 0 1 0 ; 0 1 0 ; 0 0 0] '(0 0) '(0 1)
	// SETQ M [0 0 1 0 0 0 ; 0 0 1 0 0 0 ; 0 0 0 1 0 0  ; 0 0 0 0 0 1; 0 0 0 0 0 0]
	// PLAN 'PRINT M
	// SETQ P (PLAN M '(0 0) '(5 1))
	// PLAN 'PRINT M P
	// PLAN 'SMOOTH P '(0.1 0.1)

	tOBJ ns, top, action, t, t1,t2,t3;
	fMatrix *explored = newmatrix(grid->w,grid->h);
		
	if (grid == NULL) return NULL;
	t1=cnvtIItoList(sx, sy);
	t2=makeint(0);
	t3=cnvtOOtoList(t1,t2);
	tOBJ frontier = ocons(t3,emptyObj()) ;
	tOBJ path     = emptyObj();
	freeobj(&t1);
	freeobj(&t2);
	freeobj(&t3);

	while (frontier.type!=EMPTY)
	{
		top    = cloneObj(Choice(frontier));
		action = emptyObj();

		t1 = Remove(top,frontier);
		freeobj(&frontier);
		frontier = t1;

		int x    = toint(ocar(ocar(top)));
		int y    = toint(ocar(ocdr(ocar(top))));
		int cost = toint(ocar(ocdr(top)));

		fset2(explored,x,y,1.0);

		if (x==gx && y==gy) 
		{
			int i;
			t1=olen(path);
			fMatrix *r=newmatrix(2,toint(t1)+1);
			fset2(r,0,r->h-1, gx);
			fset2(r,1,r->h-1, gy);
			t2=path;
			for (i=r->h-2; i>=0; i--)
			{
				fset2(r,0,i, tofloat(ocar(ocar(path))));
				fset2(r,1,i, tofloat(ocar(ocdr(ocar(path)))));
				path=ocdr(path);
			}
			delmatrix(explored);
			freeobj(&frontier);
			freeobj(&top);	
			freeobj(&t2);
			freeobj(&t1);
			return r;
		}

		freeobj(&path);
		path = ocar(ocdr(ocdr(top)));

		action = gpath (x,y, grid->w, grid->h);

		cost = cost + 1 + heuristic(x,y, gx, gy);

		t1 = ocons(ocar(top),path);
		freeobj(&path);
		path=t1;

		//printf ("Cost length=%d\n",cost);
		//println("path=",path);

		tOBJ a1=action;

		while (action.type==CELL)
		{
			ns      = ocar(action);
			action  = ocdr(action);

			int nx=toint(ocar(ns));
			int ny=toint(ocar(ocdr(ns)));

			if (fget2(explored,nx,ny)==0.0 && fget2(grid,nx,ny)==0.0) 
			{
				t1=cnvtIItoList(nx, ny);
				t2=makeint(cost);
				t3=cnvtOOOtoList(t1, t2, path);
				t = ocons(t3,frontier) ;
				freeobj(&frontier);
				freeobj(&t1);
				freeobj(&t2);
				freeobj(&t3);
				frontier=t;
			}
			//freeobj(&ns);
		}
		freeobj(&a1);
		freeobj(&top);
	}
	printf ("No path to goal\n");
	delmatrix(explored);
	return NULL;
}
//PLAN 'PRINT [0 0 0; 0 1 0 ; 0 0 0] [0 0;0 1;0 2;1 2; 2 2]
// S _ _ 
// v # _ 
// > > G 
void plan_print(fMatrix *grid, fMatrix *plan)
{
	int i, j;
	if (grid==NULL) return;
	fMatrix *gc=fmatcp(grid);
	if (plan != NULL)
	{
		//0 0 ; 0 1 ; 0 2
		for (i=1; i<plan->h-1; i++)
		{
			char c=' ';
			int dx=fget2(plan,0,i)-fget2(plan,0,i+1);
			int dy=fget2(plan,1,i)-fget2(plan,1,i+1);
			if (dx==0 && dy> 0) c='^';
			if (dx==0 && dy< 0) c='v';
			if (dx> 0 && dy==0) c='<';
			if (dx< 0 && dy==0) c='>';
			fset2(gc,(int)fget2(plan,0,i),(int)fget2(plan,1,i),(float)c);
		}
	}
	for (i=0; i<gc->h; i++)
	{
		for (j=0; j<gc->w; j++)
		{
			if      (plan!=NULL && fget2(plan,0,0)==j && fget2(plan,1,0)==i) 			printf("S ");
			else if (plan!=NULL && fget2(plan,0,plan->h-1)==j && fget2(plan,1,plan->h-1)==i) 	printf("G ");
			else if (fget2(gc,j,i)==1.0) 								printf("# ");
			else if (fget2(gc,j,i)>1.0) 								printf("%c ", (char)fget2(gc,j,i));
			else 											printf("_ ");
		}
		printf ("\n");
	}
	delmatrix(gc);
}


fMatrix *smooth(fMatrix *xi, float alpha, float beta, float tolerance)
{
	// 2 x N matrix	
	// Yi (x,y) += alpha*(xi-yi) + beta*(y(i+1) +  y(i-1) - 2*yi)
	int i,j;
	float change=1.0;

	if (xi==NULL) return NULL;

	if (dbg) printf ("smooth %d,%d %f %f\n", xi->w, xi->h, alpha, beta);

	fMatrix *yi=fmatcp(xi);

	while (change>tolerance)
	{
		change=0.0;
		for (j=1; j<(xi->h)-1; j++)
		{
			for (i=0; i<xi->w; i++)
			{
				float v  = fget2(yi,i,j);
				float ax = alpha * (fget2(xi,i,j) - v );
				float bx = beta  * (fget2(yi,i+1,j) + fget2(yi,i-1,j) - 2*v);
				fset2(yi, i, j, (v + ax + bx));
				change  += fabs(ax+bx);
			}
		}
	
	}
	return yi;
}

/**********************************************************/
/*  turtles                                               */
/**********************************************************/

double steering_noise    = 0.0;
double distance_noise    = 0.0;
double measurement_noise = 0.0;
double length            = 0.5;
double drift             = 0.0;

void set_params(float new_s_noise, float new_d_noise, float new_m_noise, float new_length, float new_drift) 
{
	steering_noise   = new_s_noise;
	distance_noise   = new_d_noise;
	measurement_noise= new_m_noise;
	length           = new_length;
	drift            = new_drift;
}

tTurtlep turtle_make(float world_size, float x, float y, float h)
{
	tTurtlep  p;
	if (dbg) printf ("Make Turtle %f %f %f\n",x,y,h);

	p=(tTurtlep)bas_malloc(sizeof(tTurtle));
	p->world_size	    = world_size;
	p->x 		    = x;
	p->y 		    = y;
	p->orientation 	    = h;
	p->steering_noise   = steering_noise;
	p->distance_noise   = distance_noise;
	p->measurement_noise= measurement_noise;
	p->length           = length;
	p->drift            = drift;
	return p;
}

tTurtlep turtle_make_random(float world_size)
{
	if (world_size<=0.0) world_size=1.0;
	float x = world_size * (rand() / ((double) RAND_MAX));
	float y = world_size * (rand() / ((double) RAND_MAX));
	float h = twopi      * (rand() / ((double) RAND_MAX));
	return turtle_make(world_size, x, y, h);
}

tTurtlep turtle_clone(tTurtlep p)
{
	if (dbg) printf ("Clone Turtle\n");
	tTurtlep  np=(tTurtlep)bas_malloc(sizeof(tTurtle));
	np->world_size	    	= p->world_size;
	np->x 			= p->x;
	np->y 			= p->y;
	np->orientation 	= p->orientation;
	np->steering_noise   	= p->steering_noise;
	np->distance_noise   	= p->distance_noise;
	np->measurement_noise	= p->measurement_noise;
	np->length          	= p->length ;
	np->drift 		= p->drift ;
	return np;
}

void turtle_del(tTurtlep p)
{
	if (dbg) printf ("Del Turtle\n");
	if (p != NULL)
	{
		bas_free(p);
	}
}

void turtle_print(tTurtlep p) 
{
	printf ("turtle %f %f %f",    p->x, p->y, p->orientation);
	if (dbg); printf (" : %f | noise  %f %f %f", p->world_size, p->steering_noise, p->distance_noise, p->measurement_noise);
}

void turtle_set(tTurtlep p, float a, float b, float c) 
{
	p->x 		= fmod(a, p->world_size);
	p->y 		= fmod(b, p->world_size);
	p->orientation 	= fmod(c, twopi);
}

void turtle_set_drift(tTurtlep p, float a) 
{
	p->drift 	= a;
}

void turtle_set_noise(tTurtlep p, float new_s_noise, float new_d_noise, float new_m_noise) 
{
	p->steering_noise   = new_s_noise;
	p->distance_noise   = new_d_noise;
	p->measurement_noise= new_m_noise;
}

tTurtlep turtle_simple_move(tTurtlep o, float steering, float distance) 
{
	tTurtlep p=turtle_clone(o);

        if (distance < 0.0) distance = 0.0;
	steering += o->drift;

        // apply noise
        double steering2 = rgauss(steering, p->steering_noise);
        double distance2 = rgauss(distance, p->distance_noise);

        //Execute motion

	p->orientation = fmod(o->orientation + steering2, twopi);
	p->x           = fmod(o->x + distance2 * cos(p->orientation),p->world_size);
	p->y           = fmod(o->y + distance2 * sin(p->orientation),p->world_size);

	return p;
}

tTurtlep turtle_move(tTurtlep o, float steering, float  distance) 
{
	double tolerance = 0.001, max_steering_angle = pi / 4.0;
	tTurtlep p=turtle_clone(o);

        if (steering > max_steering_angle)   	steering = max_steering_angle;
        if (steering < -max_steering_angle)  	steering = -max_steering_angle;
        if (distance < 0.0) 			distance = 0.0;

        // apply noise
        double steering2 = rgauss(steering, p->steering_noise);
        double distance2 = rgauss(distance, p->distance_noise);

        //Execute motion
        double turn = tan(steering2) * distance2 / p->length;

        if (abs(turn) < tolerance)
	{
            //approximate by straight line motion
            p->x +=  (distance2 * cos(p->orientation));
            p->y +=  (distance2 * sin(p->orientation));
            p->orientation += fmod (turn, twopi); 
	}
        else
	{
            //approximate bicycle model for motion
            double radius  = distance2 / turn;
            double cx      = p->x - (sin(p->orientation) * radius);
            double cy      = p->y + (cos(p->orientation) * radius);
            double to      = fmod((p->orientation + turn), twopi);
            p->x           = cx   + (sin(p->orientation) * radius);
            p->y           = cy   - (cos(p->orientation) * radius);
            p->orientation = to;
	}
        //check for collision
        //turtle_heck_collision(p, grid)

	return p;
}

void turtle_position(tTurtlep o, float *xp, float *yp, float *op) 
{
	float x = rgauss(o->x, o->measurement_noise);	
	float y = rgauss(o->y, o->measurement_noise);	
	float h = rgauss(o->y, o->measurement_noise);	
	*xp=x;
	*yp=y;
	*op=h;
}

fMatrix *turtle_sense(tTurtlep o, fMatrix *landmarks)
{
	int i;
	fMatrix *sr = newmatrix(1, landmarks->h);
	if (landmarks==NULL || landmarks->w != 2) return NULL;
	for (i=0; i<landmarks->h; i++)
	{
		double lx   = fget2(landmarks,0,i);
		double ly   = fget2(landmarks,1,i);
		double dist = sqrt(pow((o->x)-lx,2)+pow((o->y)-ly,2));
		dist        = dist + rgauss(0.0, o->measurement_noise);
		fset2(sr,0,i,dist);
	}	
	return sr;
}

double turtle_measure_prob(tTurtlep p, float measurement_x, float measurement_y) 
{
	double r = Gaussian(measurement_x, p->measurement_noise, p->x) * Gaussian(measurement_y, p->measurement_noise, p->y);
	return r;
}

double turtle_measure_prob2(tTurtlep p, fMatrix *measurement, fMatrix *landmarks) 
{
	int i;
	double prob=1.0;
	if (landmarks==NULL || landmarks->w!=2 || measurement==NULL || measurement->h != landmarks->h ) return 0.0;
	for (i=0; i<landmarks->h; i++)
	{
		double lx   = fget2(landmarks,0,i);
		double ly   = fget2(landmarks,1,i);
		double dist = sqrt(pow((p->x)-lx,2)+pow((p->y)-ly,2));
		prob *= Gaussian(dist, p->measurement_noise, fget2(measurement,0,i)) ;
	}	
	return prob;
}

int turtle_check_collision(tTurtlep p, fMatrix *grid) 
{
	int i,j;
	if (p==NULL || grid==NULL) return 0;
	for (i=0; i<grid->h; i++)
	{
		for (j=0; j<grid->w; j++)
		{
			if (fget2(grid,j,i) == 1.0) 
			{
				if (sqrt((p->x-(float)j)*(p->x-(float)j) + (p->y-(float)i)*(p->y-(float)i))<0.5) 
				{
					p->num_collisions++;
					return 1;
				}
			}
		}
	}
	return 0;
}

int turtle_check_goal(tTurtlep p, float goal_x, float goal_y, float threshold) 
{
	// default threshold=1.0
	if (p==NULL) return 0;
	return (sqrt((goal_x-p->x)*(goal_x-p->x) + (goal_y-p->y)*(goal_y-p->y))<threshold);
}



/**********************************************************/
/*  particles                                             */
/**********************************************************/

tParticlep particles_make2(int N, float x, float y, float h, float worldsize)
{
	int i;
	if (dbg); printf ("Make Particle\n");
	tParticlep p=(tParticlep)bas_malloc(sizeof(tParticle));
	p->N=N;
	p->world_size=worldsize;
	p->turtle_data = (tTurtlep *)bas_malloc(sizeof(tTurtlep)*N);
	for (i=0; i<N; i++)
	{
		p->turtle_data[i]=turtle_make(worldsize,x,y,h);
	}	
	return p;
}

tParticlep particles_make_random(int N, float worldsize)
{
	int i;
	if (dbg); printf ("Make random Particle\n");
	tParticlep p=(tParticlep)bas_malloc(sizeof(tParticle));
	p->N=N;
	p->turtle_data = (tTurtlep *)bas_malloc(sizeof(tTurtlep)*N);
	for (i=0; i<N; i++)
	{
		p->turtle_data[i]=turtle_make_random(worldsize);
	}	
	return p;
}

tParticlep particles_make(int N, float worldsize)
{
	return particles_make2(N, 0.0,0.0,0.0, worldsize);
}

tParticlep particles_clone(tParticlep p)
{
	int i;
	if (dbg); printf ("Clone Particle\n");

	tParticlep np   = (tParticlep)bas_malloc(sizeof(tParticle));
	np->N           = p->N;
	np->world_size	= p->world_size;
	np->turtle_data = (tTurtlep *)bas_malloc(sizeof(tTurtlep)*p->N);

	for (i=0; i<p->N; i++)
	{
		np->turtle_data[i]=turtle_clone(p->turtle_data[i]);
	}

	return np;
}

void particles_del(tParticlep p)
{
	if (dbg) printf ("Del Particles\n");
	if (p != NULL)
	{
		int i;
		for (i=0; i<p->N; i++)
		{
			turtle_del(p->turtle_data[i]);
		}
		bas_free(p->turtle_data);
		bas_free(p);
	}
}

void particles_print(tParticlep p)
{
	int i;
	printf ("Particles %d\n",p->N);
	for (i=0; i<p->N; i++)
	{
		printf ("%d)", i); turtle_print(p->turtle_data[i]); printf ("\n"); 
	}
}

/* -------

sensing and resampling
    def sense(self, Z):

---------*/

tParticlep particles_sense(tParticlep o, fMatrix *Z) 		
{
	int i;
	if (o==NULL || Z==NULL) return NULL;

#ifndef WIN32
	float mp[o->N];
#else
	float mp[10000];
#endif

	float mw=0.0;


	for (i=0; i<o->N; i++)
	{
		if (Z->h==1)
		{
			mp[i] = turtle_measure_prob(o->turtle_data[i], Z->fstore[0], Z->fstore[1]);
		}
		else
		{
//			mp[i] = turtle_measure_prob2(o->turtle_data[i], m, Z);
		}
		if (mp[i]>mw) mw=mp[i];
		printf ("%d %f\n", i, mp[i] );

	}
	printf ("Max %f\n", mw );

        // resampling - 

	tParticlep p = particles_make(o->N,o->world_size);
	int index = (int)((rand() / ((double) RAND_MAX)) * o->N);
	double beta=0.0;
	for (i=0; i<o->N; i++)
	{
		beta += ((rand() / ((double) RAND_MAX)) * 2.0*mw);
		while (beta> mp[index])
		{
			beta -= mp[index];
			index = (index+1) % (p->N);
		}
		p->turtle_data[i] = turtle_clone(o->turtle_data[index]);
	}
	return p;	
}

/* -------

motion of the particles
    def move(self, grid, steer, speed):

---------*/

tParticlep particles_move(tParticlep o, double steer, double speed, fMatrix *grid, int t ) 		
{
	int i;
	tParticlep p = particles_make(o->N,o->world_size);
	for (i=0; i<p->N; i++)
	{
		if (t)
			p->turtle_data[i] = turtle_move(o->turtle_data[i], steer, speed);
		else
			p->turtle_data[i] = turtle_simple_move(o->turtle_data[i], steer, speed);

	}
	return p;
}

/* -------

extract position from a particle set

---------*/

void particles_get_position(tParticlep p, float *xp, float *yp, float *op) 	
{
	int   i;
        float x = 0.0;
        float y = 0.0;
        float o = 0.0;

	for (i=0; i<p->N; i++)
	{
            x += p->turtle_data[i]->x;
            y += p->turtle_data[i]->y;
            // orientation is tricky because it is cyclic. By normalizing
            // around the first particle we are somewhat more robust to
            // the 0=2pi problem
            o += (fmod(p->turtle_data[i]->orientation - p->turtle_data[0]->orientation + pi , twopi)
                            + p->turtle_data[0]->orientation - pi);

	}
	*xp = x/p->N;
	*yp = y/p->N;
	*op = o/p->N;
}




