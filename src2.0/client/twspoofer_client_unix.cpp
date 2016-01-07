#include <iostream>      //cout
#include <stdio.h>       //printf
#include <stdlib.h>
#include <string.h>      //strlen
#include <string>        //string
#include <unistd.h>
#include <sys/socket.h>  //socket
#include <arpa/inet.h>   //inet_addr
#include <netdb.h>       //hostent
#include <pthread.h>     // threading,  "-lpthread" nicht vergessen!!
#include <signal.h>      // ctrl-c

using namespace std;

#define SERVER_IP "62.141.46.191"
#define SERVER_PORT 2016

int g_Socket;

struct ThreadParameters
{
	int clientid;
	int socket;
};

void sigproc(int)
{
	printf("\nGot CTRL-C, disconnecting from server\n");
	send(g_Socket, "exit", sizeof("exit"), 0);
	printf("Shutting down...\n");
	close(g_Socket);
	exit(0);
}


void *WorkingThread(void *pParam)
{
	ThreadParameters tp = *(ThreadParameters*)pParam;

	int clientid = tp.clientid;
	int s = tp.socket;
	int t = 0;
	char aBuf[32];

	while (1) //every 1 sec
	{ 
		if (t > 30)
		{
			t = 0;
			snprintf(aBuf, sizeof(aBuf), "keepalive %d", clientid);
			send(s, aBuf, sizeof(aBuf), 0);
		}
		t++;
		sleep(1000);
	}
}

int main(int argc, char* argv[])
{
	signal(SIGINT, sigproc);
	pthread_t Thread;

	char sBuffer[256], rBuffer[256];
	struct sockaddr_in info;

	// Socket
	g_Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (g_Socket == -1)
		perror("Error while creating socket");

	// Info
	info.sin_addr.s_addr = inet_addr(SERVER_IP);
	info.sin_family = AF_INET;
	info.sin_port = htons(SERVER_PORT);

	// Connect
	if (connect(g_Socket, (struct sockaddr*)&info, sizeof(info)) < 0)
		perror("Error in connect()");

	cout << "[Client]: Connected to server..." << endl;

	//client ID (keep alive)
	memset(&rBuffer, 0, sizeof(rBuffer));
	if (recv(g_Socket, rBuffer, sizeof(rBuffer), 0) > 0)
	{
		cout << "ID: " << rBuffer << endl;
		ThreadParameters *tp = new ThreadParameters;
		tp->clientid = atoi(rBuffer);
		tp->socket = g_Socket;
		pthread_create(&Thread, NULL, WorkingThread, tp);
		
		while (1)
		{
			memset(&sBuffer, 0, sizeof(sBuffer));
			memset(&rBuffer, 0, sizeof(rBuffer));

			if (recv(g_Socket, rBuffer, sizeof(rBuffer), 0) > 0)
			{
				cout << rBuffer << endl;
			}
			else
				perror("Error in recv()");

		cmd:
			cout << ">> ";
			cin.getline(sBuffer, sizeof(sBuffer));

			if (!sBuffer[0])
				goto cmd;

			if (send(g_Socket, sBuffer, strlen(sBuffer), 0) < 0)
				perror("Error in send()");
		}
	}
	else
		perror("Terminating client, no id, Error in recv()");

	close(g_Socket);

	while (1) { sleep(10000); }

	return 0;
}

