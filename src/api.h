#ifndef _API_H
#define _API_H


int PackSay(unsigned char *buffer, int id, char *message, int team);
int PackSay(unsigned char *buffer, char *message, int team);

int PackConnect(unsigned char *buffer, int id);
int PackClientInfo(unsigned char *buffer, int id);
int PackReady(unsigned char *buffer, int id);
int PackSendInfo(unsigned char *buffer, int id);
int PackEnterGame(unsigned char *buffer, int id);

int PackKeepAlive(unsigned char *buffer, int id);

int PackDisconnect(unsigned char *buffer);
int PackDisconnect(unsigned char *buffer, int id);

int PackVote(unsigned char *buffer, int v);
int PackVote(unsigned char *buffer, int id, int v);

int PackKill(unsigned char *buffer); 

int PackChangeInfo(unsigned char *buffer, char *name, char *clan, int country, char *skin, int usecustomcolor, int colorbody, int colorfeet);

int PackRconAuth(unsigned char *buffer);

int PackEmoticon(unsigned char *buffer, int id, int e);
int PackEmoticon(unsigned char *buffer, int e);

int PackRcon(unsigned char *buffer, const char *pCmd);

void Reset(int id);
void Reset();


#endif
