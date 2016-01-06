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

#define MAX_PACKET 4096

typedef struct
{
	unsigned char IHL : 4;
	unsigned char Version : 4;
	unsigned char ECN : 2;
	unsigned char DSCP : 6;
	unsigned short Length;
	unsigned short ID;
	unsigned short FragOffset : 13;
	unsigned short Flags : 3;
	unsigned char TTL;
	unsigned char Protocol;
	unsigned short Checksum;
	unsigned int SourceIP;
	unsigned int DestIP;
} IP_HDR;

typedef struct
{
	unsigned short SourcePort;
	unsigned short DestPort;
	unsigned short Length;
	unsigned short Checksum;
} UDP_HDR;

struct psd_udp {
	struct in_addr src;
	struct in_addr dst;
	unsigned char pad;
	unsigned char proto;
	unsigned short udp_len;
	UDP_HDR udp;
};

SOCKET sock[MAX_CLIENTS];
SOCKET sockDummies[MAX_CLIENTS][MAX_DUMMIES_PER_CLIENT];

int connectedDummies[MAX_CLIENTS];

unsigned int ipDummies[MAX_CLIENTS][MAX_DUMMIES_PER_CLIENT];
unsigned int ipDummiesSrv[MAX_CLIENTS];
int portDummiesSrv[MAX_CLIENTS];

unsigned char buffer[BUFLEN];

USHORT checksum(USHORT *buffer, int size)
{
	unsigned long cksum = 0;

	while (size > 1)
	{
		cksum += *buffer++;
		size -= sizeof(USHORT);
	}
	if (size)
	{
		cksum += *(UCHAR*)buffer;
	}
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);

	return (USHORT)(~cksum);
}

void SendData(int client, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort, const char *pData, int Size)
{
	SOCKADDR_IN m_Sin;
	unsigned short m_ChecksumUDP;
	char m_aPacket[MAX_PACKET];

	memset(&m_aPacket, 0, MAX_PACKET);

	IP_HDR IP;
	IP.Version = 4;
	IP.IHL = sizeof(IP_HDR) / sizeof(unsigned long);
	IP.DSCP = 0;
	IP.ECN = 1;
	IP.Length = htons(sizeof(IP_HDR) + sizeof(UDP_HDR) + Size);
	IP.ID = 0;
	IP.Flags = 0;
	IP.FragOffset = 0;
	IP.TTL = 128;
	IP.Protocol = IPPROTO_UDP;
	IP.Checksum = 0;
	IP.SourceIP = srcIp;
	IP.DestIP = dstIp;
	IP.Checksum = checksum((USHORT *)&IP, sizeof(IP));

	UDP_HDR UDP;
	UDP.SourcePort = srcPort;
	UDP.DestPort = dstPort;
	UDP.Length = htons(sizeof(UDP_HDR) + Size);
	UDP.Checksum = 0;

	char *ptr = NULL;

	m_ChecksumUDP = 0;
	ptr = m_aPacket;
	memset(&m_aPacket, 0, MAX_PACKET);

	memcpy(ptr, &IP.SourceIP, sizeof(IP.SourceIP));
	ptr += sizeof(IP.SourceIP);
	m_ChecksumUDP += sizeof(IP.SourceIP);

	memcpy(ptr, &IP.DestIP, sizeof(IP.DestIP));
	ptr += sizeof(IP.DestIP);
	m_ChecksumUDP += sizeof(IP.DestIP);

	ptr++;
	m_ChecksumUDP++;

	memcpy(ptr, &IP.Protocol, sizeof(IP.Protocol));
	ptr += sizeof(IP.Protocol);
	m_ChecksumUDP += sizeof(IP.Protocol);

	memcpy(ptr, &UDP.Length, sizeof(UDP.Length));
	ptr += sizeof(UDP.Length);
	m_ChecksumUDP += sizeof(UDP.Length);

	memcpy(ptr, &UDP, sizeof(UDP));
	ptr += sizeof(UDP);
	m_ChecksumUDP += sizeof(UDP);

	for (int i = 0; i < Size; i++, ptr++)
		*ptr = pData[i];

	m_ChecksumUDP += Size;

	UDP.Checksum = checksum((USHORT *)m_aPacket, m_ChecksumUDP);

	memset(&m_aPacket, 0, MAX_PACKET);
	ptr = m_aPacket;

	memcpy(ptr, &IP, sizeof(IP));
	ptr += sizeof(IP);

	memcpy(ptr, &UDP, sizeof(UDP));
	ptr += sizeof(UDP);

	memcpy(ptr, pData, Size);

	m_Sin.sin_family = AF_INET;
	m_Sin.sin_port = dstPort;
	m_Sin.sin_addr.s_addr = dstIp;

	sendto(sock[client], m_aPacket, sizeof(IP) + sizeof(UDP) + Size, 0, (SOCKADDR *)&m_Sin, sizeof(m_Sin));
}

void SendData(int client, int id, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort, const char *pData, int Size)
{
	SOCKADDR_IN m_Sin;
	unsigned short m_ChecksumUDP;
	char m_aPacket[MAX_PACKET];

	memset(&m_aPacket, 0, MAX_PACKET);

	IP_HDR IP;
	IP.Version = 4;
	IP.IHL = sizeof(IP_HDR) / sizeof(unsigned long);
	IP.DSCP = 0;
	IP.ECN = 1;
	IP.Length = htons(sizeof(IP_HDR) + sizeof(UDP_HDR) + Size);
	IP.ID = 0;
	IP.Flags = 0;
	IP.FragOffset = 0;
	IP.TTL = 128;
	IP.Protocol = IPPROTO_UDP;
	IP.Checksum = 0;
	IP.SourceIP = srcIp;
	IP.DestIP = dstIp;
	IP.Checksum = checksum((USHORT *)&IP, sizeof(IP));

	UDP_HDR UDP;
	UDP.SourcePort = srcPort;
	UDP.DestPort = dstPort;
	UDP.Length = htons(sizeof(UDP_HDR) + Size);
	UDP.Checksum = 0;

	char *ptr = NULL;

	m_ChecksumUDP = 0;
	ptr = m_aPacket;
	memset(m_aPacket, 0, MAX_PACKET);

	memcpy(ptr, &IP.SourceIP, sizeof(IP.SourceIP));
	ptr += sizeof(IP.SourceIP);
	m_ChecksumUDP += sizeof(IP.SourceIP);

	memcpy(ptr, &IP.DestIP, sizeof(IP.DestIP));
	ptr += sizeof(IP.DestIP);
	m_ChecksumUDP += sizeof(IP.DestIP);

	ptr++;
	m_ChecksumUDP++;

	memcpy(ptr, &IP.Protocol, sizeof(IP.Protocol));
	ptr += sizeof(IP.Protocol);
	m_ChecksumUDP += sizeof(IP.Protocol);

	memcpy(ptr, &UDP.Length, sizeof(UDP.Length));
	ptr += sizeof(UDP.Length);
	m_ChecksumUDP += sizeof(UDP.Length);

	memcpy(ptr, &UDP, sizeof(UDP));
	ptr += sizeof(UDP);
	m_ChecksumUDP += sizeof(UDP);

	for (int i = 0; i < Size; i++, ptr++)
		*ptr = pData[i];

	m_ChecksumUDP += Size;

	UDP.Checksum = checksum((USHORT *)m_aPacket, m_ChecksumUDP);

	memset(m_aPacket, 0, MAX_PACKET);
	ptr = m_aPacket;

	memcpy(ptr, &IP, sizeof(IP));
	ptr += sizeof(IP);

	memcpy(ptr, &UDP, sizeof(UDP));
	ptr += sizeof(UDP);

	memcpy(ptr, pData, Size);

	m_Sin.sin_family = AF_INET;
	m_Sin.sin_port = dstPort;
	m_Sin.sin_addr.s_addr = dstIp;

	sendto(sockDummies[client][id], m_aPacket, sizeof(IP) + sizeof(UDP) + Size, 0, (SOCKADDR *)&m_Sin, sizeof(m_Sin));
}

//even if it's dirty #NOHATE
int GetConnectedDummies(int client)
{
	return connectedDummies[client];
}

void SendChat(int client, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort, const char *msg)
{
	if (!Create(&sock[client]))
		Close(sock[client]);

	char aMsg[256];
	sprintf_s(aMsg, sizeof(aMsg), "%s", msg);

	Reset(client);
	memset(buffer, 0, BUFLEN);

	int bufferSize = PackSay(client, &buffer[0], aMsg, 0);
	SendData(client, srcIp, srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);
}

void SendKill(int client, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort)
{
	if (!Create(&sock[client]))
		Close(sock[client]);

	Reset(client);
	memset(buffer, 0, BUFLEN);

	int bufferSize = PackKill(client, &buffer[0]);
	SendData(client, srcIp, srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);
}

void SendDisconnect(int client, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort)
{
	if (!Create(&sock[client]))
		Close(sock[client]);

	Reset(client);
	memset(buffer, 0, BUFLEN);

	int bufferSize = PackDisconnect(client, &buffer[0]);
	SendData(client, srcIp, srcPort, dstIp, dstPort, (const char*)buffer, bufferSize);
}

void SendConnectDummies(int client, unsigned int dstIp, unsigned short dstPort, int amount)
{
	for (int i = 0; i < amount; i++) //generate random ips
		ipDummies[client][i] = inet_addr(GenerateIP());

	for (int i = 0; i < amount;i++) //generate sockets
		if (!Create(&sockDummies[client][i]))
			Close(sockDummies[client][i]);

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
	}
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
		if (!Create(&sockDummies[client][i]))
			Close(sockDummies[client][i]);

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

	if (!Create(&sock[client]))
		Close(sock[client]);

	int bufferSize = 0;
	char aMsg[256];

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
		sprintf_s(aMsg, sizeof(aMsg), "%s", Line.c_str());

		Reset(client);
		memset(buffer, 0, sizeof(buffer));

		// send the chat packet
		bufferSize = PackSay(client, &buffer[0], aMsg, 0);
		SendData(client, inet_addr(aSplit[0]), htons(atoi(aSplit[1])), dstIp, dstPort, (const char*)buffer, bufferSize);
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
