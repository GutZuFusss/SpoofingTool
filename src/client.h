#pragma once

#include "winsock2.h"
#include "core.h"

class Client
{
public:
	enum
	{
		UPDATE_THREAD = 0,
		WORKING_THREAD,
		NUM_THREADTYPES
	};

	class Packetgen
	{
	private:
		Client *m_pClient;

		int connectedDummies;
		unsigned int ipDummies[MAX_DUMMIES_PER_CLIENT];
		unsigned int ipDummiesSrv;
		int portDummiesSrv;

	public:
		Packetgen(Client *pClient) : m_pClient(pClient) {
			connectedDummies = 0;
			memset(ipDummies, 0, sizeof(ipDummies));
			ipDummiesSrv = 0;
			portDummiesSrv = 0;
		}

		Client *GetClient() const { return m_pClient; }

		int GetConnectedDummies() const { return connectedDummies; }

		void SendChat(unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort, const char *msg);
		void SendKill(unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort);
		void SendDisconnect(unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort);
		void SendVote(unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort, int v);
		void SendCallvote(unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort, const char *type, const char *value, const char *reason="");

		void SendConnectDummies(unsigned int dstIp, unsigned short dstPort, int amount, int vote, const char *chat=0);
		void SendDisconnectDummies(const char *chat=0);
		void SendChatDummies(const char *msg);
		void SendCallvoteDummy(int id, const char *type, const char *value, const char *reason="VOTEKECK!");
		void SendKeepAliveDummies();
		void SendEmoteDummies(int emoticon);

		void SendChatAll(unsigned int dstIp, unsigned short dstPort, const char *msg);
		void SendListIpAll(unsigned int dstIp, unsigned short dstPort);
		void SendVoteAll(unsigned int dstIp, unsigned short dstPort, int vote);
		void SendKillAll(unsigned int dstIp, unsigned short dstPort);
		void SendDisconnectAll(unsigned int dstIp, unsigned short dstPort);

		void SendRconAuth(unsigned int srcIp, unsigned short srcPort, unsigned int dstIp, unsigned short dstPort);
	};

public:
	struct
	{
		// dummies to play with
		int SequenceDummies[MAX_DUMMIES_PER_CLIENT];
		int AckDummies[MAX_DUMMIES_PER_CLIENT];

		// to spoof other guys
		int Sequence;
		int Ack;
	} m_Api;


	struct
	{
		SOCKET sock;
		SOCKET sockDummies[MAX_DUMMIES_PER_CLIENT];
	} m_Networking;


private:
	HANDLE handle[NUM_THREADTYPES];
	SOCKET socket;
	DWORD thread[NUM_THREADTYPES];

	Packetgen *m_pPacketgen;

	int id;
	int lastAck;

public:
	Client(int i, SOCKET s);
	~Client();

	int dummieSpam;
	char dummiesIP[64];
	int dummiesPort;

	inline SOCKET GetSocket() const { return socket; }
	inline Packetgen *GetPacketgen() const { return m_pPacketgen; }
	inline int GetID() const { return id; }
	inline int LastAck() const { return lastAck; }
	inline int DoAck() { return --lastAck; }
	inline int ResetAck() { return lastAck = TIMEOUT_SEC; }
	void Drop(bool dc = false);
};