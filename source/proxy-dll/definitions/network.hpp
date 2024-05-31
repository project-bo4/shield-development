#pragma once

namespace game
{
	struct netipv4_t
	{
		byte a;
		byte b;
		byte c;
		byte d;
	};

	enum netadrtype_t
	{
		NA_BOT = 0x0,
		NA_BAD = 0x1,
		NA_LOOPBACK = 0x2,
		NA_RAWIP = 0x3,
		NA_IP = 0x4,
	};

	enum netsrc_t
	{
		NS_NULL = -1,
		NS_CLIENT1 = 0x0,
		NS_CLIENT2 = 0x1,
		NS_CLIENT3 = 0x2,
		NS_CLIENT4 = 0x3,
		NS_SERVER = 0x4,
		NS_MAXCLIENTS = 0x4,
		NS_PACKET = 0x5,
	};

	struct netadr_t
	{
		union
		{
			netipv4_t ipv4;
			uint32_t addr;
		};

		uint16_t port;
		netadrtype_t type;
		netsrc_t localNetID;
	};

	struct msg_t
	{
		byte* data;           //0x0000 
		int maxsize;      //0x0008 
		int cursize;      //0x000C 
		int readcount;    //0x0010 
		char pad_unknown[32]; //0x0014
		bool unknown;         //0x0034 
		bool overflowed;      //0x0035 
		bool readOnly;        //0x0036 
	};

}