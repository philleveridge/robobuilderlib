#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include "win.h"
	int l1o[20];
	int l2o[20];
	int l3o[20];
#endif

#ifdef LINUX
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "linux.h"
#endif

#ifdef AVR
#include <avr/eeprom.h> 
#include <avr/pgmspace.h>
#include "main.h"
#include "global.h"
#include "macro.h"
#include "wckmotion.h"
#include "rprintf.h"

int abs(int);

#endif


#include "edit.h"
#include "express.h"
#include "functions.h"
#include "lists.h"

extern int dbg;
extern int speed;
extern int BasicErr;
extern BYTE cpos[];  // for motor-neuron support

int simple_network(int noi, int noo, int nl1, int nl2, int nl3, int ofset, int flg)
{
	int j;
	int agen = (flg&128); // age neurons
	int moton = (flg&64); // motonuron outputs
	int comp = (flg&32); // comparator inputs
	int rinp = (flg&16); // randomised inputs
	int sho  = (flg&8);   // show output
	int i=ofset;
	int t,cpn;

#ifndef WIN32
	int l1o[nl1];
	int l2o[nl2];
	int l3o[nl3];
#endif

	flg  = flg & 7;   // 0, 1, 2, 3  or 4 (sigmoid mode)

	if (nl2==0)
		ofset= ofset* (((noi+1)*nl1) + ((nl1+1)*nl3));
	else
		ofset= ofset* (((noi+1)*nl1) + ((nl1+1)*nl2) + ((nl2+1)*nl3));

	if (sho)
	{
		rprintf("NOI   = %d\n", noi);
		rprintf("NOO   = %d\n", noo);
		rprintf("Flags = %d\n", flg);
		rprintf("NO L1 = %d\n", nl1);
		rprintf("NO L2 = %d\n", nl2);
		rprintf("NO L3 = %d\n\n", nl3);

		if (ofset>0) rprintf("Offset = %d (%d)\n\n", i,ofset);
	}

	t=noi+noo+ofset-1; // index through weights and threshold

	cpn = (rinp!=0)?(noi/nl1):noi;
	if (sho) rprintf("Conn Per INPUT NEURON = %d\n", cpn);
	for (i=0; i<nl1; i++)
	{
		int s=0;
		if (sho) rprintf("INPUT NEURON = %d\n", i+1);
		for (j=0; j<cpn; j++)
		{
			t=t+1;
			if (comp)

				s += abs(scene[j]-scene[t]);
			else
				s += scene[j]*scene[t];
			if (sho) rprintf("Input=%d (%d x %d)\n",j,scene[j],scene[t]);
		}
		t++;
		if (comp)
		{
			if (sho) rprintf("%d<=(%d)\n",s,scene[t]);
			l1o[i]=(s<=scene[t])?1:0;
		}
		else
		{
			if (sho) rprintf("%d-(%d)\n",s,scene[t]);
			s -= scene[t];
			l1o[i]=sigmoid(s,flg);
		}
		if (sho) rprintf("O%d=%d\n", i+1,l1o[i]);
	}

	for (i=0; i<nl2; i++)
	{
		int s=0;
		if (sho) rprintf("HIDDEN NEURON = %d\n", i+1);
		for (j=0; j<nl1; j++)
		{
			t++;
			s += l1o[j]*scene[t];
			rprintf("Input=%d (%d x %d)\n", j,l1o[j],scene[t]);
		}
		t++;
		if (sho) rprintf("Th=-(%d)\n",scene[t]);

		s -= scene[t];

		l2o[i]=sigmoid(s,flg);
		if (sho) rprintf("OH=%d\n", l2o[i]);
	}

	for (i=0; i<nl3; i++)
	{
		int inp, s=0;
		if (moton && sho) rprintf("MOTOR NEURON = %d\n", i+1);
		else if (sho) rprintf("OUTPUT NEURON = %d\n", i+1);

		if (nl2==0)
		{
			for (j=0; j<nl1; j++)
			{
				t++;
				inp=l1o[j];
				s += inp*scene[t];

				if (sho) rprintf("Input=%d (%d x %d)\n",j,inp,scene[t]);
			}
		}
		else
		{
			for (j=0; j<nl2; j++)
			{
				t++;
				inp=l2o[j];
				s += inp*scene[t];

				if (sho) rprintf("Input=%d (%d x %d)\n",j,inp,scene[t]);
			}
		}

		if (moton)
		{
			int v, sn=scene[noi+i];
			sn=(sn<0) ?0 :sn;
			sn=(sn>30)?30:sn;

			v=wckPosRead(sn);
			if (v>=0)
				cpos[sn]=(BYTE)v;
			else
				rprintf("Servo read fail\n");

			l3o[i]=cpos[sn]+s;
			l3o[i]=(l3o[i]>254)?254:l3o[i];
			l3o[i]=(l3o[i]<0)  ?0  :l3o[i];

			if (sho) 
				rprintf("(%d) Th=%d\nSO=%d\n", sn, cpos[sn], l3o[i]);
		}		
		else
		{
			t++;
			if (sho) rprintf("Th=-(%d)\n",scene[t]);
			s -= scene[t];
			l3o[i]=sigmoid(s,flg);
			if (sho) rprintf("OO=%d\n", l3o[i]);
		}


	}

	for (i=0; i<noo; i++)
	{
		if (moton)

		{
			int sn=scene[noi+i];
			sn=(sn<0) ?0 :sn;
			sn=(sn>30)?30:sn;

			if (nl3>0)
			{
				if (sho) rprintf("Servo %d=%d\n", sn,l3o[i]);
				if (!dbg) wckPosSend(sn,speed,l3o[i]);
			}
			else
			{
				rprintf("?No L3 servo specified\n");
			}
		}
		else
		{
			if (nl3==0)
				scene[noi+i]=l1o[i]; // if no output layer use layer 1
			else
				scene[noi+i]=l3o[i];

			if (sho) rprintf("FO%d=%d\n", i+1,scene[noi+i]);
		}
	}

	//firelist
	if (agen)
	{
rprintf("t=%d\n",t);
		for (i=0; i<nl1; i++)  {t++; scene[t] += l1o[i]; rprintf("%d, ",l1o[i]); }
rprintf("t=%d\n",t);
		for (i=0; i<nl2; i++)  {t++; scene[t] += l2o[i]; rprintf("%d, ",l2o[i]); }
rprintf("t=%d\n",t);
		for (i=0; i<nl3; i++)  {t++; scene[t] += l3o[i]; rprintf("%d, ",l3o[i]); }
		rprintf("\n");
rprintf("t=%d\n",t);
		if (nis<=t) nis=t+1;
	}
	
	return 0;
}

int cmd_network(line_t ln)
{

	char *p=ln.text;
	int  i, param[7],noi,noo,flg,nl1,nl2,nl3,offset;
	long t2;

	// NETWORK  @Input, @output, @weight1 [, @weight2, @weight3,] flags
	if (*p=='@')
	{
		char mi,mo,mw1;
		int i;
		p++;
		mi=*p++;
		if (p[0]!="," && p[1]!='@') {BasicErr=3; return 0;}
		p+=2;
		mo=*p++;
		if (p[0]!="," && p[1]!='@') {BasicErr=3; return 0;}
		mw1=*p++;
		if (p[0]!=",") {BasicErr=3; return 0;}
		eval_expr(&p, &t2);
		flg=t2;
		noi=listsize(mi);
		noo=listsize(mo);
		nl3=0;
		nl2=0;
		nl1=noi;
		offset=0;
		//insert into scene[]

		//TBC

		// now egenrate network

		if (simple_network(noi, noo, nl1, nl2, nl3, offset, flg)<0)
			return 0;

		for (i=0; i<noo; i++)
		{
			listwrite(mo,i,scene[noi+i]);
		}
		return 0;
	}

	// NETWORK  [no inputs],[no outputs],[flgs],[nn ly1],[nn ly2],[nl3], [offset]
	// @! =I1 .. IN  O1 .. OM  W11 ..T1  WNM  .. TN

	for (i=0; i<7; i++)
	{
		param[i]=0;
		eval_expr(&p, &t2);
		param[i]=(int)t2;
		if (*p==0 && i>5) break;
		if (*p==',' && i<6)
		{
			p++;
		}
		else if (*p !=0)
		{
			BasicErr=3; break;
		}
	}
	// code here
	noi	=param[0];
	noo	=param[1];
	flg	=param[2];
	nl1	=param[3];
	nl2	=param[4];
	nl3	=param[5];
	offset	=param[6];

	if (noi<=0 || noo<=0)
	{
		// number input & output >0
		rprintfProgStr(PSTR("Err:: Input and output must be gt 0\n"));
		BasicErr=3;
		return 0;
	}

	if (nl3!= noo && nl3!=0)
	{
		//layer 3 = number outputs
		rprintfProgStr(PSTR("Err:: layer 3 neurons must match output (or be zero)\n"));
		BasicErr=3;
		return 0;
	}

	if (nl3==0 && nl2!=0)
	{
		//layer 3 = number outputs
		rprintfProgStr(("Err:: layer 3 must be non-zero if layer 2 has nodes \n"));
		BasicErr=3;
		return 0;
	}

	return simple_network(noi, noo, nl1, nl2, nl3, offset, flg);

}
