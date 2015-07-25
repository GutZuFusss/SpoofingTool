#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdafx.h"

#include "api.h"

enum
{
	PACKER_BUFFER_SIZE = 1024 * 2
};
unsigned char m_aBuffer[PACKER_BUFFER_SIZE];

unsigned char *m_pCurrent;
unsigned char *m_pEnd;
int m_Error;

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

void Reset()
{
	m_Error = 0;
	m_pCurrent = m_aBuffer;
	m_pEnd = m_pCurrent + PACKER_BUFFER_SIZE;
}

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

int Size() { return (int)(m_pCurrent - m_aBuffer); }
const unsigned char *Data() { return m_aBuffer; }

unsigned char* PackHeader(unsigned char *pData, int m_Flags, int m_Size, int m_Sequence)
{
	pData[0] = ((m_Flags & 3) << 6) | ((m_Size >> 4) & 0x3f);
	pData[1] = (m_Size & 0xf);
	return pData + 2;
}

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

int PackChatMessage(unsigned char *Buffer, char *Message, int Team)
{
	int Flags = 0;
	int Sequence = 0;// 0 - 1023
	int Ack = 0;

	Flags &= ~8; // NO COMMPRESSION FLAG CUZ IT SUCKZ

	int BufferSize = 0;

	Buffer[0] = ((Flags << 4) & 0xf0) | ((Ack >> 8) & 0xf);
	Buffer[1] = Ack & 0xff;
	Buffer[2] = 1;//--ChunkNum

	BufferSize += 3;

	//space for the header

	BufferSize += 2;

	Reset();
	AddInt(NETMSGTYPE_CL_SAY); //--packet id
	AddInt(Team); //--team
	AddString(Message, -1); //--text
	m_aBuffer[0] <<= 1; // przesuniecie packet id

	memcpy(&Buffer[BufferSize], m_aBuffer, Size());

	BufferSize += Size();

	int sizebefore = BufferSize - Size() - 2;

	//printf("%d", sizebefore);

	PackHeader(&Buffer[sizebefore], 0, Size(), Sequence);

	int i;
	for (i = 0; i < BufferSize; i++)
	{
		printf("%02X", Buffer[i]);
	}

	printf("\n");
	
	return BufferSize;
}
