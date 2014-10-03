#define _CRT_SECURE_NO_DEPRECATE 
#include <stdio.h>
#include <string.h>
//#include <strings.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "linux.h"

#include "oobj.h"
#include "mem.h"
#include "rbmread.h"


int ReadLine(FILE *fp, char *m, int n)
{
	int ch, cn=0;
	while ( (ch=fgetc(fp))>=0  && cn <n-1)
	{
		if (cn==0 && ch==' ') continue;
		m[cn++]=ch;
		if (ch=='13') break;
	}
	m[cn]=0;
	return (ch>=0);
}

char *readfield(char sep, char *s, char *b)
{
	char c;
	while (s!=0 && *s!=0) {
		c=*s++;
		if (c == sep) break;
		*b++=c;
	}
	*b=0;

	return s;
}

void rbmdelete(Motion *m)
{
	if (m != 0)
	{
		if (m->sc != 0) bas_free (m->sc);
		m->sc=0;
		if (dbg) printf ("Del RBM\n");
	}
}


void rbmprint(Motion *m)
{
	int i;
	if (m==NULL)
		return;

	printf ("Name:  %s - ", m->name);
	printf ("Servos-%d, ", m->no_servos);
	printf ("Scenes-%d {", m->no_scenes);

	for (i=0; i<m->no_servos; i++)
	{
		printf ("%d ", m->Postn[i]);			
	}
	printf ("}\n");	
}

Motion *rbmload(char *fn)
{

	/*
	* if we loaded (and wrote out) the .rbm files directly.  There 
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

        char line[MAXC];
        FILE *tr;
	Motion *mot = (Motion *)bas_malloc(sizeof(Motion));
	int Lg=0;
	mot->no_servos=0;
	mot->no_scenes=0;
	
	if ( (tr=fopen(fn,"r"))== 0)
	{
		printf ("? can't find file - %s\n",fn);
		return mot;
	}

        while (1)
        {
		ReadLine(tr, line, MAXC);

		if (dbg) printf ("line - %d\n", (Lg=strlen(line)));

		int i;
		char field[256], *l=&line[0];
	
		for (i=0;i<12; i++)
		{
			l = readfield(':', l, field);
			if (i==6)   strcpy(mot->name,field);
			if (i==11)  mot->no_servos=atoi(field);
			if (i==9)   mot->no_scenes=atoi(field);
		}

		l = readfield(':', l, field);
		l = readfield(':', l, field);


		for (i = 0; i < mot->no_servos; i++)
		{
			l = readfield(':', l, field);  mot->PGain[i]=atoi(field);
			l = readfield(':', l, field);  mot->DGain[i]=atoi(field);
			l = readfield(':', l, field);  mot->IGain[i]=atoi(field);
			l = readfield(':', l, field);  mot->eData[i]=atoi(field);
			l = readfield(':', l, field);  mot->Postn[i]=atoi(field);
			if (dbg) printf ("%d) %d, %d, %d, %d : %d\n", i, mot->PGain[i], mot->DGain[i], mot->IGain[i], mot->eData[i], mot->Postn[i]);

			l = readfield(':', l, field);
		}

		l = readfield(':', l, field);
		l = readfield(':', l, field);

		mot->sc = (Scene *)malloc(sizeof(Scene)*mot->no_scenes);

		for (int ns=0; ns< mot->no_scenes; ns++)
		{
			int nf, tt;
			l = readfield(':', l, field); nf=atoi(field);//Frames
			l = readfield(':', l, field); tt=atoi(field);//TransitionTime
			l = readfield(':', l, field); //?
			l = readfield(':', l, field); //???
			l = readfield(':', l, field); //??

			mot->sc[ns].TransitionTime=tt;
			mot->sc[ns].Frames=nf;

			if (dbg) printf ("[%d] %d %d : ", ns, nf,tt);

			for (int nq=0; nq<mot->no_servos; nq++)
			{
				int pos;
				l = readfield(':', l, field); //pos
				pos=atoi(field);
				l = readfield(':', l, field); //torq
				l = readfield(':', l, field); //edata

				mot->sc[ns].F.Position[nq] = pos;

				if (dbg) printf("%d, ", pos);

				l = readfield(':', l, field); //?
				l = readfield(':', l, field); //???
				l = readfield(':', l, field); //??
			}
			if (dbg) printf ("\n");
		}


		break;

        }
        fclose(tr);
	return mot;
}
