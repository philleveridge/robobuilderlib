#include <stdio.h>
#include <winsock.h>         // For socket(), connect(), send(), and recv()

typedef int socklen_t;
typedef char raw_type;       // Type used for raw data on this platform

#include <errno.h>             // For errno

const int RCVBUFSIZE = 32;    // Size of receive buffer

void initsocket()
{
	WSADATA wsaData;
	//init winsock
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
	{
        printf("WSAStartup() failed");
		return;
	}

}

int testsocket(char *echoString)
{
    char echoBuffer[33];    // Buffer for echo string + \0
	SOCKET sockDesc;

	struct sockaddr_in stSockAddr;

	int echoStringLen = strlen(echoString);   // Determine input length
    int bytesReceived = 0;              // Bytes read on each recv()
    int totalBytesReceived = 0;         // Total bytes read

	// Establish connection with the echo server

	if ((sockDesc = socket(PF_INET, SOCK_STREAM, IPPROTO_IP)) < 0)
	{
		printf ("Socket create failed");
		return 0;
	}

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(8888);
	stSockAddr.sin_addr.s_addr = inet_addr( "127.0.0.1" );

	if (connect(sockDesc, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))<0)
	{
		printf ("Connect failed");
		return 0;
	}

    // Send the string to the echo server
    send(sockDesc, echoString, echoStringLen, 0);

    while (totalBytesReceived < 33) 
	{
		// Receive up to the buffer size bytes from the sender
		char b[1];
		if ( recv(sockDesc, b, 1, 0) < 0) 
		{
			return 0;
		}
		// Keep tally of total bytes
		echoBuffer[bytesReceived++] = b[0];        // Terminate the string!
		if (b[0]=='$')
		{
			echoBuffer[bytesReceived++] = '\0';   
			break;
		}
    }
	
	//printf ("Rec: %s\n", echoBuffer);
	if (echoBuffer[0]=='D' && echoBuffer[1]==':' )
	{
		int n=0;
		sscanf(echoBuffer,"D:%d$", &n);
		return n;
	}

  return 0;

}
