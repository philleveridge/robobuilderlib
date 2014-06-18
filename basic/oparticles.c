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
#include "oparticles.h"

const double steering_noise    = 0.1;
const double distance_noise    = 0.03;
const double measurement_noise = 0.3;
const double pi                = 3.1415926;
const double twopi             = 6.28318530718;
 
double rgauss(double mean, double variance)
{
	static int hasSpare = 0;
	static double rand1, rand2;

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

/**********************************************************/
/*  turtles                                               */
/**********************************************************/

tTurtlep turtle_make(float x, float y, float h)
{
	tTurtlep  p;
	if (dbg); printf ("Make Turtle %f %f %f\n",x,y,h);

	p=(tTurtlep)bas_malloc(sizeof(tTurtle));
	p->x 		    = x;
	p->y 		    = y;
	p->orientation 	    = h;
	p->steering_noise   = steering_noise;
	p->distance_noise   = distance_noise;
	p->measurement_noise= measurement_noise;
	p->length           = 0.5;
	return p;
}

tTurtlep turtle_clone(tTurtlep p)
{
	if (dbg); printf ("Clone Turtle\n");
	tTurtlep  np=(tTurtlep)bas_malloc(sizeof(tTurtle));
	np->x 			= p->x;
	np->y 			= p->y;
	np->orientation 	= p->orientation;
	np->steering_noise   	= p->steering_noise;
	np->distance_noise   	= p->distance_noise;
	np->measurement_noise	= p->measurement_noise;
	np->length          	= p->length ;
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
	printf ("turtle %f %f %f", p->x, p->y, p->orientation);
}

void turtle_set(tTurtlep p, double a, double b, double c) 
{
	p->x 		= a;
	p->y 		= b;
	p->orientation 	= fmod(c, twopi);
}

void turtle_set_noise(tTurtlep p, double new_s_noise, double new_d_noise, double new_m_noise) 
{
	p->steering_noise   = new_s_noise;
	p->distance_noise   = new_d_noise;
	p->measurement_noise= new_m_noise;
}

tTurtlep turtle_move(tTurtlep o, double steering, double distance) 
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

double turtle_sense_x(tTurtlep o) 
{
        return rgauss(o->x, o->measurement_noise);
}

double turtle_sense_y(tTurtlep o) 
{
        return rgauss(o->y, o->measurement_noise);
}

double turtle_measure_prob(tTurtlep p, double measurement_x, double measurement_y) 
{
        //compute errors
        double error_x = measurement_x - p->x;
        double error_y = measurement_y - p->y;

        //calculate Gaussian
        double error =  exp(- pow(error_x,2) / pow(p->measurement_noise, 2) / 2.0) / sqrt(twopi * pow(p->measurement_noise, 2));
               error *= exp(- pow(error_y,2) / pow(p->measurement_noise, 2) / 2.0) / sqrt(twopi * pow(p->measurement_noise, 2));
        return error;
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

int turtle_check_goal(tTurtlep p, double goal_x, double goal_y, double threshold) 
{
	// default threshold=1.0
	if (p==NULL) return 0;
	return (sqrt((goal_x-p->x)*(goal_x-p->x) + (goal_y-p->y)*(goal_y-p->y))<threshold);
}



/**********************************************************/
/*  particles                                             */
/**********************************************************/

tParticlep particles_make(int N)
{
	int i;
	if (dbg); printf ("Make Particle\n");
	tParticlep p=(tParticlep)bas_malloc(sizeof(tParticle));
	p->N=N;
	p->turtle_data = (tTurtlep *)bas_malloc(sizeof(tTurtlep)*N);
	for (i=0; i<N; i++)
	{
		p->turtle_data[i]=turtle_make(0.0,0.0,0.0);
	}	
	return p;
}

tParticlep particles_clone(tParticlep p)
{
	int i;
	if (dbg); printf ("Clone Particle\n");

	tParticlep np   = (tParticlep)bas_malloc(sizeof(tParticle));
	np->N           = p->N;
	np->turtle_data = (tTurtlep *)bas_malloc(sizeof(tTurtlep)*p->N);

	for (i=0; i<p->N; i++)
	{
		np->turtle_data[i]=turtle_make(p->turtle_data[i]->x, p->turtle_data[i]->y, p->turtle_data[i]->orientation);
	}

	return np;
}

void particles_del(tParticlep p)
{
	if (dbg) printf ("Del Particles\n");
	if (p != NULL)
	{
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

void particles_sense(tParticlep p, fMatrix *Z) 		
{
	int i;
	if (p==NULL || Z==NULL) return;
/*
        w = []
        for i in range(self.N):
            w.append(self.data[i].measurement_prob(Z))
*/
	for (i=0; i<p->N; i++)
	{
		printf ("%d %f\n", i, turtle_measure_prob(p->turtle_data[i], Z->fstore[0], Z->fstore[1]));
	}

/*
        // resampling (careful, this is using shallow copy)
        p3 = []
        index = int(random.random() * self.N)
        beta = 0.0
        mw = max(w)

        for i in range(self.N):
            beta += random.random() * 2.0 * mw
            while beta > w[index]:
                beta -= w[index]
                index = (index + 1) % self.N
            p3.append(self.data[index])
        self.data = p3
*/
}

/* -------

motion of the particles
    def move(self, grid, steer, speed):

---------*/

tParticlep particles_move(tParticlep o, double steer, double speed, fMatrix *grid ) 		
{
	int i;
	tParticlep p = particles_make(o->N);
	for (i=0; i<p->N; i++)
	{
		p->turtle_data[i] = turtle_move(o, steer, speed);
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




