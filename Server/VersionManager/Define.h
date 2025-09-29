#ifndef _DEFINE_H
#define _DEFINE_H

#if defined(_DEBUG)
#include <iostream>
#endif
#include <string>
#include <mmsystem.h>

#include <shared/globals.h>
#include <shared/StringConversion.h>
#include <shared/STLMap.h>

constexpr int MAX_USER				= 3000;

constexpr int _LISTEN_PORT			= 15100;
constexpr int DB_PROCESS_TIMEOUT	= 100;

////////////////////////////////////////////////////////////
// Socket Define
////////////////////////////////////////////////////////////
#define SOCKET_BUFF_SIZE		(1024*16)
#define MAX_PACKET_SIZE			(1024*8)

#define PACKET_START1			0XAA
#define PACKET_START2			0X55
#define PACKET_END1				0X55
#define PACKET_END2				0XAA

// status
#define STATE_CONNECTED			0X01
#define STATE_DISCONNECTED		0X02
#define STATE_GAMESTART			0x03
////////////////////////////////////////////////////////////

typedef union
{
	int16_t		i;
	uint8_t		b[2];
} MYSHORT;

typedef union
{
	int32_t		i;
	uint8_t		b[4];
} MYINT;

typedef union
{
	uint32_t	w;
	uint8_t		b[4];
} MYDWORD;

import VersionManagerModel;
namespace model = versionmanager_model; 

struct _NEWS
{
	char Content[4096]	= {};
	int16_t Size		= 0;
};

struct _SERVER_INFO
{
	char	strServerIP[20]		= {};
	char	strServerName[20]	= {};
	int16_t	sUserCount			= 0;
	int16_t	sUserLimit			= 0;
	int16_t	sServerID			= 1;
};

typedef CSTLMap <model::Version>	VersionInfoList;

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
//	Global Function Define
//
// sungyong 2001.11.06

inline void GetString(char* tBuf, char* sBuf, int len, int& index)
{
	memcpy(tBuf, sBuf + index, len);
	index += len;
}

inline uint8_t GetByte(char* sBuf, int& index)
{
	int t_index = index;
	index++;
	return (uint8_t) (*(sBuf + t_index));
}

inline int GetShort(char* sBuf, int& index)
{
	index += 2;
	return *(int16_t*) (sBuf + index - 2);
}

inline uint32_t GetDWORD(char* sBuf, int& index)
{
	index += 4;
	return *(uint32_t*) (sBuf + index - 4);
}

inline float Getfloat(char* sBuf, int& index)
{
	index += 4;
	return *(float*) (sBuf + index - 4);
}

inline void SetString(char* tBuf, const char* sBuf, int len, int& index)
{
	memcpy(tBuf + index, sBuf, len);
	index += len;
}

inline void SetByte(char* tBuf, uint8_t sByte, int& index)
{
	*(tBuf + index) = (char) sByte;
	index++;
}

inline void SetShort(char* tBuf, int sShort, int& index)
{
	int16_t temp = (int16_t) sShort;

	memcpy(tBuf + index, &temp, 2);
	index += 2;
}

inline void SetDWORD(char* tBuf, uint32_t sDWORD, int& index)
{
	memcpy(tBuf + index, &sDWORD, 4);
	index += 4;
}

inline void Setfloat(char* tBuf, float sFloat, int& index)
{
	memcpy(tBuf + index, &sFloat, 4);
	index += 4;
}

inline void SetString1(char* tBuf, const char* sBuf, uint8_t len, int& index)
{
	SetByte(tBuf, len, index);
	SetString(tBuf, sBuf, len, index);
}

inline void SetString2(char* tBuf, const char* sBuf, int16_t len, int& index)
{
	SetShort(tBuf, len, index);
	SetString(tBuf, sBuf, len, index);
}

// sungyong 2001.11.06
inline int GetVarString(char* tBuf, char* sBuf, int nSize, int& index)
{
	int nLen = 0;

	if (nSize == sizeof(uint8_t))
		nLen = GetByte(sBuf, index);
	else
		nLen = GetShort(sBuf, index);

	GetString(tBuf, sBuf, nLen, index);
	*(tBuf + nLen) = 0;

	return nLen;
}

inline void SetVarString(char* tBuf, char* sBuf, int len, int& index)
{
	*(tBuf + index) = (uint8_t) len;
	index ++;

	memcpy(tBuf + index, sBuf, len);
	index += len;
}

inline CString GetProgPath()
{
	TCHAR Buf[256], Path[256];
	TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];

	::GetModuleFileName(AfxGetApp()->m_hInstance, Buf, 256);
	_tsplitpath(Buf, drive, dir, fname, ext);
	_tcscpy(Path, drive);
	_tcscat(Path, dir);
	return Path;
}

// ini config variable names
namespace ini
{
	// ODBC Config Section
	static constexpr char ODBC[] = "ODBC";
	static constexpr char DSN[] = "DSN";
	static constexpr char UID[] = "UID";
	static constexpr char PWD[] = "PWD";
	static constexpr char TABLE[] = "TABLE";

	// CONFIGURATION section
	static constexpr char CONFIGURATION[] = "CONFIGURATION";
	static constexpr char DEFAULT_PATH[] = "DEFAULT_PATH";

	// SERVER_LIST section
	static constexpr char SERVER_LIST[] = "SERVER_LIST";
	static constexpr char COUNT[] = "COUNT";

	// Download section
	static constexpr char DOWNLOAD[] = "DOWNLOAD";
	static constexpr char URL[] = "URL";
	static constexpr char PATH[] = "PATH";
}

#endif
