#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define	DBO(x) {if (&dbg) {x}}
extern int simflg, dbg;

int sockfd;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int socket_init(char *hostname, int portno)
{

    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        printf("ERROR creating socket");
        return 1;
    }
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        return 1;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
        printf("ERROR connecting");
        return 1;
    }
    return 0;
}

void initsocket()
{
	int v;
	
	if (socket_init("127.0.0.1", 8888))
	{
	    printf("Socket startup failed\n");
		simflg=0;
		return;
	}
	
	v= testsocket("V$");
	if (simflg==1) {printf ("Sim mode (%d)\n", v);}
}

int testsocket(char *echoString)
{
    char echoBuffer[256];
    
   	if (simflg==0) return 0;
    
    int n = write(sockfd,echoString,strlen(echoString));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(echoBuffer,256);
    n = read(sockfd,echoBuffer,255);
    if (n < 0) 
         error("ERROR reading from socket");

    printf("%s\n",echoBuffer);

	if (echoBuffer[0]=='D' && echoBuffer[1]==':' )
	{
		int n=0;
		sscanf(echoBuffer,"D:%d$", &n);
		return n;
	}
    //close(sockfd);
    return 0;
}

/* wck commands */
void wckPosSend(unsigned char ServoID, char Torque, unsigned char Position)
{
	char buff[64];
	sprintf(buff, "S:%d:%d$", ServoID, Position);
	testsocket(buff);

	DBO(printf ("SOCK: Servo Send %d [%d] -> %d\n", ServoID, Torque, Position);)
}
int  wckPosRead(char ServoID)
{
	int r;
	char buff[64];
	sprintf(buff, "R:%d$", ServoID);
	r = testsocket(buff);

	DBO(printf ("SOCK: Servo Read %d\n", ServoID); )
	return r;
}
void wckSetPassive(char ServoID)
{
	char buff[64];
	sprintf(buff, "P:%d$", ServoID);
	testsocket(buff);

	DBO(printf ("SOCK: Servo Passive %d\n", ServoID); )
}

void wckSyncPosSend(char LastID, char SpeedLevel, char *TargetArray, char Index)
{
	int i=0;
	DBO(printf ("SOCK: Servo Synch Send  %d [%d]\n", LastID, SpeedLevel);)

	// convert this to "Y:%d...:%d"
	//tbd
	for (i=Index; i<=LastID; i++)
	{
			wckPosSend(i,SpeedLevel,TargetArray[i]);
	}
}
