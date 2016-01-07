#pragma once

#include "winsock2.h"
#include "ws2tcpip.h" //IP_HDRINCL is here

#pragma comment(lib,"ws2_32.lib") //winsock 2.2 library

bool CreateSocket(int client);
void CloseSocket(int client);

bool CreateSocket_d(int client, int id);
void CloseSocket_d(int client, int id);

void send(SOCKET s, char *text);

void SendData(int client, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort, const char *pData, int Size);
void SendData(int client, int id, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort, const char *pData, int Size);