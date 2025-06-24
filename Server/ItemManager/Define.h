#ifndef _DEFINE_H
#define _DEFINE_H

#include <shared/globals.h>

#define MAX_ID_SIZE			20
#define MAX_PW_SIZE			12
#define MAX_ITEM			28

////////////////////////////////////////////////////////////
// Socket Define
////////////////////////////////////////////////////////////
#define SOCKET_BUFF_SIZE	(1024*8)
#define MAX_PACKET_SIZE		(1024*2)

#define PACKET_START1		0XAA
#define PACKET_START2		0X55
#define PACKET_END1			0X55
#define PACKET_END2			0XAA
//#define PROTOCOL_VER		0X01

// status
#define STATE_CONNECTED		0X01
#define STATE_DISCONNECTED	0X02
#define STATE_GAMESTART		0x03

/////////////////////////////////////////////////////
// ITEM_SLOT DEFINE
#define RIGHTEAR			0
#define HEAD				1
#define LEFTEAR				2
#define NECK				3
#define BREAST				4
#define SHOULDER			5
#define RIGHTHAND			6
#define WAIST				7
#define LEFTHAND			8
#define RIGHTRING			9
#define LEG					10
#define LEFTRING			11
#define GLOVE				12
#define FOOT				13
/////////////////////////////////////////////////////

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


// DEFINE MACRO PART...
#define BufInc(x) (x)++;(x) %= SOCKET_BUF_SIZE;

// DEFINE Shared Memory Queue Flag

#define E				0x00
#define R				0x01
#define W				0x02
#define WR				0x03

// DEFINE Shared Memory Queue Return VALUE

#define SMQ_BROKEN		10000
#define SMQ_FULL		10001
#define SMQ_EMPTY		10002
#define SMQ_PKTSIZEOVER	10003
#define SMQ_WRITING		10004
#define SMQ_READING		10005

// DEFINE Shared Memory Costumizing

#define MAX_PKTSIZE		512
#define MAX_COUNT		4096
#define SMQ_ITEMLOGGER	"ITEMLOG_SEND"

// Packet Define...
#define WIZ_ITEM_LOG		0x19	// Send To Agent for Writing Log
#define WIZ_DATASAVE		0x37	// User GameData DB Save Request

#define SLOT_MAX			14		// 착용 아템 MAX
#define HAVE_MAX			28		// 소유 아템 MAX (인벤토리창)	
#define ITEMCOUNT_MAX		9999	// 소모 아이템 소유 한계값
#define WAREHOUSE_MAX		196		// 창고 아이템 MAX
/////////////////////////////////////////////////////////////////////////////////
// Structure Define
/////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
//	Global Function Define
//

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

inline void SetString(char* tBuf, char* sBuf, int len, int& index)
{
	memcpy(tBuf + index, sBuf, len);
	index += len;
}

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

inline void SetInt64(char* tBuf, int64_t nInt64, int& index)
{
	CopyMemory(tBuf + index, &nInt64, 8);
	index += 8;
}

inline int64_t GetInt64(char* sBuf, int& index)
{
	index += 8;
	return *(int64_t*) (sBuf + index - 8);
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

	LogFileName.Format("%s\\Aujard.log", ProgPath);

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
