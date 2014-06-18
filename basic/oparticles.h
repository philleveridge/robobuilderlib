#ifndef O_PART_H
#define O_PART_H

#include "oobj.h"
#include "fmatrix.h"

typedef struct Turtles   {
	float x;
	float y;
	float orientation;
	float length;
	float steering_noise;
	float distance_noise;
	float measurement_noise;

	int   num_collisions;
	int   num_steps;
} tTurtle, *tTurtlep;


typedef struct Particles {
	int N;
	float steering_noise;
	float distance_noise;
	float measurement_noise;
	tTurtlep *turtle_data;
} tParticle, *tParticlep;

extern double 		rgauss			(double mean, double variance);

extern tTurtlep 	turtle_make		(float x, float y, float h) ;
extern tTurtlep 	turtle_clone		(tTurtlep p) ;
extern void 		turtle_del		(tTurtlep p) ;

extern void 		turtle_print		(tTurtlep p) ;
extern void 		turtle_set		(tTurtlep p, double a, double b, double c) ;
extern void 		turtle_set_noise	(tTurtlep p, double new_s_noise, double new_d_noise, double new_m_noise) ;

extern tTurtlep 	turtle_move		(tTurtlep o, double steering, double distance) ;
extern double 		turtle_sense_x		(tTurtlep o) ;
extern double 		turtle_sense_y		(tTurtlep o) ;
extern double 		turtle_measure_prob	(tTurtlep p, double measurement_x, double measurement_y) ;

extern int 		turtle_check_collision	(tTurtlep p, fMatrix *grid) ;
extern int 		turtle_check_goal	(tTurtlep p, double goal_x, double goal_y, double threshold) ;


/**********************************************************/
/*  particles                                             */
/**********************************************************/

extern tParticlep 	particles_make		(int N) ;
extern tParticlep	particles_clone		(tParticlep p) ;
extern void 		particles_del		(tParticlep p) ;

extern void 		particles_print		(tParticlep p) ;
extern void 		particles_sense		(tParticlep p, fMatrix *Z) ;
extern tParticlep	particles_move		(tParticlep p, double steer, double speed, fMatrix *grid ) ;	
extern void 		particles_get_position	(tParticlep p, float *xp, float *yp, float *op) ;

#endif
