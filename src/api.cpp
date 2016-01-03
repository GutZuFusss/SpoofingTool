#include "stdafx.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"
#include "dummydata.h"

enum
{
	PACKER_BUFFER_SIZE = 1024 * 4
};

unsigned char m_aBuffer[PACKER_BUFFER_SIZE];

unsigned char *m_pCurrent;
unsigned char *m_pEnd;
int m_Error;

int SequenceDummies[64];// 0 - 1023
int AckDummies[64];

int Sequence;
int Ack;

/* Used to compress the packets */
unsigned char* CompressionPack(unsigned char *pDst, int i)
{
	*pDst = (i >> 25) & 0x40; // set sign bit if i<0
	i = i ^ (i >> 31); // if(i<0) i = ~i

	*pDst |= i & 0x3F; // pack 6bit into dst
	i >>= 6; // discard 6 bits
	if (i)
	{
		*pDst |= 0x80; // set extend bit
		while (1)
		{
			pDst++;
			*pDst = i&(0x7F); // pack 7bit
			i >>= 7; // discard 7 bits
			*pDst |= (i != 0) << 7; // set extend bit (may branch)
			if (!i)
				break;
		}
	}

	pDst++;
	return pDst;
}

/* Resetting of the packer */
void ResetPacker()
{
	m_Error = 0;
	m_pCurrent = m_aBuffer;
	m_pEnd = m_pCurrent + PACKER_BUFFER_SIZE;
}

/* Easier random numbers */
int GetRand(int Start, int End)
{
    int randnum = End + rand() / (RAND_MAX / (Start - End + 1) + 1);
    
    return randnum;
}

/* Append an integer to the packet */
void AddInt(int i)
{
	if (m_Error)
		return;

	// make sure that we have space enough
	if (m_pEnd - m_pCurrent < 6)
	{
		m_Error = 1;
	}
	else
		m_pCurrent = CompressionPack(m_pCurrent, i);
}

/* Append a string to the packet */
void AddString(const char *pStr, int Limit)
{
	if (m_Error)
		return;

	//
	if (Limit > 0)
	{
		while (*pStr && Limit != 0)
		{
			*m_pCurrent++ = *pStr++;
			Limit--;

			if (m_pCurrent >= m_pEnd)
			{
				m_Error = 1;
				break;
			}
		}
		*m_pCurrent++ = 0;
	}
	else
	{
		while (*pStr)
		{
			*m_pCurrent++ = *pStr++;

			if (m_pCurrent >= m_pEnd)
			{
				m_Error = 1;
				break;
			}
		}
		*m_pCurrent++ = 0;
	}
}

/* Get the size of the packet we are currently building up */
int Size() { return (int)(m_pCurrent - m_aBuffer); }
/* Returns the buffer that contains the packet data */
const unsigned char *Data() { return m_aBuffer; }

/* enum with the IDs of the different package types */
enum
{
	NETMSG_NULL = 0,

	// the first thing sent by the client
	// contains the version info for the client
	NETMSG_INFO = 1,

	// sent by server
	NETMSG_MAP_CHANGE,		// sent when client should switch map
	NETMSG_MAP_DATA,		// map transfer, contains a chunk of the map file
	NETMSG_CON_READY,		// connection is ready, client should send start info
	NETMSG_SNAP,			// normal snapshot, multiple parts
	NETMSG_SNAPEMPTY,		// empty snapshot
	NETMSG_SNAPSINGLE,		// ?
	NETMSG_SNAPSMALL,		//
	NETMSG_INPUTTIMING,		// reports how off the input was
	NETMSG_RCON_AUTH_STATUS,// result of the authentication
	NETMSG_RCON_LINE,		// line that should be printed to the remote console

	NETMSG_AUTH_CHALLANGE,	//
	NETMSG_AUTH_RESULT,		//

	// sent by client
	NETMSG_READY,			//
	NETMSG_ENTERGAME,
	NETMSG_INPUT,			// contains the inputdata from the client
	NETMSG_RCON_CMD,		//
	NETMSG_RCON_AUTH,		//
	NETMSG_REQUEST_MAP_DATA,//

	NETMSG_AUTH_START,		//
	NETMSG_AUTH_RESPONSE,	//

	// sent by both
	NETMSG_PING,
	NETMSG_PING_REPLY,
	NETMSG_ERROR,

	// sent by server (todo: move it up)
	NETMSG_RCON_CMD_ADD,
	NETMSG_RCON_CMD_REM,
};

/* Additional message types */
enum
{
	NETMSG_INVALID = 0,
	NETMSGTYPE_SV_MOTD,
	NETMSGTYPE_SV_BROADCAST,
	NETMSGTYPE_SV_CHAT,
	NETMSGTYPE_SV_KILLMSG,
	NETMSGTYPE_SV_SOUNDGLOBAL,
	NETMSGTYPE_SV_TUNEPARAMS,
	NETMSGTYPE_SV_EXTRAPROJECTILE,
	NETMSGTYPE_SV_READYTOENTER,
	NETMSGTYPE_SV_WEAPONPICKUP,
	NETMSGTYPE_SV_EMOTICON,
	NETMSGTYPE_SV_VOTECLEAROPTIONS,
	NETMSGTYPE_SV_VOTEOPTIONLISTADD,
	NETMSGTYPE_SV_VOTEOPTIONADD,
	NETMSGTYPE_SV_VOTEOPTIONREMOVE,
	NETMSGTYPE_SV_VOTESET,
	NETMSGTYPE_SV_VOTESTATUS,
	NETMSGTYPE_CL_SAY,
	NETMSGTYPE_CL_SETTEAM,
	NETMSGTYPE_CL_SETSPECTATORMODE,
	NETMSGTYPE_CL_STARTINFO,
	NETMSGTYPE_CL_CHANGEINFO,
	NETMSGTYPE_CL_KILL,
	NETMSGTYPE_CL_EMOTICON,
	NETMSGTYPE_CL_VOTE,
	NETMSGTYPE_CL_CALLVOTE,
	NETMSGTYPE_CL_ISDDNET,
	NETMSGTYPE_SV_DDRACETIME,
	NETMSGTYPE_SV_RECORD,
	NETMSGTYPE_SV_PLAYERTIME,
	NETMSGTYPE_SV_TEAMSSTATE,
	NETMSGTYPE_CL_SHOWOTHERS,
	NUM_NETMSGTYPES
};

/* VITAL and FLUSH flags, see teeworlds source to know when to use which flag */
enum
{
	FLAGS_VITAL = 1,
	FLAGS_FLUSH = 4
};

/* Some core control messages used to connect the dummies */
enum
{
	NET_CTRLMSG_KEEPALIVE = 0,
	NET_CTRLMSG_CONNECT = 1,
	NET_CTRLMSG_CLOSE = 4
};

/* Couldn't get any of the stuff working with this going yet */
enum
{
	NET_PACKETFLAG_CONTROL = 16,
};

/* Make the header of the packet ready to send, applies flags and more */
unsigned char* PackHeader(unsigned char *pData, int fl /*flags*/, int si /*size*/, int sq /*sequence*/)
{
	pData[0] = ((f & 3) << 6) | ((s >> 4) & 0x3f);
	pData[1] = (s & 0xf);
	if (f&FLAGS_VITAL)
	{
		pData[1] |= (sq >> 2) & 0xf0;
		pData[2] = sq & 0xff;

		return pData + 3;
	}
	return pData + 2;
}

/* Begin crafting our packet */
int StartofPacking(unsigned char *buffer, int flags = FLAGS_FLUSH)
{
	int Flags = 0;
	//int Sequence = 0;// 0 - 1023
	//int Ack = 0;

	Ack = GetRand(1, 1024);

	Flags &= ~8; // NO COMMPRESSION FLAG CUZ IT SUCKZ

	int BufferSize = 0;

	if (flags == NET_PACKETFLAG_CONTROL)
		buffer[0] = ((1 << 4) & 0xf0) | ((Ack >> 8) & 0xf);
	else
		buffer[0] = ((Flags << 4) & 0xf0) | ((Ack >> 8) & 0xf);

	buffer[1] = Ack & 0xff;

	if (flags == NET_PACKETFLAG_CONTROL)
		buffer[2] = 0;//--ChunkNum
	else
		buffer[2] = 1;//--ChunkNum

	BufferSize += 3;

	// space for the header

	if (flags == NET_PACKETFLAG_CONTROL)
	{
		Sequence = 0;
		//++Ack[id] %= 1024;
		return BufferSize;
	}

	if (flags&FLAGS_VITAL)
	{
		BufferSize += 3;
		Sequence = (Sequence + 1) % 1024;
		//++Ack %= 1024;
	}
	else
		BufferSize += 2;

	ResetPacker();

	return BufferSize;
}

/* Another function to begin the crafting of the packet with an additional "id" parameter to use a specific socket*/
int StartofPacking(int id, unsigned char *buffer, int flags = FLAGS_FLUSH)
{
	int Flags = 0;
	//int Sequence = 0;// 0 - 1023
	//int Ack = 0;

	AckDummies[id] = GetRand(1, 1024);

	Flags &= ~8; // NO COMMPRESSION FLAG CUZ IT SUCKZ

	int BufferSize = 0;

	if (flags == NET_PACKETFLAG_CONTROL)
		buffer[0] = ((1 << 4) & 0xf0) | ((AckDummies[id] >> 8) & 0xf);
	else
		buffer[0] = ((Flags << 4) & 0xf0) | ((AckDummies[id] >> 8) & 0xf);

	buffer[1] = AckDummies[id] & 0xff;

	if (flags == NET_PACKETFLAG_CONTROL)
		buffer[2] = 0;//--ChunkNum
	else
		buffer[2] = 1;//--ChunkNum

	BufferSize += 3;

	//space for the header

	if (flags == NET_PACKETFLAG_CONTROL)
	{
		SequenceDummies[id] = 0;
		//++AckDummies[id] %= 1024;
		return BufferSize;
	}
	
	if (flags&FLAGS_VITAL)
	{
		BufferSize += 3;
		SequenceDummies[id] = (SequenceDummies[id] + 1) % 1024;
		//++Ack %= 1024;
	}
	else
		BufferSize += 2;

	ResetPacker();
	
	return BufferSize;
}

/* Finalize the crafting of the packet */
int EndofPacking(unsigned char *buffer, int buffersize, int flags, bool system = false)
{
	m_aBuffer[0] <<= 1; // shift the packet id
	if (system)
		m_aBuffer[0] |= 1;

	memcpy(&buffer[buffersize], m_aBuffer, Size());

	buffersize += Size();

	//printf("%d", sizebefore);
	int sizebefore = 0;
	if (flags&FLAGS_VITAL)
		sizebefore = buffersize - Size() - 3;
	else
		sizebefore = buffersize - Size() - 2;

	PackHeader(&buffer[sizebefore], flags, Size(), Sequence); // the header

	return buffersize;
}

/* Also for finalizing the crafting of the packet, again with additional id parameter to define the socket to use*/
int EndofPacking(unsigned char *buffer, int buffersize, int id, int flags, bool system = false)
{
	m_aBuffer[0] <<= 1; // shift the packet id
	if(system)
		m_aBuffer[0] |= 1;

	memcpy(&buffer[buffersize], m_aBuffer, Size());

	buffersize += Size();

	//printf("%d", sizebefore);
	int sizebefore = 0;
	if (flags&FLAGS_VITAL)
		sizebefore = buffersize - Size() - 3;
	else
		sizebefore = buffersize - Size() - 2;

	PackHeader(&buffer[sizebefore], flags, Size(), SequenceDummies[id]); // the header

	return buffersize;
}

/* Craft a chatmessage (id) */
int PackSay(unsigned char *buffer, int id, char *message, int team)
{
	int BufferSize = StartofPacking(id, buffer, FLAGS_FLUSH);

	AddInt(NETMSGTYPE_CL_SAY); //--packet id
	AddInt(team); //--team
	AddString(message, -1); //--text
	
	return EndofPacking(buffer, BufferSize, id, FLAGS_FLUSH);
}

/* Craft a chatmessage */
int PackSay(unsigned char *buffer, char *message, int team)
{
	int BufferSize = StartofPacking(buffer, FLAGS_FLUSH);

	AddInt(NETMSGTYPE_CL_SAY); //--packet id
	AddInt(team); //--team
	AddString(message, -1); //--text

	return EndofPacking(buffer, BufferSize, FLAGS_FLUSH);
}

/* Craft a connect packet */
int PackConnect(unsigned char *buffer, int id)
{
	//memcpy((char *)buffer, "\x10\x00\x00\x01", 4);
	//return 4;

	int BufferSize = StartofPacking(id, buffer, NET_PACKETFLAG_CONTROL);

	buffer[BufferSize] = NET_CTRLMSG_CONNECT;

	BufferSize++;

	return BufferSize;
}

/* Craft a keep-alive packet */
int PackKeepAlive(unsigned char *buffer, int id)
{
	int BufferSize = StartofPacking(id, buffer, NET_PACKETFLAG_CONTROL);

	buffer[BufferSize] = NET_CTRLMSG_KEEPALIVE;

	BufferSize++;

	return BufferSize;
}

/* Craft a packet containing the client's info */
int PackClientInfo(unsigned char *buffer, int id)
{
	int BufferSize = StartofPacking(id, buffer, FLAGS_VITAL | FLAGS_FLUSH);

	AddInt(NETMSG_INFO); //--packet id
	AddString("0.6 626fce9a778df4d4", 128);// GAME_NETVERSION "0.6 626fce9a778df4d4"
	AddString("kek", 128); // password (to teh server?)

	return EndofPacking(buffer, BufferSize, id, FLAGS_VITAL | FLAGS_FLUSH, true);

	//Sequence++;
	//memcpy((char *)buffer, "\x00\x00\x01\x42\x02\x01\x03\x30\x2e\x36\x20\x36\x32\x36\x66\x63\x65\x39\x61\x37\x37\x38\x64\x66\x34\x64\x34\x00\x62\x61\x6e\x61\x6e\x65\x6e\x62\x61\x75\x6d\x00", 40);
	//return 40;
}

/* Craft a ready packet*/
int PackReady(unsigned char *buffer, int id)
{
	int BufferSize = StartofPacking(id, buffer, FLAGS_VITAL | FLAGS_FLUSH);

	AddInt(NETMSG_READY); //--packet id

	return EndofPacking(buffer, BufferSize, id, FLAGS_VITAL | FLAGS_FLUSH, true);
	
	
	//memcpy((char *)buffer, "\x00\x01\x01\x40\x01\x02\x1d", 7);
	//return 7;
}

/* Craft the enter-game packet */
int PackEnterGame(unsigned char *buffer, int id)
{
	int BufferSize = StartofPacking(id, buffer, FLAGS_VITAL | FLAGS_FLUSH);

	AddInt(NETMSG_ENTERGAME); //--packet id

	return EndofPacking(buffer, BufferSize, id, FLAGS_VITAL | FLAGS_FLUSH, true);

	//memcpy((char *)buffer, "\x00\x06\x01\x40\x01\x04\x1f", 7);
	//return 7;
}

/* Craft the send-info packet (change info of the dummies here) */
int PackSendInfo(unsigned char *buffer, int id)
{
	int BufferSize = StartofPacking(id, buffer, FLAGS_VITAL | FLAGS_FLUSH);

	AddInt(NETMSGTYPE_CL_STARTINFO);
	AddString(pNames[id], -1);// nick
	AddString("69", -1);// clan
	AddInt(-1);// country
	AddString(pSkins[rand()%15], -1);// skin
	AddInt(0);// use default colors
	AddInt(65048);// body
	AddInt(65048);// feet

	return EndofPacking(buffer, BufferSize, id, FLAGS_VITAL | FLAGS_FLUSH);

	//memcpy((char *)buffer, "\x00\x03\x01\x43\x0e\x03\x28\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x00\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x00\x40\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x61\x00\x01\x80\xfc\xc7\x05\x80\xfc\x07", 68);
	//return 68;
}

/* Craft the disconnect packet (id) */
int PackDisconnect(unsigned char *buffer, int id)
{
	int BufferSize = StartofPacking(id, buffer, NET_PACKETFLAG_CONTROL);

	buffer[BufferSize] = NET_CTRLMSG_CLOSE;

	BufferSize++;

	return BufferSize;
}

/* Craft the disconnect packet */
int PackDisconnect(unsigned char *buffer)
{
	int BufferSize = StartofPacking(buffer, NET_PACKETFLAG_CONTROL);

	buffer[BufferSize] = NET_CTRLMSG_CLOSE;

	BufferSize++;

	return BufferSize;
}

/* Craft the voting packet (id) */
int PackVote(unsigned char *buffer, int id, int v)
{
	int BufferSize = StartofPacking(id, buffer, FLAGS_FLUSH);

	AddInt(NETMSGTYPE_CL_VOTE);
	AddInt(v);
	return EndofPacking(buffer, BufferSize, id, FLAGS_FLUSH);
}

/* Craft the voting packet */
int PackVote(unsigned char *buffer, int v)
{
	int BufferSize = StartofPacking(buffer, FLAGS_FLUSH);

	AddInt(NETMSGTYPE_CL_VOTE);
	AddInt(v);
	return EndofPacking(buffer, BufferSize, FLAGS_FLUSH);
}

/* Craft the kill packet (id) */
int PackKill(unsigned char *buffer, int id)
{
	int BufferSize = StartofPacking(id, buffer, FLAGS_FLUSH);

	AddInt(NETMSGTYPE_CL_KILL);
	return EndofPacking(buffer, BufferSize, id, FLAGS_FLUSH);
}

/* Craft the kill packet */
int PackKill(unsigned char *buffer)
{
	int BufferSize = StartofPacking(buffer, FLAGS_FLUSH);

	AddInt(NETMSGTYPE_CL_KILL);
	return EndofPacking(buffer, BufferSize, FLAGS_FLUSH);
}

/* TODO: Get this working (great for banning people) */
int PackRconAuth(unsigned char *buffer)
{
	/*int BufferSize = StartofPacking(buffer, id, FLAGS_FLUSH);

	AddInt(NETMSG_RCON_AUTH);
	AddString("", 32);
	AddString("wrongpw", 32);
	AddInt(1);
	return EndofPacking(buffer, BufferSize, id, FLAGS_FLUSH);*/
	memcpy((char *)buffer, "\x10\x00\x01\x00\x07\x24\x00\x31\x32\x33\x00\x01", 12);
	return 12;
}

/* Craft the packet containing a rcon line */
int PackRcon(unsigned char *buffer, const char *pCmd)
{
	int BufferSize = StartofPacking(buffer, FLAGS_VITAL);

	AddInt(NETMSG_RCON_AUTH);
	AddString(pCmd, 256);
	return EndofPacking(buffer, BufferSize, FLAGS_VITAL);
}

/* Crafts the emoticon packet (id) */
int PackEmoticon(unsigned char *buffer, int id, int e)
{
	int BufferSize = StartofPacking(id, buffer, FLAGS_FLUSH);

	AddInt(NETMSGTYPE_CL_EMOTICON);
	AddInt(e);
	return EndofPacking(buffer, BufferSize, id, FLAGS_FLUSH);
}

/* Crafts the emoticon packet */
int PackEmoticon(unsigned char *buffer, int e)
{
	int BufferSize = StartofPacking(buffer, FLAGS_FLUSH);

	AddInt(NETMSGTYPE_CL_EMOTICON);
	AddInt(e);
	return EndofPacking(buffer, BufferSize, FLAGS_FLUSH);
}

/* Craft the change-info packet */
int PackChangeInfo(unsigned char *buffer, char *name, char *clan, int country, char *skin, int usecustomcolor, int colorbody, int colorfeet)
{
	int BufferSize = StartofPacking(buffer, FLAGS_VITAL);

	AddInt(NETMSGTYPE_CL_CHANGEINFO);
	AddString(name, -1);
	AddString(clan, -1);
	AddInt(country);
	AddString(skin, -1);
	AddInt(usecustomcolor);
	AddInt(colorbody);
	AddInt(colorfeet);

	return EndofPacking(buffer, BufferSize, FLAGS_VITAL);
}

/* Reset a dummy (id) */
void Reset(int id)
{
	SequenceDummies[id] = 0;
	AckDummies[id] = 0;
	ResetPacker();
}

/* The complete packer, inluding the ack and the seq */
void Reset()
{
	Sequence = 0;
	Ack = 0;
	ResetPacker();
}
