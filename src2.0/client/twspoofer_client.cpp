#include "stdafx.h"

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <Windows.h>

#include <cstdio>
#include <iostream>
using namespace std;

#define SERVER_IP "62.141.46.191"
#define SERVER_PORT 2016

struct ThreadParameters
{
	int clientid;
	SOCKET socket;
};

DWORD WINAPI WorkingThread(LPVOID lpParam)
{
	ThreadParameters tp = *(ThreadParameters*)lpParam;

	int clientid = tp.clientid;
	SOCKET s = tp.socket;
	char aBuf[5];

	while (1) //every 15 sec
	{
		sprintf_s(aBuf, sizeof(aBuf), "\x16 %d", clientid);
		send(s, aBuf, sizeof(aBuf), 0);
		Sleep(15000);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD Thread;

	char sBuffer[256], rBuffer[256];
	SOCKET g_Socket;
	SOCKADDR_IN info;

	// WSA
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 0), &data) != 0)
		cout << "Error in WSAStartup(): " << WSAGetLastError() << endl;

	// Socket
	g_Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (g_Socket == INVALID_SOCKET)
		cout << "Error in socket(): " << WSAGetLastError() << endl;

	// Info
	info.sin_addr.s_addr = inet_addr(SERVER_IP);
	info.sin_family = AF_INET;
	info.sin_port = htons(SERVER_PORT);

	// Connect
	if (connect(g_Socket, (struct sockaddr*)&info, sizeof(info)) == SOCKET_ERROR)
		cout << "Error in connect(): " << WSAGetLastError() << endl;

	cout << "[Client]: Connected to server..." << endl;

	//client ID (keep alive)
	memset(&rBuffer, 0, sizeof(rBuffer));
	if (recv(g_Socket, rBuffer, sizeof(rBuffer), 0) != SOCKET_ERROR)
	{
		cout << "ID: " << rBuffer << endl;
		ThreadParameters tp;
		tp.clientid = atoi(rBuffer);
		tp.socket = g_Socket;
		CreateThread(NULL, 0, WorkingThread, &tp, 0, &Thread);
	
		while (1)
		{
			memset(&sBuffer, 0, sizeof(sBuffer));
			memset(&rBuffer, 0, sizeof(rBuffer));

			if (recv(g_Socket, rBuffer, sizeof(rBuffer), 0) != SOCKET_ERROR)
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
				cout << "Error in recv(): " << WSAGetLastError() << endl;

		cmd:
			cout << ">> ";
			cin.getline(sBuffer, sizeof(sBuffer));

			if (!sBuffer[0])
				goto cmd;

			if (send(g_Socket, sBuffer, strlen(sBuffer), 0) == SOCKET_ERROR)
				cout << "Error in send(): " << WSAGetLastError() << endl;
		}
	}
	else
		cout << "Terminating client, no id, Error in recv(): " << WSAGetLastError() << endl;

	closesocket(g_Socket);
	WSACleanup();

	while (1);

	return 0;
}

