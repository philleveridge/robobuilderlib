#ifndef RBMREAD_H
#define RBMREAD_H


#define MAXC 1024*8

typedef struct frame
{
	int Position[24];
	int Torq[24];
	int eData[24];
} Frame;


typedef struct scene
{
	char name[32];
	int TransitionTime;
	int Frames;
	Frame F;

} Scene;

typedef struct motion
{
	char name[32];
	int no_servos;
	int no_scenes;

	int PGain[24];
	int DGain[24];
	int IGain[24];
	int eData[24];
	int Postn[24];

	Scene *sc;

} Motion;

extern void 	rbmprint	(Motion *m);
extern void 	rbmdelete	(Motion *m);
extern Motion 	rbmload		(char *fn);



#endif
