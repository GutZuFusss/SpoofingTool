#ifndef _API_H
#define _API_H


int PackSay(unsigned char *buffer, char *message, int team);

int PackConnect(unsigned char *buffer, int id);
int PackClientInfo(unsigned char *buffer, int id);
int PackReady(unsigned char *buffer, int id);
int PackSendInfo(unsigned char *buffer, int id);
int PackEnterGame(unsigned char *buffer, int id);

int PackKeepAlive(unsigned char *buffer, int id);

int PackDisconnect(unsigned char *buffer, int id);

void Reset(int id);


#endif