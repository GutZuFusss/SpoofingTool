#pragma once

/* Packet-crafting functions 
   d from dummies

   id = dummy id
   client = client id
*/
int PackSay_d(int client, int id, unsigned char *buffer, char *message, int team);
int PackSay(int client, unsigned char *buffer, char *message, int team);

int PackConnect_d(int client, int id, unsigned char *buffer);
int PackClientInfo_d(int client, int id, unsigned char *buffer);
int PackReady_d(int client, int id, unsigned char *buffer);
int PackSendInfo_d(int client, int id, unsigned char *buffer);
int PackEnterGame_d(int client, int id, unsigned char *buffer);

int PackKeepAlive_d(int client, int id, unsigned char *buffer);

int PackDisconnect_d(int client, int id, unsigned char *buffer);
int PackDisconnect(int client, unsigned char *buffer);

int PackVote_d(int client, int id, unsigned char *buffer, int v);
int PackVote(int client, unsigned char *buffer, int v);

int PackKill_d(int client, int id, unsigned char *buffer); 
int PackKill(int client, unsigned char *buffer);

//int PackChangeInfo(int client, unsigned char *buffer, char *name, char *clan, int country, char *skin, int usecustomcolor, int colorbody, int colorfeet);

//int PackRconAuth(int client, unsigned char *buffer);

int PackEmoticon_d(int client, int id, unsigned char *buffer, int e);
int PackEmoticon(int client, unsigned char *buffer, int e);

//int PackRcon(unsigned char *buffer, const char *pCmd);

void Reset_d(int client, int id);
void Reset(int client);
