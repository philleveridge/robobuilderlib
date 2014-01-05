
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "linux.h"

typedef struct scene
{
	char name[32];
	int TransitionTime;
	int Frames;
	int mPositions;
	int mTorque;
	int mExternalData;
} Scene;

char *ReadLine(FILE *fp, char *m)
{
	int ch, cn=0;
	while ( (ch=fgetc(fp))>=0 )
	{
		if (cn==0 && ch==' ') continue;
		m[cn++]=ch;
		if (ch=='13') break;
	}
	m[cn]=0;
printf("%s\n", m);
	return (ch>=0);
}

int loadrbm(char *fn)
{
	int* PGain;
	int* DGain;
	int* IGain;
	int* Position;
	int* extData;
	char name[256];

        Scene *scenes;

        int no_servos = 0;
	int no_scenes = 0;

	/*
	*  How about if we loaded (and wrote out) the .rbm files directly.  There 
	basically just ascii files - using ': ' separators here's one (The 
	simple wave - HunoDemo_Hi)
	Ive broken down (and added some comments as to what I think the format). 
	The actual file is one very long string (attached)

	1:13:0000000000000:XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXX:11:  (* File header Version Info etc )

	[6] 
	HunoDemo_Hi:0:1:5:2:16:4:   (* header : Filename, Flag?, Flag?, No_scenes, ?, No Servos, ? *)
	[13]
	00:020:030:000:0:125:       (* Initial position for each servo = servo ID, Pgain, Rgain, Igain, ext, Posiiton)
	[19]
	*  01:020:030:000:0:201:
	02:020:030:000:0:163:
	03:020:030:000:0:067:
	........
	[109]
	000:07:Scene_0:10:500:   (* Scene header:  ?, ?, name, 10 Frames, 500ms)
	[114]
	*  0:00:125:125:2:0:        (*, ?, Servo ID, Start Pos, end Pos, Torque, ext Port)
	0:01:179:179:2:0:
	0:02:199:199:2:0:        (* repeat for each servo *)

	* .... repeat for each scene ...........
	* 
	*/

        char line[1024];
        FILE *tr;

	if ( (tr=fopen(fn,"r"))== 0)
	{
		printf ("? can't find file - %s\n",fn);
		return 0;
	}


        while (ReadLine(tr, line))
        {
	printf ("line - %s\n", line);
/*
            line = line.Trim();
            string[] a = line.Split(':');

            name = a[6];
            no_servos = Convert.ToInt16(a[11]);
            no_scenes = Convert.ToInt16(a[9]);

            rprintf ("Motion file: %s\n", a[6]);
            rprintf ("Servos: %d\n", no_servos);
            rprintf ("Scenes: %d\n", no_scenes);

            IGain    = int[no_servos];
            DGain    = int[no_servos];
            PGain    = int[no_servos];
            Position = int[no_servos];
            extData  = int[no_servos];
            scenes   = new Scene[no_scenes];

            int c = 13; // start of 00 postion

            for (int i = 0; i < no_servos; i++)
            {
                PGain[i]    = (int)(a[c + 1 + i * 6]);
                DGain[i]    = (int)(a[c + 2 + i * 6]);
                IGain[i]    = (int)(a[c + 3 + i * 6]);
                extData[i]  = (int)(a[c + 4 + i * 6]);
                Position[i] = (int)(a[c + 5 + i * 6]);
            }

            c = c + no_servos*6 + 2 ; // start of scene

            int s = 0;

            while (c + no_servos * 6 + 5 <= a.Length + 1)
            {
                if (s == no_scenes) break;

                rprintf ("Scene %s %s\n", s, a[c]);

                scenes[s]                = new Scene();
                strncpy(scenes[s].name,a[c],32);
                scenes[s].TransitionTime = (int)(a[c + 2]);
                scenes[s].Frames         = (int)(a[c + 1]);

                scenes[s].mExternalData  = new uint[no_servos];
                scenes[s].mPositions     = new uint[no_servos];  // scene end positions
                scenes[s].mTorque        = new uint[no_servos];

                for (int i = 0; i < no_servos; i++)
                {
                    scenes[s].mPositions[i]    = (int)(a[c + 6 + i * 6]);
                    scenes[s].mTorque[i]       = (int)(a[c + 7 + i * 6]);
                    scenes[s].mExternalData[i] = (int)(a[c + 8 + i * 6]);
                }
                c += no_servos * 6 + 5;
                s = s + 1;
            }
*/
        }
        fclose(tr);
}
