#pragma once

#include "winsock2.h"
#include "ws2tcpip.h" //IP_HDRINCL is here

#pragma comment(lib,"ws2_32.lib") //winsock 2.2 library



/* Defines */
#define MAX_CLIENTS 64
#define MAX_DUMMIES_PER_CLIENT 64
#define BUFLEN 2048

#define DUMMIES_PORT 1337

#define TIMEOUT 60
#define TIMEOUT_SEC TIMEOUT + 1 //we kick on 1, not 0





void Output(char *pBuf);

bool Create(SOCKET *pSock);
void Close(SOCKET sock);

void send(SOCKET s, char *text);

const char *GenerateIP();
void exec(char* cmd);
