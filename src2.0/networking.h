#pragma once

#include "winsock2.h"
#include "ws2tcpip.h" //IP_HDRINCL is here

#pragma comment(lib,"ws2_32.lib") //winsock 2.2 library

#include "client.h"

bool CreateSocket(Client *pClient);
void CloseSocket(Client *pClient);

bool CreateSocket_d(Client *pClient, int id);
void CloseSocket_d(Client *pClient, int id);

void send(SOCKET s, char *text);

void SendData(Client *pClient, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort, const char *pData, int Size);
void SendData(Client *pClient, int id, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort, const char *pData, int Size);