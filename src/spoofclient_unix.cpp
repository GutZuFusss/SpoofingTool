#include <iostream>    //cout
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <string>  //string
#include <unistd.h>
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <netdb.h> //hostent
using namespace std;

#define SERVER_IP "78.31.64.155"
#define SERVER_PORT 2015

int main()
{
	char sBuffer[256], rBuffer[256];
	int g_Socket;
	struct sockaddr_in info;

	// Socket
	g_Socket = socket(AF_INET , SOCK_STREAM , 0);
    if (g_Socket == -1)
        perror("Error in socket()");

    // Info
	info.sin_addr.s_addr = inet_addr(SERVER_IP);
    info.sin_family = AF_INET;
    info.sin_port = htons(SERVER_PORT);

    //Connect
    if (connect(g_Socket, (struct sockaddr*)&info, sizeof(info)) < 0)
        perror("Error in connect()");

    cout << "Connected" << endl;

    while(1)
    {
    	memset(&sBuffer, 0, sizeof(sBuffer));
		memset(&rBuffer, 0, sizeof(rBuffer));

		if(recv(g_Socket, rBuffer, sizeof(rBuffer), 0) < 0)
			perror("Error in recv()");
		else
			cout << rBuffer << endl;

cmd:
		cout << "Send cmd: ";
		cin.getline(sBuffer, sizeof(sBuffer));

		if(!sBuffer[0])
			goto cmd;

		if(send(g_Socket, sBuffer, strlen(sBuffer), 0) < 0)
			perror("Error in send()");
		else
			cout << sBuffer << " sent." << endl;
    }

    close(g_Socket);

	return 0;
}