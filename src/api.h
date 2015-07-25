#ifndef _API_H
#define _API_H


int PackSay(unsigned char *buffer, char *message, int team);


int PackConnect(unsigned char *buffer);
int PackClientInfo(unsigned char *buffer);
int PackReady(unsigned char *buffer);
int PackSendInfo(unsigned char *buffer);
int PackEnterGame(unsigned char *buffer);

int PackRconAuth(unsigned char *buffer);

#endif