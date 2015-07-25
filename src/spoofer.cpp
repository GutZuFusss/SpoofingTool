/* Paszczak wrote this v2.0 */

// spoofer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "winsock2.h"
#include "ws2tcpip.h" //IP_HDRINCL is here
#include "conio.h"

#pragma comment(lib,"ws2_32.lib") //winsock 2.2 library

#include "spoofer.h"

#include "api.h"



void SendData(const char *pData, int Size)
{
	m_PayloadSize = Size;

	ZeroMemory(m_aMessage, MAX_MESSAGE);

	for (unsigned int i = 0; i < m_PayloadSize; i++)
		m_aMessage[i] = pData[i];

	IP_HDR IP;
	IP.Version = 4;
	IP.IHL = sizeof(IP_HDR) / sizeof(unsigned long);
	IP.DSCP = 0;
	IP.ECN = 1;
	IP.Length = htons(sizeof(IP_HDR) + sizeof(UDP_HDR) + m_PayloadSize);
	IP.ID = 0;
	IP.Flags = 0;
	IP.FragOffset = 0;
	IP.TTL = 128;
	IP.Protocol = IPPROTO_UDP;
	IP.Checksum = 0;
	IP.SourceIP = m_FromIP;
	IP.DestIP = m_ToIP;
	IP.Checksum = checksum((USHORT *)&IP, sizeof(IP));

	UDP_HDR UDP;
	UDP.SourcePort = m_FromPort;
	UDP.DestPort = m_ToPort;
	UDP.Length = htons(sizeof(UDP_HDR) + m_PayloadSize);
	UDP.Checksum = 0;

	char *ptr = NULL;

	m_ChecksumUDP = 0;
	ptr = m_aPacket;
	ZeroMemory(m_aPacket, MAX_PACKET);

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

	for (unsigned int i = 0; i < m_PayloadSize; i++, ptr++)
		*ptr = m_aMessage[i];
	m_ChecksumUDP += m_PayloadSize;

	UDP.Checksum = checksum((USHORT *)m_aPacket, m_ChecksumUDP);

	ZeroMemory(m_aPacket, MAX_PACKET);
	ptr = m_aPacket;

	memcpy(ptr, &IP, sizeof(IP));
	ptr += sizeof(IP);

	memcpy(ptr, &UDP, sizeof(UDP));
	ptr += sizeof(UDP);

	memcpy(ptr, m_aMessage, m_PayloadSize);

	m_Sin.sin_family = AF_INET;
	m_Sin.sin_port = m_ToPort;
	m_Sin.sin_addr.s_addr = m_ToIP;

	int Bytes = sendto(m_Sock, m_aPacket, sizeof(IP) + sizeof(UDP) + m_PayloadSize, 0, (SOCKADDR *)&m_Sin, sizeof(m_Sin));



	/*if(Bytes != SOCKET_ERROR)
	std::cout << Bytes << " sent" << std::endl; */


}

bool Create(SOCKET *pSock)
{
	WSADATA data;
	char aBuf[1024];

	if (WSAStartup(MAKEWORD(2, 2), &data))
	{
		sprintf_s(aBuf, sizeof(aBuf), "WSAStartup() failed: %d\n", GetLastError());
		Output(aBuf);
		return false;
	}

	*pSock = WSASocket(AF_INET, SOCK_RAW, IPPROTO_UDP, NULL, 0, 0);

	if (*pSock == INVALID_SOCKET)
	{
		sprintf_s(aBuf, sizeof(aBuf), "WSASocket() failed: %d\n", WSAGetLastError());
		Output(aBuf);
		return false;
	}

	BOOL optvalue = TRUE;

	if (setsockopt(*pSock, IPPROTO_IP, IP_HDRINCL, (char *)&optvalue, sizeof(optvalue)) == SOCKET_ERROR)
	{
		sprintf_s(aBuf, sizeof(aBuf), "setsockopt(IP_HDRINCL) failed: %d\n", WSAGetLastError());
		Output(aBuf);
		return false;
	}

	return true;
}

void Output(char *pBuf)
{
	//FILE *pFile = fopen("Sniffer.txt","a+"); 
	//fprintf(pFile,"%s", pBuf); 
	//fclose(pFile); 
	printf("%s", pBuf);
}

void Close()
{
	closesocket(m_Sock);
	WSACleanup();
	//getchar();
}

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

int _tmain(int argc, _TCHAR* argv[])
{
	/*int srcPort = 0, dstPort = 0;
	char srcIP[256];
	char dstIP[256];

	printf("source ip:\n");

	gets_s(srcIP);

	printf("source port:\n");

	scanf_s("%d", &srcPort);

	printf("destination ip:\n");

	gets_s(dstIP);

	printf("destination port:\n");

	scanf_s("%d", &dstPort);*/

	if (!Create(&m_Sock))
		Close();

	/*char aHostname[1024];
	gethostname(aHostname, 1024);
	struct hostent *pHostent;
	pHostent = gethostbyname(aHostname);

	m_ToIP = inet_addr("92.222.22.82");
	m_ToPort = htons(8303);

	m_FromIP = /*inet_addr("1.30.158.56");inet_addr(inet_ntoa(*(struct in_addr *)*pHostent->h_addr_list));
	m_FromPort = htons(1337);*/

	m_ToIP = inet_addr("92.222.64.188");
	m_ToPort = htons(8707);

	m_FromIP = inet_addr("192.168.100.10");
	m_FromPort = htons(1111);

	char message[256];

	//start communication
	while (1)
	{
		printf("Enter message: ");
		gets_s(message);

		unsigned char Buffer[2048];
		ZeroMemory(Buffer, sizeof(Buffer));
		int BufferSize = PackChatMessage(&Buffer[0], message, 0);

		//send the message
		SendData((const char*)Buffer, BufferSize);
	}

	Close();
	return 0;
}

