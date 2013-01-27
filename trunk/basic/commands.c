#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include "win.h"

#define MAX_FOR_NEST	10
#define MAX_GOSUB_NEST	50
#define MAX_FOR_EXPR	60 

#endif

#ifdef LINUX
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "linux.h"

#define MAX_FOR_NEST	10
#define MAX_GOSUB_NEST	50
#define MAX_FOR_EXPR	60 

#endif

#ifdef AVR
#include <avr/eeprom.h> 
#include <avr/pgmspace.h>

#include "main.h"
#include "global.h"
#include "macro.h"
#include "wckmotion.h"
#include "rprintf.h"

#define MAX_FOR_NEST	6
#define MAX_GOSUB_NEST	5
#define MAX_FOR_EXPR	20 

#endif

#define MAX_TOKEN 		8

#include "edit.h"
#include "express.h"
#include "functions.h"
#include "lists.h"


void record	(int mode);
void playback	(int mode);

static int  forptr[MAX_FOR_NEST];   			  	// nested for/next
static char nxtptr[MAX_FOR_NEST][MAX_FOR_EXPR];   	// nested for/next
static int  fp; 

static int gosub[MAX_GOSUB_NEST];  					// nested gosubs
static int gp;

extern int BasicErr;
extern int dbg;

int speed=2;
int mtype=2;

int cs=0;
int lnc[20];         //line cache of pointers

void init_cmdptr()
{
	gp=0;
	fp=0;
	cs=0;
	BasicErr=0;
}

int findcache(int l)
{
	int i=0;
	while (i<cs)
	{
		if (lnc[i]==l)
		{
			return lnc[i+1];
		}
		i+=2;
	}
	return 0;
}

void linenocache(int l, int m)
{
	if (findcache(l)==0) // hit?
	{
		//add to cache
		if (cs<18)
		{
			lnc[cs]=l;
			lnc[cs+1]=m;
			cs +=2;
		}
	}
}

int gotoln(int gl)
{
	int p;

	if ((p=findcache(gl))>0)
	{
		return p;
	}
	p = findln(gl);
	if (p<3 || getlineno(p) != gl ) return -1; // no such line
	linenocache(gl,p);
	return p;
}

/*************************************************************************************************************

           RUNTIME routines

*************************************************************************************************************/

int cmd_dummy(line_t ln)
{
	char *p=ln.text;	
	rprintf ("Dummy -[%c] [%s]\n", ln.var, p);
	return 0;
}

int cmd_for(line_t l)
{
	char *p=l.text;
	char *to;
	long n=0;

 	forptr[fp] = nxtline; 	// remember where next instruction is
	// eval expr1 of line.text = "expr1 TO expr2"
	// i.e var=expr1

	to = strstr(p," TO ");
	if (to==0) 
		BasicErr=1;
	else
	{
		strncpy(nxtptr[fp],to+4, MAX_FOR_EXPR);
		*to='\0'; // null terminate expr1
		switch (eval_expr(&p, &n))
		{
			case ARRAY:
				n=listsize(arrayname);
			case NUMBER:
				setvar(l.var, n);
				break;	
			default:
				BasicErr=1;
				return -1;
		}	
	}
	fp++;
	return 0;
}

int cmd_next(line_t l)
{
	int t_ptr;
	long n;
	char *p=l.text;
	// increment var and check condition
	if (fp<1)
	{
		return -1;
	}
	t_ptr=forptr[fp-1];

	// increment var
	setvar(l.var, getvar(l.var) + (long)1);

	// test against expr2 i.e var<=expr2
	n=0;
	p=nxtptr[fp-1];

	switch (eval_expr(&p, &n))
	{
		case NUMBER: 
			break;
		case ARRAY: 
			n=listsize(arrayname);
			break;
		default:
			BasicErr=1;
			return -1;
	}
			
	if (getvar(l.var) <= n) 
	{ 
		// if true set ptr=stack; 
		setline(t_ptr); 
		return 0; //tmp=1
	}	
	else
	{
		fp--;
	}
	return 0;
}

int cmd_let(line_t l)
{		
	long n=0;
	char *p=l.text;

	if (l.var=='@')
	{
		cmd_inset(l);
		return 0;
	}
	switch(eval_expr(&p, &n))
	{
	case NUMBER:
		setvar(l.var,n);
		break;
	case ARRAY:
		//LET A=@B set A to size of Array B
		setvar(l.var,listsize(arrayname));
		break;
	default:
		BasicErr=1;
		break;
	}
	return 0;
}

extern WORD send_hex_str(char *bus_str, int n);
extern WORD send_hex_array(int *p, int n);

int cmd_print(line_t l)
{
	long n=0;
	char *p=l.text;

	while (1)
	{
		if (*p=='\0') break; // done

		switch (eval_expr(&p, &n))
		{
		case NUMBER:
			rprintf ("%ld", n); //COMPLEX FORM
			break;
		case STRING:
			n=str_expr(p);
			if (l.var==1)
			{
				WORD w=send_hex_str(p, n);
				scene[0]=w/256;
				scene[1]=w%256;
				nis=2;
			}
			else if (l.var==2)
			{
				send_bus_str(p, n);
			}
			else
			{
				rprintfStrLen(p,0,n);
			}
			p=p+n+1;
			break;
		case ARRAY:
			if (listsize(arrayname)>0)
			{
				if (l.var==1)
				{
					// send to wxkbus
					WORD w=send_hex_array(scene,nis);
					scene[0]=w/256;
					scene[1]=w%256;
					nis=2;
				}
				else
				{
					int k,sz; long w=0;
					if (*p==':')
					{
						p++;
						if (eval_expr(&p, &w) != NUMBER)
						{
							BasicErr=3; break;
						}
					}

					sz=listsize(arrayname);
					if (w<=0)  w=sz;
					if (w>sz) w=sz;
					for (k=0; k<=(sz/w); k++)
					{
						if (k*w<sz) rprintf ("%d", listread(arrayname, k*w));
						for (n=1; (n<w) && ((k*w+n)<listsize(arrayname)); n++) {rprintf (",%d",listread(arrayname, k*w+n));}
						if (k*w<sz) rprintfCRLF();
					}
				}
			}
			else
				rprintfStr("Null");
		}

		if (*p=='\0') break; // done

		if (*p==',') rprintfChar(' ');

		if (*p!=';' && *p!=',') {
			BasicErr=4; //rprintf ("synatx prob= [%d]\r\n", *p); // DEBUG
			break;
		}
		if (*p==';' && *(p+1)=='\0') // last one
			break;
		p++;
	}
	// check for last ; (no crlf)
	if (*p!=';')	
		rprintfCRLF (); 			
	return 0;
}

extern int nos;
int cmd_stand(line_t l)
{
	long n=0;
	char *p=l.text;

	if (*p != '\0' && eval_expr(&p, &n)==NUMBER)
	{
		standup((int)n); 	
	}
	else
	{
		standup(nos);
	}
	return 0;
}

int cmd_goto(line_t l)		
{
	int t = gotoln(l.value);
	if (t<0)
	{
		BasicErr=3; 
		return 0xcc;	
	}		
	setline(t);
	return 0;
}

int cmd_put(line_t l)		
{		
	long n=0;
	char *p=l.text;
	if (eval_expr(&p, &n)==NUMBER)
		put_special(l.var, n);
	else
		BasicErr=1;
	return 0;
}

int cmd_if(line_t l)		
{
	long n=0;
	char *p=l.text;

	if (eval_expr(&p, &n)==NUMBER)
	{
		if (*p++ != ',')
		{
			BasicErr=5; 
			return -1;
		}

		if (n == 0)
		{	
			//skip to next comma
			while (*p!=0 && *p!=',') p++;
			p++;
		}

		if (eval_expr(&p, &n)==NUMBER && n!=0)
		{
			int t = gotoln(n);
			if (t<0)
				BasicErr=5;
			else
				setline(t);
		}
	}
	return 0;
}

extern int speed;

int cmd_servo(line_t l)
{
	long n=0;
	char *p=l.text;
	long v=l.var;
	if (v>=0 && v<=31)
		v=getvar(v);
	else
		v=v-32;

	if (*p=='@') // set passive mode
	{
		// set pass servo id=line.var
		wckSetPassive(v);
	}
	else
	if (*p=='~') // set IO mode
	{
		p++;
		eval_expr(&p, &n);
		wckWriteIO(v, n) ;
	}
	else
	{
		eval_expr(&p, &n);
		if (n>00 && n<=254)
		{
			// set pos servo id=line.var, n
			// char SpeedLevel
			wckPosSend(v, speed, n);
		}
	}
	return 0;
}

int cmd_list(line_t l)
{
		//stack assign
		if (l.text[0]==':')
		{
			char *p=l.text+1;
			int i=strlen(p);
			while (*p != '\0' && i>0)
			{
				setvar(*p-'A',  eswap( getvar(*p-'A'),--i));
				p++;
			}
			return;
		}
		//stack un-assign
		if (l.text[0]=='#')
		{
			char *p;
			p=l.text+strlen(l.text+1);
			while (p > l.text )
			{
				setvar(*p-'A',epop());
				p--;
			}
			return;
		}
		//stack values
		if (l.text[0]=='^')
		{
			char *p=l.text+1;
			long n=0;
			while (1)
			{
				if (eval_expr(&p, &n)!=ARRAY)
				{
					epush(n);
				}
				if (*p!=',') break;
				p++;
			}
			return;
		}

		//new lists
		list_eval((l.var==32)?'!' : ('A' + l.var), l.text, 0);
		return 0;
}
int cmd_data(line_t l)
{ 
		list_eval((l.var==32)?'!' : ('A' + l.var), l.text, 1);
		return 0;
}	

int cmd_run(line_t l)
{
		long n=0;
		char *p=l.text;
		if (eval_expr(&p, &n) != NUMBER)
		{
			BasicErr=1;
			return 0;
		}
		if (n>=0 && n <=18)
		{
		/*	0x00:  //PunchLeft      0x01:  //PunchRight  0x02:  //SidewalkLeft
			0x03:  //SidewalkRight  0x04:  //TurnLeft    0x05:  //TurnRight
			0x06:  //GetupBack      0x07:  //GetupFront  0x08:  //WalkForward
			0x09:  //WalkBackward   0x0A:  //lshoot      0x0B:  //rshoot
			0x0C:  //rsidewalk      0x0D:  //lsidewalk   0x0E:  //standupr
			0x0F:  //standupf       0x10:  //sitdown     0x11:  //hi
			0x12:  //kick left front turn */
			rprintf("Play Motion %d\r\n",n);
			PlayMotion(n);
		}
		else
		{
			rprintfProgStr(PSTR("Valid range: 0-18\r\n"));
		}		
		return 0;
}

extern int gkp;

int cmd_wait(line_t l)
{

	int key;
	long n=l.value;
	char *p=l.text;
	
	if (*p!=0 )
	{
#ifdef IMAGE
extern int imready, pbrunning;

		if (!strncmp(p,"IMAG",4))
		{
			while (imready==0 && (key=uartGetByte())<0 ) 
				; // wait for signal
			imready=0;
			if (key>=0) gkp=key;
			return 0;
		}

		if (!strncmp(p,"PLAY",4))
		{
			while (pbrunning==0 && (key=uartGetByte())<0 ) 
				; // wait for signal
			pbrunning=0;
			if (key>=0) gkp=key;
			return 0;
		}
#endif

		if (!strncmp(p,"KEY",3))
		{
			gkp=-1;
			while (1) 
			{
				if ((key=irGetByte())>=0)  break;
				if ((key=uartGetByte())>=0) break;
			}
if (dbg) rprintf ("key=%d\n",key);

			gkp=key;
			return 0;
		}

		if (eval_expr(&p, &n) != NUMBER)
		{
			BasicErr=1;
			return 0;
		}
	}
	delay_ms((int)n);
	return 0;
}

int cmd_play(line_t l)
{	SendToSoundIC(l.value);
	return 0;
}

int cmd_mtype(line_t l)
{
	char *p=l.text;
	long n=0;
	if (eval_expr(&p, &n)!=NUMBER)
	{
		BasicErr=3; return -1;
	}
	if (n<0) PP_mtype=0;
	PP_mtype=n%4;
	return 0;
}

int cmd_speed(line_t l)
{
	char *p=l.text;
	long n=0;
	if (eval_expr(&p, &n)!=NUMBER)
	{
		BasicErr=3; return -1;
	}
	if (speed<0) n=0;
	speed=n%4;
	return 0;
}

int cmd_end(line_t l)
{
	rprintfProgStr (PSTR("End of program\r\n")); 
	return 0xCC;
}

void push_line(unsigned int n)
{
	if (gp<MAX_GOSUB_NEST-1)
		gosub[gp++]=n;
}

int cmd_gosub(line_t l)
{	
	int t;
	push_line(nxtline);
	t=gotoln(l.value);

	if (l.text != 0 && l.text[0]==',')
	{
		char *p=l.text+1;
		long n=0;
		while (1)
		{
			if (eval_expr(&p, &n)!=ARRAY)
			{
				epush(n);
			}
			if (*p!=',') break;
			p++;
		}
	}


	if (t<0)
		return 0xCC;	// this needs an error message		
	setline(t);
	//tmp=1;	
	return 0;
}
int cmd_return(line_t l)
{
	if (gp>0) {
		setline(gosub[--gp]);
		//tmp=1;
	} else {
		BasicErr=7;
	}
	return 0;
}

int cmd_out(line_t ln)
{
	long l=1;
	long n=0;
	char *p=ln.text;
	switch (eval_expr(&p, &n))
	{
	case NUMBER:
		if (*p==',')
		{
			p++;
			eval_expr(&p, &l);
		}
		while (l>0) { rprintfChar(n); l--;}
		break;
	case ARRAY:
		n=0;
		if (*p==':')
		{
			p++;
			if (eval_expr(&p, &n) != NUMBER)
			{
				BasicErr=3; break;
			}
		}
		for (l=0; l<listsize(arrayname); l++)
		{
			if (l>0 && n>0 && l%n==0) rprintfCRLF();
			rprintfChar(listread(arrayname,l));
		}
		break;
	}
	return 0;
}

int cmd_get(line_t ln)
{
	int ch=0,c=0;
	while (ch!=10 && ch!=13)
	{
		while ((ch = uartGetByte())<0) ; // wait for input
		scene[c++]=ch;
		rprintfChar(ch);
	}
	nis=c-1;
	return 0;
}

extern volatile BYTE   MIC_SAMPLING;

int cmd_light(line_t ln)
{
	long n=0;
	char *p=ln.text;
	if (eval_expr(&p, &n)!=NUMBER)
	{
		BasicErr=3; return -1;
	}
	if (*p==',')
	{
		//LIGHTS A,@{5,5,10,15,20,25}
		long tn;
		p++;
		if (eval_expr(&p, &tn)!=ARRAY)
		{
			BasicErr=3; return 0;
		}

		listreadc(arrayname);

		MIC_SAMPLING=0;
		if (nis==5)
			blights(n, scene);
		else
		{
			BasicErr=3; // must be 5 elements
			return 0;
		}
	}
	else
	{
		MIC_SAMPLING=0;
		lights(n);
	}
	return 0;
}

extern int fmflg;

int cmd_move(line_t ln)
{
	char *p=ln.text;
	// MOVE @A,C,D
	// Move to position @A, in C steps, taking D ms
	// MOVE @A
	// No args - send servo positions synchronously
	// with args (No Frames / Time in Ms) - use MotionBuffer
	// MOVE A,B,C,D
	// Move to position @![a,a+b],C,D

	// MOVE REC n
	// MOVE PLAY n
			
	if (p!=0 && *p != 0)
	{	
		int j, st=0;
		long tm=0,fm=0, nb=0;
		BYTE pos[32];

		if (!strncmp(p,"REC ",4))
		{
			p+=4;
			if (eval_expr(&p, &nb) != NUMBER)
			{
				BasicErr=1;
				return 0;
			}
			record(nb);
			return 0;
		}

		if (!strncmp(p,"PLAY ",5))
		{
			p+=4;
			if (eval_expr(&p, &nb) != NUMBER)
			{
				BasicErr=1;
				return 0;
			}
			playback(nb);
			return 0;
		}

		switch (eval_expr(&p, &fm))
		{
		case ARRAY:
			listreadc(arrayname);
			if (*p=='\0')
			{
				if (nis>0)
					wckSyncPosSend(nis-1, speed, scene, 0);
				return 0; //tmp;
			}
			break;
		case NUMBER:
			st=fm;
			if (*p==',')
			{
				p++;
				if (eval_expr(&p, &nb) != NUMBER)
				{
					BasicErr=1;
					return 0;
				}
			}
			if (st>=nis || st+nb>nis)
			{
				BasicErr=5;
				return 0;
			}
			break;
		}

		if (*p++ != ',') { BasicErr=1; return 0;}
		eval_expr(&p, &fm);
		if (*p++ != ',') { BasicErr=1; return 0;}
		eval_expr(&p, &tm);

		if (nb==0) nb=nis-st;

		for (j=0; j<nb; j++)
		{
			if (scene[j+st]>=0 && scene[j+st]<=254)
				pos[j] = scene[j+st];
			else
			{
				BasicErr=5;
				return 0;
			}

			if (j<16)
			{
				pos[j] += offset[j];
			}

			if (pos[j]<0)
				pos[j] = 0;

			if (pos[j]>254)
				pos[j] = 254;

			if (dbg) rprintf("DBG:: MOVE %d=%d\n", j, pos[j]);
		}

		if (!dbg) PlayPose(tm, fm, speed, pos, (fmflg==0)?nb:0);
		fmflg=1;
	}
	//
	return 0;
}

int cmd_offset(line_t ln)
{
	long n=0;
	char *p=ln.text;
	// OFFSET @A	load @A into offset
	// OFFSET #		load Basic18-Basic16 into offset
	// OFFSET		zero offset
	int i;
	if (p==0 || *p=='\0')
	{				
		for (i=0; i<16; i++)
		{
			offset[i]=0;
		}
	}
	else if (*p == '#')
	{
		for (i=0; i<16; i++)
		{
			offset[i]=(basic18[i]-basic16[i]);
		}
	}
	else if (eval_expr(&p, &n)==ARRAY)
	{
		if (arrayname=='!')
		{
			for (i=0; i<16 && i<nis; i++)
			{
				offset[i]=scene[i];
			}
		}else{
			//
			int sz=listsize(arrayname);
			for (i=0; i<16 && i<sz; i++)
			{
				offset[i]=listread(arrayname,i);
			}
		}
	}
	else
		BasicErr=1;
	return 0;
}

int cmd_delsel(line_t ln)
{
	// i.e. DELETE 5
	// or   DELETE *
	//      DELETE 5,7
	//      DELETE 5,*
	long n=0;
	char *p=ln.text;
	long n2;
	char an='!';

	if (*p=='@')
	{
		p++;
		an=*p;
		if (an != '!' && ( an<'A' || an>'Z')  || *(p+1) != ',')
		{
			BasicErr=3; return 0;
		}
		p++;				
		p++;
	}
	if (*p=='*')
	{
		p++;
		listdelete(an, 0,0,2); 
		return 0;
	}
	if (eval_expr(&p, &n)!=NUMBER)
	{
		BasicErr=3; return 0;
	}
	if (n<0 || n>=SCENESZ)
	{
		BasicErr=3; return 0;
	}

	n2=1;
	if (*p==',')
	{
		p++;
		if (*p=='*')
		{
			p++;
			n2=0;
		}
		else
		if (eval_expr(&p, &n2)!=NUMBER)
		{
			BasicErr=3; return 0;
		}
	}
	else
		n2=n;

	listdelete(an, n, n2, ln.token==DELETE);
	return 0;
}

int cmd_inset(line_t ln)
{
	// i.e. SET [@A,]I,V or SET @A[i]=V
	// current array ![I]=V
	char *p=ln.text;
	long n=0;	

	int ind=0;
	char an='!';

	if (*p=='@')
	{
		p++;
		an=*p;
		if (!(an == '!' || ( an>='A' && an<='Z')  && ( *(p+1) == ',' || *(p+1) == '[')))
		{
			BasicErr=3; 	return 0;
		}
		p+=2;
	}
	if (eval_expr(&p, &n)!=NUMBER)
	{
		BasicErr=3; 	
		return 0;
	}
	ind=n;
	if ( !(*p == ',' || *p==']')  || n<0 || n>=SCENESZ)
	{
		BasicErr=3; 	
		return 0;
	}
	if (*p==',')
	{
		p=p+1;
	}
	else if (*p==']' &&  *(p+1)=='=')
	{
		p=p+2;
	}
	if (eval_expr(&p, &n)!=NUMBER)
	{
		BasicErr=3; 	
		return 0;
	}

	listset(an, ind, n, ln.token==INSERT);
	return 0;
}

void swap(int *x,int *y)
{
   int temp;
   temp = *x;
   *x = *y;
   *y = temp;
}

void quicksort(int list[],int m,int n)
{
   int key,i,j,k;
   if( m < n)
   {
      k = (m+n)/2;
      swap(&list[m],&list[k]);
      key = list[m];
      i = m+1;
      j = n;
      while(i <= j)
      {
         while((i <= n) && (list[i] <= key))
                i++;
         while((j >= m) && (list[j] > key))
                j--;
         if( i < j)
                swap(&list[i],&list[j]);
      }
      // swap two elements
      swap(&list[m],&list[j]);
      // recursively sort the lesser list
      quicksort(list,m,j-1);
      quicksort(list,j+1,n);
   }
}

int cmd_sort(line_t ln)
{
	char *p=ln.text;
	long n=0;
	if (ln.var==1)
	{
		//Length of gen, no of generation, no to surive
		// e.g.
		// PRINT @{12,1,1,1,2,2,2,3,3,3,1,5,2}
		// SORT #3,3,3
		long i, param[3],sho=0,lgn,nog,nts;
		for (i=0; i<3; i++)
		{
			eval_expr(&p, &param[i]);
			if (*p==',' && i<2)
			{
				p++;
			}
			else if (*p !=0)
			{
				BasicErr=3; break;
			}
		}

		sho=0; //1;
		lgn=param[0];
		nog=param[1];
		nts=param[2];

		if (sho)
		{
			rprintf("length    = %ld\n", lgn);
			rprintf("N of Gen  = %ld\n", nog);
			rprintf("N to Save = %ld\n", nts);
			if (nts<1 || nts>nog)
			{
				BasicErr=3; return 0;
			}
		}

		for (i=0; i<nts; i++)
		{
			int j,k,mx;
			int sof=lgn*nog + i;

			mx=scene[sof];
			//rprintf("loop %d (%d,%d)\n", i, sof, mx);

			for (j=sof+1; j<(lgn+1)*nog; j++)
			{
				if (scene[j] > mx )
				{
					//swap fit factors
					//rprintf("swap %d and %d\n", i, i+(j-sof));
					scene[sof]= scene[j];
					scene[j]=mx;
					mx=scene[sof];
					//rprintf("MAX=%d\n", mx);
					//swap genes
					for (k=0; k<lgn; k++)
					{
						int t=scene[k+i*lgn];
						scene[k+i*lgn]=scene[k+(i+j-sof)*lgn];
						scene[k+(i+j-sof)*lgn]=t;
					}
				}
			}
		}
		nis = nts*lgn;
	}
	else
	{
		if (eval_expr(&ln.text, &n) != ARRAY)
		{
			BasicErr=1;
			return 0;
		}
		quicksort(scene,0,nis-1);
	}
	return 0;
}

int cmd_ic2o(line_t ln)
{
	char *p=ln.text;
	long n=0;	
	BYTE ob[20];
	long i, addr=0;
	if (eval_expr(&p, &addr)==NUMBER)
	{
		if (*p++ != ',') { BasicErr=1; return 0;}
		if (eval_expr(&p, &n) != ARRAY)
		{
			BasicErr=1;
			return 0;
		}
		listreadc(arrayname);
		for (i=0; i<nis; i++)
			ob[i]=scene[i];
		I2C_write(addr, nis, ob);
	}
	return 0;
}

int cmd_ic2i(line_t ln)
{
	char *p=ln.text;
	long n=0;	
	int i; long ibc=0, addr=0;		
	if (eval_expr(&p, &addr)==NUMBER)
	{
		BYTE ob[20];
		BYTE ib[20];
			
		if  (*p++ != ',')
		{
			BasicErr=1;
			return 0;
		}
		if (eval_expr(&p, &ibc)==NUMBER)
		{
			if  (*p == ',')
			{
				if (eval_expr(&p, &n) != ARRAY)
				{
					BasicErr=1;
					return 0;
				}
				listreadc(arrayname);
			}
			else
				nis=0;
		}	
		for (i=0; i<nis; i++)
			ob[i]=scene[i];

		I2C_read (addr, nis, ob, ibc, ib);
			
		for (i=0; i<ibc; i++)
			scene[i]=ib[i];	
		nis=ibc;				
	}
	return 0;
}

int cmd_step(line_t ln)
{
	char *p=ln.text;
	long n=0;	
		//STEP servo=from,to[,inc][,dlay]
		{
			long sf, st, si=5, sp, sn, sw=75, cnt=0, sd=8;
			long v=ln.var;
			if (v>=0 && v<=31)
				v=getvar(v);
			else
				v=v-32;
			n=0;

			if (eval_expr(&p, &sf)!=NUMBER)
			{
				BasicErr=3; return 0;
			}
			if (*p++ != ',')
			{
				BasicErr=2; return 0;
			}
			if (eval_expr(&p, &st)!=NUMBER)
			{
				BasicErr=3; return 0;
			}
			if (*p==',')
			{
				p++;
				if (eval_expr(&p, &si)!=NUMBER)
				{
					BasicErr=3; return 0;
				}
				sd=si+1;
				if (*p==',')
				{
					p++;
					if (eval_expr(&p, &sd)!=NUMBER)
					{
						BasicErr=3; return 0;
					}
					if (*p==',')
					{
						p++;
						if (eval_expr(&p, &sw)!=NUMBER)
						{
							BasicErr=3; return 0;
						}
					}
				}
			}

			sp = wckPosRead(v); // get servo current position
			
			//rprintf("STEP %d %d %d %d %d\n", sf, st, sp, si, sd, sw);

			if (sf < st) 
			{
				if (sp < sf) 
				{
					wckPosSend(v, speed, sf);
					sp=sf;
					delay_ms(sw);
				}
				sn=sp;
				while (cnt++<25 && (sp-sn)<sd && sn<st)
				{
					sp += si;
					if (sp>0 && sp<=254)
					{
						wckPosSend(v, speed, sp);
					}
					//sleep
					delay_ms(sw);
					sn = wckPosRead(v);
					//rprintf("-> %d %d %d %d\n", cnt, sp, sn, sd);
				}
			}
			else
			{
				if (sp > sf) 
				{
					wckPosSend(v, speed, sf);
					sp=sf;
					delay_ms(sw);
				}
				sn=sp;
				while (cnt++<25 && (sn-sp)<sd && sn>st)
				{
					sp -= si;
					if (sp>0 && sp<=254)
					{
						wckPosSend(v, speed, sp);
					}
					//sleep
					delay_ms(sw);
					sn = wckPosRead(v);
					//rprintf("<- %d %d %d %d\n", cnt, sp, sn, sd);
				}
			}
		}
		return 0;
}


extern volatile BYTE   sData[];
extern int 	           sDcnt;
extern volatile BYTE   MIC_LEVEL;
extern volatile WORD   MIC_DLY  ;
extern volatile BYTE   MIC_STOP ;
extern volatile BYTE   MIC_RATE ;
extern volatile BYTE   MIC_NOS;

int cmd_sample(line_t ln)
{
	//SAMPLE n,d
	// trigger level =n, max delay= d, (4ms per sample)
	char *p=ln.text;
	long n=4, d=2000; // defaults
	if (eval_expr(&p, &n)==NUMBER)
	{
		if (*p==',')
		{
			int i;
			p++;
			eval_expr(&p, &d);
					
			MIC_LEVEL=n;
			MIC_RATE=4; //4ms
			MIC_NOS=64; //max size for FFT
			sDcnt=0;
					
			if (*p==',')
			{
				p++;
				eval_expr(&p, &n);
				MIC_RATE = n;
						
				if (*p==',')
				{
					p++;
					eval_expr(&p, &n);
					if (n>0 && n<=SCENESZ)
						MIC_NOS = n;
				}
			}

			for (i=0; i<MIC_NOS; i++) 
			{
				sData[i]=0;     // and clear
			}
			sample_sound(1);				

			MIC_STOP=1;  
			MIC_DLY=d/MIC_RATE; 
#ifdef AVR
			while (MIC_STOP==1)
			{
				// wait until sampling complete
			}
#else
			printf ("WIN: SAMPLE Level=%d,Dly=%d,Rate=%d,Nos=%d\n", MIC_LEVEL, MIC_DLY, MIC_RATE, MIC_NOS);
#endif
			if (MIC_DLY==0)
				nis=0;
			else
			{
				for (i=0; i<MIC_NOS; i++) 
				{
					scene[i]=sData[i];     // and clear
				}
				nis=MIC_NOS;
			}
		}
		else
			BasicErr=1;
	}
	return 0;
}

int timer=0;
int tline=0;
int kline=0;
int imline=0;

int cmd_on(line_t ln)
{
	char *p=ln.text;
	long a,b;
	int t;

#ifdef IMAGE
	if (strncmp(p,"IMAGE,",6)==0)
	{
		// ON IMAGE GOSUB y
		p+=6;
		if (eval_expr(&p, &a) != NUMBER)
		{
			BasicErr=1;
			return 0;
		}
		imline=(int)a;
  		rprintf("Set image handler %ld\n", a);
		return 0;
	}
#endif

	if (strncmp(p,"KEY,",4)==0)
	{
		// ON KEY GOSUB y
		p+=4;
		if (eval_expr(&p, &a) != NUMBER)
		{
			BasicErr=1;
			return 0;
		}
		kline=(int)a;
		return 0;
	}

	if (strncmp(p,"TIME",4)!=0)
	{
			BasicErr=3;return 0;
	}
	// ON TIME x GOSUB y
	p+=4;
	if (eval_expr(&p, &a) != NUMBER)
	{
		BasicErr=1;
		return 0;
	}
	if (*p++!=',')
	{
			BasicErr=3;return 0;
	}
	if (eval_expr(&p, &b) != NUMBER)
	{
		BasicErr=1;
		return 0;
	}

	t=(int)b;

	if (dbg) rprintf("Set timer %ld, %d\n", a,t);
	timer=(int)a;
	tline=t;
	gtick=0;

	return 0;
}

int cmd_scale(line_t ln)
{
	// scale @X,10     - scales array to max value 10
	// scale @X,5,1,0  - threshold  @X >=5 set 1 else 0
	// results in @!

	char *p=ln.text;
	long n=0,ct=0,cf=0;	
	int i,s;

	if (eval_expr(&p, &n) != ARRAY)
	{
		BasicErr=1;
		return 0;
	}

	listreadc(arrayname);

	if (*p==',')
	{
		p++;
		eval_expr(&p, &n);
	}

	if (*p==',' || *p=='#')  //threshold rather than scale
	{
		char f=*p;
		p++;
		eval_expr(&p, &ct);
		if (*p==',')
		{
			p++;
			eval_expr(&p, &cf);
		}

		for (i=0; i<nis; i++)
		{
			if (f==',')
			{
				if (scene[i]>=n) scene[i]=ct; else scene[i]=cf;
			}
			else
			{
				if (abs(scene[i])>=n) scene[i]=ct; else scene[i]=cf;
			}
		}
		return 0;
	}


	if (n>1)  //scale the array
	{
		s=scene[0];
		for (i=0; i<nis; i++)
		{
			if (scene[i]>s) s=scene[i];
		}

		if (s==0) return 0;

		s=n/s;

		for (i=0; i<nis; i++)
		{
			scene[i]=scene[i]*s; // scale
		}
	}
	return 0;
}

int cmd_fft(line_t ln)
{
	char *p=ln.text;
	long n=0;	
	int m,i,s;
	if (eval_expr(&p, &n) != ARRAY)
	{
		BasicErr=1;
		return 0;
	}

	listreadc(arrayname);

	n=0;
	if (*p==',')
	{
		p++;
		eval_expr(&p, &n);
	}
	if (nis==8)
		m=3; // 16 elements
	else if (nis==16)
		m=4;
	else if (nis==32)
		m=5;
	else if (nis==64) // max must be <1/2 SDATASZ (for imag)
		m=6;
	else
	{
		rprintfProgStr (PSTR("Invalid sample size must be one of 8,16,32 or 64\n"));
		BasicErr=1;
		return 0;
	}
	if (n>1)  //scale the array
	{
		s=scene[0];
		for (i=0; i<nis; i++)
		{
			if (scene[i]>s) s=scene[i];
			scene[i+nis]=0; // zero imag
		}
		s=n/s;
		for (i=0; i<nis; i++)
		{
			scene[i]=scene[i]*s; // scale
		}
	}

	if (dbg) 
	{
		rprintf ("%d", nis);
		for (i=0; i<nis; i++)
		{
			rprintf (",%d", scene[i]);
		}
		rprintfCRLF();
				
		rprintf ("FFT (%d) = %d\r\n", m, fix_fft(scene, &scene[nis], m, 0));
				
		for (i=0; i<nis; i++)
		{
			rprintf ("%d) = (%d %d) %d\r\n", i, 
				(int)scene[i],  
				(int)scene[nis+i], 
				(int)Sqrt((scene[i]*scene[i]) + (scene[nis+i]*scene[nis+i])));
		}

	}
	else
	{
		fix_fft(scene, &scene[nis], m, 0);
	}

	for (i=0; i<nis; i++)
	{
		scene[i]=(int)Sqrt((scene[i]*scene[i]) + (scene[nis+i]*scene[nis+i])); //power
	}
	return 0;
}


int cmd_gen(line_t line)
{
	// GEN [No Gn], length, Mute rate%, Mute rnge, Val[min/max], type
	// GEN 4,16,5,2,0,254,0	
	char *p=line.text;	
	int i, param[7],ty,sho,nog,ln,mr,mm;
	long t;
	for (i=0; i<7; i++)
	{
		eval_expr(&p, &t);
		param[i]=(int)t;
		if (*p==',' && i<6)
		{
			p++;
		}
		else if (*p !=0)
		{
			BasicErr=3; break;
		}
	}

	ty=param[6]&3; // 0-1 2
	sho=param[6]&8; // show flag
	nog=param[0];
	ln=param[1];
	mr=param[2];
	mm=param[3];

	if (mr>0 && mm==0)
	{
		rprintfProgStr(PSTR("Error - mutation range must be >0\n"));
		BasicErr=4; 
		return 0;
	}

	if (sho)
	{
		rprintf("Type      = %d\n", ty);
		rprintf("length    = %d\n", ln);
		rprintf("Generate  = %d\n", nog);
		rprintf("Mut rate  = %d\n", mr);
		rprintf("Mut range = +/-%d\n", mm);
	}

	for (i=0; i<nog; i++)
	{
		int v=0,e;
		int co1=0;
		int co2=0;

		if (ty>0)
		{
			co1 = rand()%(ln+1);
			co2 = rand()%(ln+1);
			if (co2<co1) swap(&co1,&co2);
			if (sho)
			{
				if (ty>0) rprintf("CO pt1    = %d\n", co1);
				if (ty>1) rprintf("CO pt2    = %d\n", co2);
			}
		}

		for (e=0; e<ln; e++)
		{
			if (ty==0)
			{
				v=scene[e];    // gene from parent 1
			}
			if (ty==1)
			{
				// single cross over
				if (e<co1)
					v=scene[e]; //straight copy p1
				else
					v=scene[e+ln]; //straight copy p2
			}
			if (ty==2)
			{
				// two cross over
				if (e<co1 || e>=co2)
					v=scene[e]; //straight copy p1
				else
					v=scene[e+ln]; //straight copy p2
			}

			if (rand()%100<mr)
			{
				v = v + (rand()%(2*mm))-mm;
				if (v<param[4]) v=param[4];
				if (v>param[5]) v=param[5];
			}

			if (ty==1 || ty==2)
				scene[2*ln+(ln*i)+e]=v; // copy
			else
				scene[ln+(ln*i)+e]=v; // copy
		}
	}

	nis = nog*ln;
	for (i=0; i<nis; i++)
	{
		if (ty==1 || ty==2)
			scene[i]=scene[i+2*ln];
		else
			scene[i]=scene[i+ln];
	}
	return 0;
}

extern BYTE EEMEM FIRMWARE[];  // used by Robobuilder OS
extern BYTE EEMEM PERSIST [];  // persistent data store

int cmd_poke(line_t ln)
{
    int f=1;
 	int i;
	long n=0;
	char *p=ln.text;
	if (*p=='$')
	{
		p++;
		f=0;
	}
 	if (*p=='@')
	{
		p++;
		for (i=0; i<nis; i++)
			eeprom_write_byte(PERSIST+i, scene[i]);
	}
 	else if (eval_expr(&p, &n)==NUMBER)
	{
		// put result into address line.var
		BYTE addr=ln.var;
        if (f)
			eeprom_write_byte(FIRMWARE+addr, n);
        else
			eeprom_write_byte(PERSIST+addr, n);                             
	}
	else
		BasicErr=1;
	return 0;
}


int cmd_extend(line_t ln)
{
#ifdef AVR
		rprintfProgStr(PSTR("? Cmd not available\r\n"));
#else
		extend(ln.text);
#endif
	return 0;
}

int cmd_ikin(line_t line)
{
	//inverse kienetics - TBD
	return 0;
}

//matrix.c matrix fuctions 
extern int matzero	(char ln1, int a, int b, int c, int d) ;
extern int matzerodiag	(char ln1) ;
extern int transpose	(char ln1, char ln2) ;  		// @A^T"
extern int multiply	(char ln1, char ln2, char lnx);   	// @X = @A * @B"//
extern int convolve	(char ln1, char ln2);   		// @A # @B"
extern int histogram	(char ln1, int mode);   		// @A 
extern int matgetw	(char m);
extern int matgeth	(char m);
extern int matcreate	(char m, int w, int h);
extern int matprint	(char m);

int cmd_matrix(line_t ln)
{
	char *p=ln.text;
	int i,w;
	char m, ma, mb, mx;
	long n;

	if (!strncmp(p,"DEF ",4))
	{
		// !MAT DEF A;1;2
		p+=4;
		if (*p != '\0')
		{			
			m=*p++;
			if (*p == ';' || *p == '=')
			{
				p++;
				eval_expr(&p, &n);
				w=n;

				if (*p++ == ';')
				{
					eval_expr(&p, &n);
					matcreate(m,w,n);
					return 0;
				}
			}

		}
	}

	if (!strncmp(p,"PRINT ",6))
	{
		p+=6;		
		while (*p != '\0')
		{
			m=*p++;
			matprint(m);
			if (!(*p == ';' || *p == ','))
			 return 0;
			p++;
		}
	}

	if (!strncmp(p,"HIST ",5))
	{
		// !MAT HIST 1;A			
		p+=5;
		if (*p != '\0')
		{
			eval_expr(&p, &n);
			if (*p++ == ';')
			{				
				ma=p[0];
				histogram(ma, n);
				return 0;
			}
		}
	}

	if (!strncmp(p,"LET ",4) || (p[0]>='A' && p[0]<='Z' && p[1]=='='))
	{
		if (p[1]=='E')
			p+=4;

		if (p[1]=='=')
		{
			mx=p[0];
			p=p+2;
		}
		else
			return 1;

		if (*p =='(')
		{
			//LET A=(expr)
			char *t=p;
			long el;	
			int tn = nis;
			int ne = listsize(mx);
			for (i=0; i<ne; i++)
			{
				p=t;
				n=0;
				nis=listread(mx,i);
				eval_expr(&p,&el);
				listwrite(mx, i, (int)el);
			}
			nis=tn;
			return 0;
		}

		ma=*p++;
		if (*p =='\0')
		{
			//LET A=B
			matcreate(mx, matgetw(ma), matgeth(ma));
			listcopy(mx, ma);
		}
		if (*p=='^')
		{
			//LET A=B^
			transpose(ma,mx);
			p++;
		}
		if (*p=='#')
		{
			//LET A=B#C
			p++;
			mb=*p++;
			convolve(ma, mb) ;
			listcopy(mx, '!');
		}

		if (ma =='Z' && *p=='E' && *(p+1)=='R')
		{
			//LET A=ZER
			p+=3;
			matzerodiag(mx);
		}

		if (*p=='*')
		{
			//LET A=B*C
			p++;
			mb=*p++;
			multiply(ma,mb,mx);
		}
		if (*p=='+' || *p=='-' || *p=='.' || *p=='/')
		{
			//LET A=B+C
			char op=*p++;
			if (op=='.') op='*';
			mb=*p++;
			add_sub(ma,mb,op);
			//copy into mx @!
			matcreate(mx, matgetw(ma), matgeth(ma));
			listcopy(mx, '!');
		}
		return 0;
	}


	if (!strncmp(p,"ZERO ",5))
	{
		// !MAT ZERO A;1;1;2;2
		// !MAT ZERO A	
		int a[4];
		p+=5;

		if (*p != '\0')
		{			
			ma=*p++;

			if (*p != ';')
			{
				//MAT ZERO A -> set diaganol to zero
				matzerodiag(ma);
				return 0;
			}

			for (i=0; i<4; i++)
			{
				if (i<3 && (*p != ';'|| *p != ','))
					return 1;
				p++;
				eval_expr(&p, &n);
				a[i]=n;
			}
			matzero(ma,a[0],a[1],a[2],a[3]);
			return 0;
		}
	}
	BasicErr=1;
	return 1;
}

/**************************************************************************/

int pbrunning=0;
int pbstep=0;
int pbtime=0;

void record(int mode)
{
	//scene[0]=time
	//scene[1]=nof servos
	//scene[2]..scene[2+nof-1] = servoids

	int i,n,p;

	if (mode>=16)
	{
		n=mode;
		scene[0]=500;
		scene[1]=n;	
		for (i=0; i<n; i++)
		{
			scene[2+i]=i;
		}			
	}
	else
	{
		n=scene[1];
		if (nis<n+2) { rprintf ("Err - setup array [%d,%d]\n",nis,n); return;}
	}

	for (i=2; i<2+n; i++)
	{
		if (scene[i]<0 || scene[i]>30)  { rprintf ("Err - servo id %d\n", scene[i]); return;}
		// set passive
		if (dbg>1) rprintf ("Passive %d\n",scene[i]); 
		wckSetPassive(scene[i]);
	}

	p=2+n;

	rprintfProgStr (PSTR("Press any key to record or 'esc' to finish\n"));

	while (1)
	{
		//wait for key press
		int key;

		while ((key=uartGetByte())<0) ;

		if (key==27)
		{
			rprintfProgStr (PSTR("Done\n"));
			nis=p;
			return;
		}

		rprintf("%d: ", 1+(p-n-2)/n);

		for (i=2; i<2+n; i++)
		{
			if (scene[i]<0 || scene[i]>30)  { rprintf ("Err -servo id %d\n", scene[i]); return;}
			// record
			scene[p] = wckPosRead(scene[i]); //read values
			rprintf("%d ", scene[p]);
			p++;
		}
		rprintfCRLF();
	}
}

void playback(int mode)
{
	int i;
	int n=scene[1];

	if (nis<n+2) { rprintf ("Err - setup array [%d,%d]\n",nis,n); return;}

	if (mode==3)
	{
		pbrunning=0; // pause
		return;
	}

	if (mode==4)
	{
		pbrunning=1; // re-start
		return;
	}

	if (mode==5)
	{
		pbrunning=1; // reset
		pbstep=0;	
		return;
	}

	if (mode>=10)
	{
		pbrunning=1;           // run from
		pbstep=2+(mode-9)*n;	 // step 
	}
	else
	{
		//mode 1 & 2 - set up
		pbstep=2+n;
		pbrunning=1; // start running
	}
	pbtime=scene[0];

	if (mode==2)  
		return;  // asynch mode

	while (1)
	{
		for (i=2; i<2+n; i++)
		{
			if (scene[i]<0 || scene[i]>30)  { rprintf ("Err - servo id %d\n", scene[i]); return;}
			// wckMove(scene[i],scene[p++])
			if (dbg>1) rprintf ("Move %d = %d\n",scene[i],scene[pbstep]); 
			wckPosSend(scene[i], 4, scene[pbstep++]);
		}

		//wait for time t;
		if (dbg>1) 
			rprintf ("Wait %d\n",scene[0]); 
		delay_ms(pbtime);

		if (mode >= 10 || pbstep >=nis)
		{
			rprintfProgStr (PSTR("Done\r\n"));
			pbrunning=0;
			return;
		}	
	}
}


void pb2()
{
	int i=0;
	int n=scene[1];

	for (i=2; i<2+n; i++)
	{
		if (scene[i]<0 || scene[i]>30)  { rprintf ("invalid servo id %d\n", scene[i]); return;}
		// wckMove(scene[i],scene[p++])
		if (dbg>1) rprintf ("Move %d = %d\n",scene[i],scene[pbstep]); 
		wckPosSend(scene[i], 4, scene[pbstep++]);
	}

	if (pbstep >=nis)
	{
		rprintfProgStr (PSTR("Done\r\n"));
		pbrunning=0;
		pbtime=0;
		return;
	}
	pbtime=gtick + scene[0];
}


int cmd_network(line_t ln);

int (*cmdtab[])(line_t) = {
	cmd_let,   //LET
	cmd_for,   //FOR
	cmd_if,    //IF    
	cmd_dummy, //THEN
	cmd_dummy, //ELSE
	cmd_goto,  //GOTO  
	cmd_print, //PRINT 
	cmd_get,   //GET
	cmd_put,   //PUT
	cmd_end,   //END
	cmd_list,  //LIST   
	cmd_run,   //XACT
	cmd_wait,  //WAIT  
	cmd_next,  //NEXT
	cmd_servo, //SERVO 
	cmd_move,  //MOVE
	cmd_gosub, //GOSUB 
	cmd_return,//RETURN
	cmd_poke,  //POKE 
	cmd_stand, //STAND
	cmd_play,  //PLAY
	cmd_out,   //OUT
	cmd_offset,//OFFSET
	cmd_run,   //RUN
	cmd_ic2o,  //I2CO
	cmd_ic2i,  //I2CI 
	cmd_step,  //STEP
	cmd_speed, //SPEED
	cmd_mtype, //MTYPE
	cmd_light, //LIGHTS
	cmd_sort,  //SORT
	cmd_fft,   //FFT
	cmd_sample,//SAMPLE
	cmd_scale, //SCALE
	cmd_data,  //DATA
	cmd_inset, //SET
	cmd_inset, //INSERT
	cmd_delsel,//DELETE
	cmd_gen,   //GEN
	cmd_network,//NETWOR
	cmd_delsel,//SELECT
	cmd_extend,//!(EXPAND)
	cmd_on,    //ON
	cmd_matrix //!(EXPAND)
};

unsigned char execute(line_t line, int dbf)
{
	//   Execute action
	unsigned char t= (unsigned char)(*cmdtab[line.token])(line);
	return (t==0)?1:t;
}
