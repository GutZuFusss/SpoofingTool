#pragma once

int GetConnectedDummies(int client);

void SendChat(int client, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort, const char *msg);
void SendKill(int client, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort);
void SendDisconnect(int client, unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort);

void SendConnectDummies(int client, unsigned int dstIp, unsigned short dstPort, int amount);
void SendDisconnectDummies(int client);
void SendChatDummies(int client, const char *msg);
void SendKeepAliveDummies(int client);
void SendEmoteDummies(int client, int emoticon);

void SendChatAll(int client, unsigned int dstIp, unsigned short dstPort, const char *msg);
void SendListIpAll(int client, unsigned int dstIp, unsigned short dstPort);
void SendKillAll(int client, unsigned int dstIp, unsigned short dstPort);
void SendDisconnectAll(int client, unsigned int dstIp, unsigned short dstPort);
