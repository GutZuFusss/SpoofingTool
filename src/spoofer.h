#ifndef _SPOOFER_H
#define _SPOOFER_H

#define MAX_PACKET 4096
#define MAX_MESSAGE 4086
#define BUFLEN 2048

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




void SendData(const char *pData, int Size);
bool Create(SOCKET *pSock);
void Output(char *pBuf);
void Close();
USHORT checksum(USHORT *buffer, int size);

#endif