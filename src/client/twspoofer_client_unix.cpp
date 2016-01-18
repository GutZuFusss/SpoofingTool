#include <iostream>      //cout
#include <stdio.h>       // printf, perror
#include <stdlib.h>      // atoi, exit
#include <string.h>      // strlen
#include <unistd.h>      // sleep, close
#include <arpa/inet.h>   // inet_addr
#include <pthread.h>     // thread_create, compile with "-lpthread"!
#include <signal.h>      // for ctrl-c

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
	char aBuf[5];

	while (1) //every 15 sec
	{
		sprintf_s(aBuf, sizeof(aBuf), "\x16 %d", clientid);
		send(s, aBuf, sizeof(aBuf), 0);
		Sleep(15000);
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
	{
		perror("Error while creating socket");
		return 1;
	}

	// Info
	info.sin_addr.s_addr = inet_addr(SERVER_IP);
	info.sin_family = AF_INET;
	info.sin_port = htons(SERVER_PORT);

	// Connect
	if (connect(g_Socket, (struct sockaddr*)&info, sizeof(info)) < 0)
	{
		perror("Error while connecting");
		return 1;
	}

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
				if(rBuffer[0] == '\x04')
				{
					cout << "End of transmission: ";
					if(rBuffer[1] == '\x06')
					{
						cout << "Disconneted from server." << endl; // maybe leave these message to the server?
						break;
					}
					else if(rBuffer[1] == '\x15')
					{
						cout << "Ack timeout." << endl;
						break;
					}
					else cout << "No reason given." << endl;
				}
				else
					cout << rBuffer << endl;
			}
			else
				perror("Error while receiving");

		cmd:
			cout << ">> ";
			cin.getline(sBuffer, sizeof(sBuffer));

			if (!sBuffer[0])
				goto cmd;

			if (send(g_Socket, sBuffer, strlen(sBuffer), 0) < 0)
				perror("Error while sending");
		}
	}
	else
		perror("Terminating client, no id, Error in recv()");

	close(g_Socket);

	while (1) { sleep(10000); }

	return 0;
}

