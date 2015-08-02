#ifndef _API_H
#define _API_H


int PackSay(unsigned char *buffer, int id, char *message, int team);

int PackConnect(unsigned char *buffer, int id);
int PackClientInfo(unsigned char *buffer, int id);
int PackReady(unsigned char *buffer, int id);
int PackSendInfo(unsigned char *buffer, int id);
int PackEnterGame(unsigned char *buffer, int id);

int PackKeepAlive(unsigned char *buffer, int id);

int PackDisconnect(unsigned char *buffer, int id);

int PackVote(unsigned char *buffer, int id, int v);
int PackKill(unsigned char *buffer, int id);

int PackRconAuth(unsigned char *buffer, int id);

int PackEmoticon(unsigned char *buffer, int id, int e);
int PackRcon(unsigned char *buffer, int id, const char *pCmd);

void Reset(int id);


#endif
