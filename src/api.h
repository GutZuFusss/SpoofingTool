#pragma once

#include "client.h"

/* Packet-crafting functions 
   d for dummies

   id = dummy id
   pClient = pointer to client's api
*/
int PackSay_d(Client *pClient, int id, unsigned char *buffer, char *message, int team);
int PackSay(Client *pClient, unsigned char *buffer, char *message, int team);

int PackConnect_d(Client *pClient, int id, unsigned char *buffer);
int PackClientInfo_d(Client *pClient, int id, unsigned char *buffer);
int PackReady_d(Client *pClient, int id, unsigned char *buffer);
int PackSendInfo_d(Client *pClient, int id, unsigned char *buffer);
int PackEnterGame_d(Client *pClient, int id, unsigned char *buffer);

int PackKeepAlive_d(Client *pClient, int id, unsigned char *buffer);

int PackDisconnect_d(Client *pClient, int id, unsigned char *buffer);
int PackDisconnect(Client *pClient, unsigned char *buffer);

int PackVote_d(Client *pClient, int id, unsigned char *buffer, int v);
int PackVote(Client *pClient, unsigned char *buffer, int v);

int PackCallvote_d(Client *pClient, int id, unsigned char *buffer, const char *typ, const char *val, const char *rsn);
int PackCallvote(Client *pClient, unsigned char *buffer, const char *typ, const char *val, const char *rsn);

int PackKill_d(Client *pClient, int id, unsigned char *buffer); 
int PackKill(Client *pClient, unsigned char *buffer);

//int PackChangeInfo(int client, unsigned char *buffer, char *name, char *clan, int country, char *skin, int usecustomcolor, int colorbody, int colorfeet);

int PackRconAuth(Client *pClient, unsigned char *buffer, char *password="G3784NN3DuN008");
int PackRcon(Client *pClient, unsigned char *buffer, const char *pCmd);

int PackEmoticon_d(Client *pClient, int id, unsigned char *buffer, int e);
int PackEmoticon(Client *pClient, unsigned char *buffer, int e);


void Reset_d(Client *pClient, int id);
void Reset(Client *pClient);
