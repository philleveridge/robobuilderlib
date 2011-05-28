#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

extern int simflg;

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
    char buffer[256];
    
   	if (simflg==0) return;
    
    int n = write(sockfd,echoString,strlen(echoString));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}