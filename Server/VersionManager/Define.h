#ifndef _DEFINE_H
#define _DEFINE_H

#include <string>
#include <mmsystem.h>

#define _LISTEN_PORT		15100

#define MAX_USER			3000
#define CLIENT_SOCKSIZE		10

#define MAX_ID_SIZE			20
////////////////////////////////////////////////////////////
// Socket Define
////////////////////////////////////////////////////////////
#define SOCKET_BUFF_SIZE	(1024*16)
#define MAX_PACKET_SIZE		(1024*8)

#define PACKET_START1				0XAA
#define PACKET_START2				0X55
#define PACKET_END1					0X55
#define PACKET_END2					0XAA

// status
#define STATE_CONNECTED			0X01
#define STATE_DISCONNECTED		0X02
#define STATE_GAMESTART			0x03

// Socket type
#define TYPE_ACCEPT				0x01
#define TYPE_CONNECT			0x02

// Overlapped flag
#define OVL_RECEIVE				0X01
#define OVL_SEND				0X02
#define OVL_CLOSE				0X03
////////////////////////////////////////////////////////////

typedef union {
	short int	i;
	BYTE		b[2];
} MYSHORT;

typedef union {
	int			i;
	BYTE		b[4];
} MYINT;

typedef union {
	DWORD		w;
	BYTE		b[4];
} MYDWORD;

struct _VERSION_INFO
{
	short		sVersion;
	short		sHistoryVersion;
	std::string	strFileName;
	std::string	strCompName;
};

struct _SERVER_INFO
{
	char	strServerIP[20];
	char	strServerName[20];
	short	sUserCount;

	_SERVER_INFO()
	{
		memset(strServerIP, 0, sizeof(strServerIP));
		memset(strServerName, 0, sizeof(strServerName));
		sUserCount = 0;
	}
};


// Packet Define...

#define LS_VERSION_REQ				0x01
#define LS_DOWNLOADINFO_REQ			0x02
#define LS_LOGIN_REQ				0xF3
#define LS_MGAME_LOGIN				0xF4
#define LS_SERVERLIST				0xF5

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

inline BYTE GetByte(char* sBuf, int& index)
{
	int t_index = index;
	index++;
	return (BYTE) (*(sBuf + t_index));
}

inline int GetShort(char* sBuf, int& index)
{
	index += 2;
	return *(short*) (sBuf + index - 2);
}

inline DWORD GetDWORD(char* sBuf, int& index)
{
	index += 4;
	return *(DWORD*) (sBuf + index - 4);
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
};

inline void SetByte(char* tBuf, BYTE sByte, int& index)
{
	*(tBuf + index) = (char) sByte;
	index++;
}

inline void SetShort(char* tBuf, int sShort, int& index)
{
	short temp = (short) sShort;

	CopyMemory(tBuf + index, &temp, 2);
	index += 2;
}

inline void SetDWORD(char* tBuf, DWORD sDWORD, int& index)
{
	CopyMemory(tBuf + index, &sDWORD, 4);
	index += 4;
}

inline void Setfloat(char* tBuf, float sFloat, int& index)
{
	CopyMemory(tBuf + index, &sFloat, 4);
	index += 4;
}

// sungyong 2001.11.06
inline int GetVarString(char* tBuf, char* sBuf, int nSize, int& index)
{
	int nLen = 0;

	if (nSize == sizeof(BYTE))
		nLen = GetByte(sBuf, index);
	else
		nLen = GetShort(sBuf, index);

	GetString(tBuf, sBuf, nLen, index);
	*(tBuf + nLen) = 0;

	return nLen;
}

inline void SetVarString(char* tBuf, char* sBuf, int len, int& index)
{
	*(tBuf + index) = (BYTE) len;
	index ++;

	CopyMemory(tBuf + index, sBuf, len);
	index += len;
}

inline CString GetProgPath()
{
	char Buf[256], Path[256];
	char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];

	::GetModuleFileName(AfxGetApp()->m_hInstance, Buf, 256);
	_splitpath(Buf, drive, dir, fname, ext);
	strcpy(Path, drive);
	strcat(Path, dir);
	CString _Path = Path;
	return _Path;
}

inline void LogFileWrite(LPCTSTR logstr)
{
	CString ProgPath, LogFileName;
	CFile file;
	int loglength;

	ProgPath = GetProgPath();
	loglength = strlen(logstr);

	LogFileName.Format("%s\\Login.log", ProgPath);

	file.Open(LogFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite);

	file.SeekToEnd();
	file.Write(logstr, loglength);
	file.Close();
}

inline int DisplayErrorMsg(SQLHANDLE hstmt)
{
	SQLCHAR       SqlState[6], Msg[1024];
	SQLINTEGER    NativeError;
	SQLSMALLINT   i, MsgLen;
	SQLRETURN     rc2;
	char		  logstr[512] = {};

	i = 1;
	while ((rc2 = SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, i, SqlState, &NativeError, Msg, sizeof(Msg), &MsgLen)) != SQL_NO_DATA)
	{
		sprintf(logstr, "*** %s, %d, %s, %d ***\r\n", SqlState, NativeError, Msg, MsgLen);
		LogFileWrite(logstr);

		i++;
	}

	if (strcmp((char*) SqlState, "08S01") == 0)
		return -1;
	else
		return 0;
}

#endif
